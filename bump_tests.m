clear all; clc; close all

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])

%cd CompileFolder % move to CompileFolder on start (optional)

% Do not delete the above lines, they are used to run this script

% ==========================================================================
%  bump_tests -- 20 automated bump tests for motor sysID + PI/PD tuning
% --------------------------------------------------------------------------
%  WHAT THIS DOES
%    Drives the motor through the MSE312 FIRMWARE over serial (the `v <volts>`
%    command + the telemetry stream) and runs a bump (step) test at every 10%
%    duty from 10..100%, in BOTH directions -> 10 x 2 = 20 tests. For each it
%    logs the angular-velocity step response, fits the first-order motor model
%
%          omega(s)     K
%          -------- = -------          (angular velocity per commanded volt)
%           V(s)      tau*s + 1
%
%    and from the averaged K, tau it recommends:
%       * PI gains for an ANGULAR-VELOCITY loop           (plant  K/(tau s+1))
%       * PD gains for an ANGLE loop, error taken mod 2pi (plant  K/(s(tau s+1)))
%    The position plant is just the velocity plant integrated, so ONE set of
%    velocity bumps tunes both loops.
%
%  WHY DRIVE THROUGH THE FIRMWARE (not MATLAB's arduino() like lab4)?
%    - Survives full speed: the encoder is counted on-MCU (lean ISRs) and only
%      20 Hz telemetry crosses USB, so 100% duty never floods the link (which is
%      exactly what breaks lab4's readCount polling).
%    - Wiring-agnostic: the firmware owns the motor pins, so this works whether
%      the SN7400 NAND board is installed or you're on the direct D12/D13 drive.
%    - The identified K/tau live in the SAME filtered omega and volt->duty
%      convention the PID uses, so the gains drop straight into the firmware.
%
%  REQUIREMENT: the board must be running the MSE312 firmware (src/main.cpp),
%    NOT MATLAB's server firmware that nand_debug.m / lab4 flash. If you just ran
%    nand_debug.m, re-upload the firmware (PlatformIO: pio run -t upload) first.
%    The script errors out with a clear message if no telemetry arrives.
%
%  SAFETY: every test starts from rest. Between tests the motor is short-braked
%    and allowed to stop before the next (and before any direction reversal).
%    Secure the motor -- it will spin up to full speed on the 100% steps.
% ==========================================================================


% ==========================================================================
%  1) SETTINGS  -- edit these, then just press Run
% ==========================================================================
PORT   = 'COM5';        % Arduino USB port -- arduino() auto-detects the Mega here.
                        % Confirm with serialportlist("available") if it moves (replugging
                        % into a different USB socket changes the number). Close Simulink and
                        % clear any arduino object first -- they hold the port and a raw
                        % serialport() open will block/fail while it's held.
BAUD   = 115200;        % must match Serial.begin() in the firmware

% --- Full-scale voltage convention (MUST match firmware V_MAX) ------------
V_FS   = 12.0;          % firmware maps duty = |v|/12 -> commanding v = duty*12
                        % reproduces the requested duty exactly, and K comes out
                        % in the same "volts" the PID outputs. Leave at 12.

% --- Test grid ------------------------------------------------------------
DUTIES = 0.1:0.1:0.4;   % 10%..40% in 10% steps (4 levels -> 8 tests). Above ~40% the
                        % motor inrush browns out the supply and drops the board; stay
                        % in this range unless you stiffen the power / add decoupling.
DIRS   = [+1 -1];       % +1 = CW (v>0), -1 = CCW (v<0)  -> 4 x 2 = 8 tests total

% --- Bump timing (seconds) ------------------------------------------------
T_BASE = 0.4;           % baseline-at-rest logged before the step
T_BUMP = 2.5;           % logged after the step (long enough to reach steady speed)
T_STOP = 3.5;           % settle time to let the motor stop between tests. The NAND
                        % board can't short-brake, so the motor COASTS to rest --
                        % raise this if high-duty runs haven't fully stopped in time.

% --- Controller-design targets (starting points -- tune to taste) ---------
VEL_SPEEDUP = 5.0;      % velocity loop made ~this many times faster than open-loop
POS_ZETA    = 0.8;      % position loop target damping ratio
POS_TS      = 0.40;     % position loop target ~2% settling time (s)

% --- Which tests to trust when averaging K/tau ----------------------------
FIT_MIN_DUTY = 0.2;     % ignore <20% duty when averaging K/tau (low speed is more
                        % friction/deadband-dominated); uses 20/30/40% here

% --- Output ---------------------------------------------------------------
RAW_CSV = 'bump_tests_raw.csv';       % every logged sample, tagged by test
SUM_CSV = 'bump_tests_summary.csv';   % one row per test: dir, duty, v, K, tau
DO_PLOT = true;


% ==========================================================================
%  2) OPEN the serial link to the firmware
% ==========================================================================
fprintf('Opening %s at %d baud ...\n', PORT, BAUD);
sp = serialport(PORT, BAUD);
sp.Timeout = 1;                       % bound any readline() stall
configureTerminator(sp, "LF");        % firmware ends lines with \r\n; we strtrim the \r
pause(2.0);                           % opening the port resets the Arduino -> let it boot
flush(sp);
writeline(sp, "telem on");            % start the CSV telemetry stream
writeline(sp, "brake");               % known safe state (held, omega = 0)
pause(0.3);
fprintf('Link open. Firmware telemetry enabled.\n\n');


% ==========================================================================
%  3) RUN the 20 bump tests
% ==========================================================================
nTests = numel(DIRS) * numel(DUTIES);
tests  = struct('dir',cell(nTests,1),'duty',[],'v',[],'t',[],'omega',[], ...
                'theta',[],'K',[],'tau',[],'omega_ss',[]);
n = 0;

fprintf('%-5s %5s %7s | %10s %8s %9s\n', ...
        'dir', 'duty', 'v (V)', 'omega_ss', 'K', 'tau (s)');
fprintf('%s\n', repmat('-', 1, 52));

% try/catch (not onCleanup -- unreliable in a script) guarantees the motor is
% braked if anything throws mid-run.
try
    for dir = DIRS
        for duty = DUTIES
            vCmd = dir * duty * V_FS;             % signed command volts

            % --- reset to a clean, stopped start ---
            writeline(sp, "brake");               % PWM off -> motor coasts to rest
            pause(T_STOP);                        % let it coast down fully
            writeline(sp, "zero");                % reset encoder count + vel filter
            writeline(sp, "telem on");            % re-assert: a brownout reset drops telem
            pause(0.1);
            flush(sp);                            % drop stale settle-telemetry

            % --- the bump: log baseline, step to vCmd, log the response ---
            [t, omega, theta] = runBump(sp, vCmd, T_BASE, T_BUMP);
            writeline(sp, "brake");               % stop the motor after the bump

            % --- fit K/(tau s + 1) to this step ---
            [K, tau, omega_ss] = fitFirstOrder(t, omega, vCmd);

            n = n + 1;
            tests(n).dir  = dir;   tests(n).duty = duty;  tests(n).v = vCmd;
            tests(n).t    = t;     tests(n).omega = omega; tests(n).theta = theta;
            tests(n).K    = K;     tests(n).tau  = tau;   tests(n).omega_ss = omega_ss;

            fprintf('%-5s %4.0f%% %7.2f | %10.2f %8.3f %9.3f\n', ...
                    ternary(dir>0,'CW','CCW'), 100*duty, vCmd, omega_ss, K, tau);
        end
        fprintf('%s\n', repmat('-', 1, 52));
    end
catch runErr
    try, writeline(sp, "brake"); catch, end       % motor safe first
    fprintf(2, '\n!! Run aborted: %s\n', runErr.message);
end

tests = tests(1:n);
try, writeline(sp, "brake"); catch, end
if n == 0
    try, writeline(sp, "telem off"); catch, end
    error('No bump tests completed -- see the message above (firmware/telemetry?).');
end
if n < nTests
    fprintf(2, 'Proceeding with the %d of %d tests that completed.\n', n, nTests);
else
    fprintf('\nAll %d bump tests done.\n', n);
end


% ==========================================================================
%  4) SAVE the data
% ==========================================================================
% Long-format raw log: one row per sample, tagged with its test.
rawTid=[]; rawDir=[]; rawDuty=[]; rawV=[]; rawT=[]; rawW=[]; rawTh=[];
for i = 1:numel(tests)
    m = numel(tests(i).t);
    rawTid  = [rawTid;  i*ones(m,1)];         %#ok<AGROW>
    rawDir  = [rawDir;  tests(i).dir*ones(m,1)];  %#ok<AGROW>
    rawDuty = [rawDuty; tests(i).duty*ones(m,1)]; %#ok<AGROW>
    rawV    = [rawV;    tests(i).v*ones(m,1)];    %#ok<AGROW>
    rawT    = [rawT;    tests(i).t(:)];       %#ok<AGROW>
    rawW    = [rawW;    tests(i).omega(:)];   %#ok<AGROW>
    rawTh   = [rawTh;   tests(i).theta(:)];   %#ok<AGROW>
end
writetable(table(rawTid,rawDir,rawDuty,rawV,rawT,rawW,rawTh, ...
    'VariableNames',{'test','dir','duty','v_V','t_s','omega_rad_s','theta_rad'}), RAW_CSV);

writetable(table([tests.dir]',[tests.duty]',[tests.v]', ...
                 [tests.omega_ss]',[tests.K]',[tests.tau]', ...
    'VariableNames',{'dir','duty','v_V','omega_ss','K','tau_s'}), SUM_CSV);
fprintf('Saved %s (raw samples) and %s (per-test fits).\n', RAW_CSV, SUM_CSV);


% ==========================================================================
%  5) AGGREGATE K/tau and recommend gains
% ==========================================================================
fprintf('\n=== Identified model  omega/V = K/(tau*s + 1) ===\n');
[Kcw,  tauCw ] = aggregate(tests, +1, FIT_MIN_DUTY);
[Kccw, tauCcw] = aggregate(tests, -1, FIT_MIN_DUTY);
fprintf('  CW : K = %.4f (rad/s)/V,  tau = %.4f s\n', Kcw,  tauCw);
fprintf('  CCW: K = %.4f (rad/s)/V,  tau = %.4f s\n', Kccw, tauCcw);

K   = mean([Kcw Kccw], 'omitnan');       % symmetric design point
tau = mean([tauCw tauCcw], 'omitnan');
fprintf('  Design point (both-dir avg): K = %.4f (rad/s)/V,  tau = %.4f s\n', K, tau);

% Deadband: lowest duty in each direction that actually produced motion.
fprintf('  Deadband: CW starts ~%.0f%% duty, CCW starts ~%.0f%% duty\n', ...
        100*deadbandDuty(tests,+1), 100*deadbandDuty(tests,-1));

% --- (a) VELOCITY PI: pole-zero cancel (Ti = tau), speed up by VEL_SPEEDUP ---
%   C(s) = Kp_v (1 + 1/(Ti s)),  Ti = tau  ->  closed loop tau_cl = tau/VEL_SPEEDUP
%   Kp_v = VEL_SPEEDUP / K ,  Ki_v = Kp_v / tau
Kp_v = VEL_SPEEDUP / K;
Ki_v = Kp_v / tau;
fprintf('\n=== Angular-VELOCITY loop  (PI, plant K/(tau s+1)) ===\n');
fprintf('  target: ~%.0fx faster than open loop (closed-loop tau ~ %.3f s)\n', ...
        VEL_SPEEDUP, tau/VEL_SPEEDUP);
fprintf('  Kp_v = %.4f  V/(rad/s)\n', Kp_v);
fprintf('  Ki_v = %.4f  V/(rad/s)/s   (Ti = tau = %.3f s)\n', Ki_v, tau);

% --- (b) POSITION PD: place a 2nd-order closed loop (zeta, omega_n) ---------
%   plant K/(s(tau s+1)), C = Kp + Kd s  ->  tau s^2 + (1+K Kd) s + K Kp = 0
%   match tau(s^2 + 2 zeta wn s + wn^2):  Kp = tau wn^2 / K ,  Kd = (2 zeta wn tau - 1)/K
wn   = 4 / (POS_ZETA * POS_TS);          % ~2% settling time -> natural frequency
Kp_p = tau * wn^2 / K;
Kd_p = (2*POS_ZETA*wn*tau - 1) / K;
fprintf('\n=== ANGLE loop, angle mod pi  (PD, plant K/(s(tau s+1))) ===\n');
fprintf('  target: zeta = %.2f, settling ~%.2f s  ->  wn = %.2f rad/s\n', ...
        POS_ZETA, POS_TS, wn);
fprintf('  Kp_p = %.4f  V/rad\n', Kp_p);
if Kd_p < 0
    fprintf(2, '  Kd_p = %.4f -> NEGATIVE: the plant pole already gives more damping\n', Kd_p);
    fprintf(2, '         than requested. Use Kd_p = 0 (pure P is enough), or lower\n');
    fprintf(2, '         POS_TS / raise POS_ZETA to demand a faster, tighter loop.\n');
    Kd_p = 0;
end
fprintf('  Kd_p = %.4f  V/(rad/s)\n', Kd_p);
fprintf('\n  Firmware note: set g_pid.kp = Kp_p, g_pid.kd = Kd_p, g_pid.ki = 0 for the\n');
fprintf('  angle PD. Half-turn symmetric control: reduce the angle mod pi and wrap\n');
fprintf('  the error into [-pi/2, pi/2] before the PID:\n');
fprintf('    theta_m = mod(theta, pi);  e = mod(target - theta_m + pi/2, pi) - pi/2;\n');
fprintf('  Validate with validate_tuning.m (expected sim vs actual motor run).\n');


% ==========================================================================
%  6) PLOTS
% ==========================================================================
if DO_PLOT
    % (a) all step responses, colored by duty, split by direction
    figure('Name','bump tests -- velocity step responses','NumberTitle','off');
    for j = 1:numel(DIRS)
        subplot(numel(DIRS),1,j); hold on; grid on
        for i = 1:numel(tests)
            if tests(i).dir == DIRS(j)
                plot(tests(i).t, tests(i).omega, 'LineWidth',1.0, ...
                     'DisplayName',sprintf('%.0f%%',100*tests(i).duty));
            end
        end
        xlabel('time since step (s)'); ylabel('\omega (rad/s)');
        title(sprintf('%s step responses', ternary(DIRS(j)>0,'CW','CCW')));
        legend('Location','eastoutside');
    end

    % (b) steady-state speed vs commanded volts (linearity + deadband)
    figure('Name','bump tests -- speed vs volts','NumberTitle','off'); hold on; grid on
    v_all = [tests.v]'; w_all = [tests.omega_ss]';
    plot(v_all(v_all>0),  w_all(v_all>0),  'o-','LineWidth',1.2,'DisplayName','CW');
    plot(-v_all(v_all<0),-w_all(v_all<0),  's-','LineWidth',1.2,'DisplayName','CCW (|.|)');
    xlabel('|commanded voltage| (V)'); ylabel('|\omega_{ss}| (rad/s)');
    title('Steady-state speed vs voltage (slope \approx K, x-intercept \approx deadband)');
    legend('Location','best');

    % (c) fitted K and tau vs duty
    figure('Name','bump tests -- K and tau vs duty','NumberTitle','off');
    subplot(2,1,1); hold on; grid on
    for j = 1:numel(DIRS)
        mask = ([tests.dir]'==DIRS(j));
        plot(100*[tests(mask).duty]', abs([tests(mask).K]'), 'o-', ...
             'LineWidth',1.2,'DisplayName',ternary(DIRS(j)>0,'CW','CCW'));
    end
    ylabel('|K| (rad/s)/V'); title('Gain K vs duty'); legend('Location','best');
    subplot(2,1,2); hold on; grid on
    for j = 1:numel(DIRS)
        mask = ([tests.dir]'==DIRS(j));
        plot(100*[tests(mask).duty]', [tests(mask).tau]', 'o-', ...
             'LineWidth',1.2,'DisplayName',ternary(DIRS(j)>0,'CW','CCW'));
    end
    xlabel('duty (%)'); ylabel('\tau (s)'); title('Time constant \tau vs duty');
    legend('Location','best');
end

fprintf('\nDone.\n');
% Motor is braked; telemetry left on and the port (variable sp) left open so you
% can keep sending commands. Run  clear sp  to release COM5 when finished.


% ==========================================================================
%  LOCAL FUNCTIONS
% ==========================================================================

function [t, omega, theta] = runBump(sp, vCmd, T_BASE, T_BUMP)
% Log telemetry for T_BASE s at rest, issue the step to vCmd, then log T_BUMP s
% of the response. Returns time (s) measured from the STEP instant, plus the
% filtered angular velocity (rad/s) and wrapped angle (rad) from the firmware.
    raw     = zeros(0, 7);       % [t_fw theta omega duty dir1 dir2 mode]
    t0      = tic;
    stepped = false;
    while toc(t0) < (T_BASE + T_BUMP)
        if sp.NumBytesAvailable > 0
            row = parseTelem(readline(sp));
            if ~isempty(row), raw(end+1, :) = row; end %#ok<AGROW>
        end
        if ~stepped && toc(t0) >= T_BASE
            writeline(sp, sprintf('v %.4f', vCmd));   % <-- the bump
            stepped = true;
        end
    end
    if isempty(raw)
        error(['No telemetry received. Is the MSE312 firmware flashed (not the ' ...
               'MATLAB server firmware from nand_debug/lab4)? Re-upload it, then ' ...
               'rerun.']);
    end

    % Time zero = the step. Detect it as the first sample that leaves the brake
    % state (brake holds BOTH dir bits high; a drive command sets exactly one).
    notBrake = ~(raw(:,5) & raw(:,6));
    kStep    = find(notBrake, 1, 'first');
    if isempty(kStep), kStep = 1; end

    t     = raw(:,1) - raw(kStep,1);   % seconds since the step (negative before)
    theta = raw(:,2);
    omega = raw(:,3);
end

function row = parseTelem(line)
% Parse one telemetry line "t,theta,omega,duty,DIR1,DIR2,mode" -> 1x7 numbers.
% Returns [] for comments (# ...), blanks, or malformed/partial lines.
    row = [];
    if ismissing(line), return; end
    s = strtrim(line);
    if isempty(s) || startsWith(s, '#'), return; end
    p = str2double(split(s, ','));
    if numel(p) >= 7 && all(isfinite(p(1:3)))
        row = p(1:7).';
    end
end

function [K, tau, omega_ss] = fitFirstOrder(t, omega, vCmd)
% First-order step-response fit for K/(tau s + 1) from a bump starting at rest:
%   omega_ss = mean of the last 30% of the post-step record
%   K        = omega_ss / vCmd
%   tau      = time after the step to reach 63.2% of omega_ss
    post = (t >= 0);
    tp = t(post);  wp = omega(post);
    if numel(wp) < 4
        K = NaN; tau = NaN; omega_ss = NaN; return
    end
    tail     = wp(ceil(0.7*numel(wp)):end);
    omega_ss = mean(tail);
    K        = omega_ss / vCmd;

    target = 0.632 * omega_ss;
    if omega_ss >= 0, hit = find(wp >= target, 1, 'first');
    else,             hit = find(wp <= target, 1, 'first');
    end
    if isempty(hit), tau = NaN; else, tau = tp(hit) - tp(1); end
end

function [K, tau] = aggregate(tests, dir, minDuty)
% Median K and tau over the trustworthy tests (given direction, duty >= minDuty,
% finite fits). Median rejects the odd bad bump without hand-picking.
    Ks = []; taus = [];
    for i = 1:numel(tests)
        if tests(i).dir == dir && tests(i).duty >= minDuty ...
                && isfinite(tests(i).K) && isfinite(tests(i).tau)
            Ks(end+1)   = abs(tests(i).K);   %#ok<AGROW>
            taus(end+1) = tests(i).tau;      %#ok<AGROW>
        end
    end
    if isempty(Ks), K = NaN; tau = NaN; else, K = median(Ks); tau = median(taus); end
end

function d = deadbandDuty(tests, dir)
% Lowest duty (given direction) that produced real motion (|omega_ss| > 5 rad/s).
    d = NaN;
    duties = sort(unique([tests([tests.dir]==dir).duty]));
    for duty = duties
        for i = 1:numel(tests)
            if tests(i).dir == dir && tests(i).duty == duty ...
                    && abs(tests(i).omega_ss) > 5
                d = duty; return
            end
        end
    end
end

function out = ternary(cond, a, b)
    if cond, out = a; else, out = b; end
end
