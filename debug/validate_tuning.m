clear all; clc; close all

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])

%cd CompileFolder % move to CompileFolder on start (optional)

% Do not delete the above lines, they are used to run this script

% ==========================================================================
%  validate_tuning -- expected (simulated) vs actual (measured) angle PD result
% --------------------------------------------------------------------------
%  Closes an ANGLE control loop with the PD gains from bump_tests, where the
%  controlled variable is the shaft angle REDUCED MOD PI (half-turn symmetry: 0
%  and pi are the same position). The position error is taken the short way round
%  that pi-circumference circle, i.e. wrapped into [-pi/2, pi/2]:
%
%       theta_m = mod(theta, pi)                    % 0..pi
%       e       = mod(target - theta_m + pi/2, pi) - pi/2      % [-pi/2, pi/2]
%       u       = Kp*e - Kd*omega     (derivative on measurement)   [clamp +/-12 V]
%
%  TWO SEPARATE PHASES (run one at a time -- this is the "save the bandwidth"
%  workflow you asked for):
%    PHASE 1  DO_EXPECTED = true, DO_EXPERIMENT = false
%             Pure simulation of the identified model K/(tau s+1) under the PD
%             law above. Touches NO hardware / serial. Saves the expected curve
%             to validate_expected.mat and plots it. Run this FIRST.
%    PHASE 2  DO_EXPERIMENT = true  (set DO_EXPECTED = false to reuse the saved
%             curve, or leave it true to recompute)
%             Runs the SAME PD law on the real motor via the firmware (MATLAB
%             software-in-the-loop: read telemetry theta/omega -> compute u ->
%             send `v u`). Then overlays actual vs the saved expected on ONE graph.
%
%  Model + gains come from bump_tests_summary.csv (set USE_MANUAL_GAINS to type
%  them in instead). Requires the MSE312 firmware for PHASE 2 (not the MATLAB
%  server firmware that nand_debug/lab4 flash).
%
%  NOTE ON RATE: PHASE 2 is a software-in-the-loop test at the ~20 Hz telemetry
%  rate with USB latency, so the actual may lag the expected slightly. Once these
%  gains live in the firmware PID (100 Hz), it will track the expected more
%  closely. To see the ideal continuous curve, lower SIM_DT below LOOP_DT.
% ==========================================================================


% ==========================================================================
%  1) SETTINGS
% ==========================================================================
PORT   = 'COM5';        % Arduino USB port (arduino() auto-detects the Mega here). Confirm
                        % with serialportlist("available"); close Simulink / clear any
                        % arduino object first -- they hold the port.
BAUD   = 115200;        % must match the firmware Serial.begin()
V_FS   = 12.0;          % clamp / firmware full-scale volts (V_MAX)

% --- What to run this time ------------------------------------------------
DO_EXPECTED   = true;   % PHASE 1: simulate expected result (no hardware)
DO_EXPERIMENT = false;  % PHASE 2: run it on the motor, then overlay

% --- Test command ---------------------------------------------------------
THETA_TARGET  = 1.0;    % angle setpoint in [0, pi) rad (mod-pi space). Keep away
                        % from 0 and pi/2 (the seam and the antipode). ~57 deg.
T_SIM         = 2.5;    % seconds to simulate / run the step
LOOP_DT       = 0.05;   % nominal control period on hardware (~telemetry rate)
SIM_DT        = 0.05;   % simulation step; = LOOP_DT for a fair compare (lower it
                        % to preview the ideal near-continuous response)
T_STOP        = 2.0;    % brake settle before the run (let the motor reach rest)

% --- Model + PD gains -----------------------------------------------------
USE_MANUAL_GAINS = false;      % true = use the K/tau/Kp/Kd typed below
SUM_CSV          = 'bump_tests_summary.csv';   % from bump_tests.m
FIT_MIN_DUTY     = 0.3;        % ignore <30% duty tests when averaging K/tau
POS_ZETA         = 0.8;        % PD design: target damping ratio
POS_TS           = 0.40;       % PD design: target ~2% settling time (s)
% ...used only when USE_MANUAL_GAINS = true:
K_MAN   = 8.0;   TAU_MAN = 0.15;   KP_MAN = 5.0;   KD_MAN = 0.3;

% --- Files ----------------------------------------------------------------
EXPECTED_MAT = 'validate_expected.mat';   % expected curve saved here (PHASE 1)
ACTUAL_CSV   = 'validate_actual.csv';     % measured curve saved here (PHASE 2)

target = mod(THETA_TARGET, pi);           % force into the [0, pi) mod-pi space


% ==========================================================================
%  2) MODEL + GAINS  (needed by both phases)
% ==========================================================================
if USE_MANUAL_GAINS
    K = K_MAN; tau = TAU_MAN; Kp = KP_MAN; Kd = KD_MAN; wn = NaN;
    fprintf('Gains: manual  K=%.3f tau=%.3f Kp=%.3f Kd=%.3f\n', K, tau, Kp, Kd);
else
    if exist(SUM_CSV, 'file') ~= 2
        error(['%s not found. Run bump_tests.m first, or set USE_MANUAL_GAINS ' ...
               '= true.'], SUM_CSV);
    end
    [K, tau] = modelFromCsv(SUM_CSV, FIT_MIN_DUTY);
    wn = 4 / (POS_ZETA * POS_TS);
    Kp = tau * wn^2 / K;
    Kd = (2*POS_ZETA*wn*tau - 1) / K;
    if Kd < 0, Kd = 0; end                 % plant already that damped -> pure P
    fprintf('Model (from %s): K=%.4f (rad/s)/V, tau=%.4f s\n', SUM_CSV, K, tau);
    fprintf('PD design (zeta=%.2f, ts=%.2f -> wn=%.2f): Kp=%.4f V/rad, Kd=%.4f V/(rad/s)\n', ...
            POS_ZETA, POS_TS, wn, Kp, Kd);
end
fprintf('Target = %.4f rad (mod pi),  clamp = +/-%.1f V\n\n', target, V_FS);


% ==========================================================================
%  3) PHASE 1 -- EXPECTED (simulate; no hardware, no serial)
% ==========================================================================
haveExp = false;
if DO_EXPECTED
    fprintf('PHASE 1: simulating expected closed-loop response ...\n');
    Sexp = simClosedLoop(K, tau, Kp, Kd, target, SIM_DT, T_SIM, V_FS);
    save(EXPECTED_MAT, 'Sexp', 'K', 'tau', 'Kp', 'Kd', 'target', 'V_FS');
    haveExp = true;
    fprintf('Expected result saved to %s\n\n', EXPECTED_MAT);
end


% ==========================================================================
%  4) PHASE 2 -- ACTUAL (run the same PD law on the motor)
% ==========================================================================
haveAct = false;
if DO_EXPERIMENT
    fprintf('PHASE 2: opening %s and running the motor ...\n', PORT);
    sp = serialport(PORT, BAUD);
    sp.Timeout = 1;
    configureTerminator(sp, "LF");
    pause(2.0);                     % opening the port resets the Arduino -> boot
    flush(sp);
    writeline(sp, "telem on");
    writeline(sp, "brake");
    pause(0.3);
    try
        Sact = runClosedLoopHW(sp, Kp, Kd, target, T_SIM, V_FS, T_STOP);
        haveAct = true;
    catch runErr
        try, writeline(sp, "brake"); catch, end
        fprintf(2, '!! Experiment aborted: %s\n', runErr.message);
    end
    try, writeline(sp, "brake"); catch, end
    if haveAct
        writetable(table(Sact.t, mod(Sact.theta,pi), Sact.omega, Sact.u, ...
            'VariableNames', {'t_s','theta_modpi_rad','omega_rad_s','u_V'}), ACTUAL_CSV);
        fprintf('Actual result saved to %s\n\n', ACTUAL_CSV);
    end
end

% If we ran the experiment but did not recompute the expected this time, load the
% saved expected so we can still overlay.
if haveAct && ~haveExp && exist(EXPECTED_MAT, 'file') == 2
    L = load(EXPECTED_MAT); Sexp = L.Sexp; haveExp = true;
    fprintf('Loaded saved expected curve from %s for the overlay.\n', EXPECTED_MAT);
end


% ==========================================================================
%  5) GRAPH  --  expected vs actual (angle mod pi) + control effort
% ==========================================================================
figure('Name','Tuning validation: expected vs actual','NumberTitle','off');

subplot(2,1,1); hold on; grid on
yline(target, 'k--', 'target', 'LineWidth',1.0);
if haveExp
    plot(Sexp.t, mod(Sexp.theta, pi), 'LineWidth',1.6, 'DisplayName','expected (sim)');
end
if haveAct
    plot(Sact.t, mod(Sact.theta, pi), 'LineWidth',1.4, 'DisplayName','actual (motor)');
end
ylabel('\theta mod \pi (rad)'); title('Angle step response (mod \pi control)');
legend('Location','southeast');

subplot(2,1,2); hold on; grid on
if haveExp, plot(Sexp.t, Sexp.u, 'LineWidth',1.4, 'DisplayName','expected u'); end
if haveAct, plot(Sact.t, Sact.u, 'LineWidth',1.2, 'DisplayName','actual u'); end
yline( V_FS, ':'); yline(-V_FS, ':');
xlabel('time since step (s)'); ylabel('command u (V)');
title('Control effort'); legend('Location','best');


% ==========================================================================
%  6) METRICS comparison
% ==========================================================================
fprintf('=== Step-response metrics (angle mod pi) ===\n');
fprintf('%-16s %10s %10s %12s\n', '', 'overshoot', 'settle(s)', 'ss error');
if haveExp
    m = stepMetrics(Sexp.t, mod(Sexp.theta,pi), target);
    fprintf('%-16s %9.1f%% %10.3f %12.4f\n', 'expected (sim)', m.overshoot, m.settle, m.ss_err);
end
if haveAct
    m = stepMetrics(Sact.t, mod(Sact.theta,pi), target);
    fprintf('%-16s %9.1f%% %10.3f %12.4f\n', 'actual (motor)', m.overshoot, m.settle, m.ss_err);
end
if ~haveAct
    fprintf(['\n(Only the expected curve was generated. Set DO_EXPERIMENT = true\n' ...
             ' to run the motor and overlay the actual result.)\n']);
end
fprintf('\nDone.\n');
% PHASE 2 leaves the motor braked and the port (sp) open -- run  clear sp  to free COM5.


% ==========================================================================
%  LOCAL FUNCTIONS
% ==========================================================================

function e = wrapErrHalfPi(target, theta)
% Shortest-path angle error in the half-turn-symmetric (mod pi) space.
% Measured angle is reduced mod pi (0 == pi); the error is wrapped into
% [-pi/2, pi/2] so the controller always turns the short way.
    thetaM = mod(theta, pi);                    % 0..pi
    e = mod((target - thetaM) + pi/2, pi) - pi/2;
end

function S = simClosedLoop(K, tau, Kp, Kd, target, dt, T, V_FS)
% Simulate the mod-pi angle PD loop on the identified first-order model.
%   velocity:  omega/V = K/(tau s+1)   (exact ZOH step:  a=e^{-dt/tau}, b=K(1-a))
%   angle:     theta = integral(omega)  (trapezoidal)
%   control:   u = Kp*e - Kd*omega, e = wrapErrHalfPi(target, theta), clamp +/-V_FS
    N  = floor(T/dt) + 1;
    t  = (0:N-1)' * dt;
    th = zeros(N,1);  w = zeros(N,1);  u = zeros(N,1);
    a  = exp(-dt/tau);  b = K*(1 - a);
    for k = 2:N
        e     = wrapErrHalfPi(target, th(k-1));         % controller uses last state
        uk    = max(min(Kp*e - Kd*w(k-1), V_FS), -V_FS);
        w(k)  = a*w(k-1) + b*uk;                        % velocity update
        th(k) = th(k-1) + 0.5*(w(k) + w(k-1))*dt;       % integrate to angle
        u(k)  = uk;
    end
    u(1) = u(2);
    S.t = t;  S.theta = th;  S.omega = w;  S.u = u;
end

function S = runClosedLoopHW(sp, Kp, Kd, target, T, V_FS, T_STOP)
% Software-in-the-loop: close the same mod-pi PD loop on the real motor. Reads
% the firmware telemetry (theta wrapped 0..2pi, filtered omega), computes u, and
% streams `v u` back. Time is measured from the step (loop start).
    writeline(sp, "brake");  pause(T_STOP);      % start from rest
    writeline(sp, "zero");   pause(0.1);         % theta = 0, filter reset
    flush(sp);

    % Confirm telemetry is actually flowing before we start commanding.
    tPrime = tic;  primed = false;
    while toc(tPrime) < 2.0
        if sp.NumBytesAvailable > 0 && ~isempty(parseTelem(readline(sp)))
            primed = true; break
        end
    end
    if ~primed
        error(['No telemetry. Is the MSE312 firmware flashed (not the MATLAB ' ...
               'server firmware) and streaming? Re-upload it, then rerun.']);
    end

    t = []; th = []; w = []; u = [];
    t0 = tic;
    while toc(t0) < T
        row = parseTelem(readline(sp));      % one telem line ~ paces at 20 Hz
        if isempty(row), continue; end
        theta = row(2);  omega = row(3);
        e  = wrapErrHalfPi(target, theta);
        uk = max(min(Kp*e - Kd*omega, V_FS), -V_FS);
        writeline(sp, sprintf('v %.4f', uk));
        t(end+1,1)  = toc(t0);  th(end+1,1) = theta;  %#ok<AGROW>
        w(end+1,1)  = omega;    u(end+1,1)  = uk;      %#ok<AGROW>
    end
    writeline(sp, "brake");
    S.t = t;  S.theta = th;  S.omega = w;  S.u = u;
end

function row = parseTelem(line)
% Parse "t,theta,omega,duty,DIR1,DIR2,mode" -> 1x7 numbers; [] for #comments,
% blanks, or partial lines.
    row = [];
    if ismissing(line), return; end
    s = strtrim(line);
    if isempty(s) || startsWith(s, '#'), return; end
    p = str2double(split(s, ','));
    if numel(p) >= 7 && all(isfinite(p(1:3))), row = p(1:7).'; end
end

function [K, tau] = modelFromCsv(csv, minDuty)
% Median K and tau over the trustworthy bump tests (duty >= minDuty, finite).
    T    = readtable(csv);
    good = abs(T.duty) >= minDuty & isfinite(T.K) & isfinite(T.tau_s);
    K    = median(abs(T.K(good)));
    tau  = median(T.tau_s(good));
end

function m = stepMetrics(t, y, target)
% Overshoot (%), 2% settling time (s) and steady-state error for a step to target.
    yf         = mean(y(t >= 0.8*t(end)));       % final value
    m.ss_err   = target - yf;
    m.overshoot = max(0, (max(y) - target) / max(target, eps) * 100);
    band    = 0.02 * max(abs(target), eps);
    outside = find(abs(y - target) > band);
    if isempty(outside), m.settle = 0; else, m.settle = t(outside(end)); end
end
