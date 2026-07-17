clear all; clc; close all

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])

%cd CompileFolder % move to CompileFolder on start (optional)

% Do not delete the above lines, they are used to run this script

% ==========================================================================
%  nand_debug -- bench test / CHARACTERIZE the SN7400 NAND combiner board
% --------------------------------------------------------------------------
%  WHAT THIS DOES
%    Drives the SN7400 board from the Arduino (D9 = PWM, D8 = DIR) and reports
%    what each output node actually does -- WITHOUT the H-bridge or motor in the
%    loop. It does NOT assume an idealized truth table; instead it sweeps the
%    inputs and classifies each probed node in plain English (tracks PWM, tracks
%    inverted PWM, constant HIGH/LOW, = DIR, floating, ...). You then read off the
%    real behaviour. An optional model-check (CHECK_MODEL) is available once you
%    know the wiring.
%
%    IMPORTANT -- the SN7400 is INVERTING (it's NAND). A healthy board therefore
%    idles its outputs HIGH and chops the ACTIVE input with INVERTED PWM:
%        active line  ->  NOT PWM      (its averaged voltage FALLS as duty rises)
%        idle line    ->  HIGH (~VCC)
%    At 0% duty both H-bridge inputs sit HIGH -- for the TB6568KQ that is the
%    BRAKE state, not a fault. (The earlier "shoot-through" alarm was bogus for
%    this driver; both-HIGH = brake, and it can't shoot through from its logic
%    inputs anyway.) This is why an idealized "PWM AND DIR" model reports FAIL on
%    a perfectly good board -- so pass/fail is OFF by default; characterize first.
%
%  WIRING (all nodes stay within 0..5 V -- SN7400 is a 5 V TTL part)
%
%        Arduino                         NAND board                Arduino
%        -------                         ----------                -------
%        D9  (PWM out)  ------------>  PWM in
%        D8  (DIR out)  ------------>  DIR in
%                                       PWM (buffered) out ----->  A0
%                                       IN1 out          ----->  A1
%                                       IN2 out          ----->  A2
%        GND ------------------------  GND  ------------------- (common ground!)
%
%    Common ground between the Arduino and the NAND board is essential or the
%    analog readings are meaningless. If a probe reads a fixed mid-rail voltage
%    (~0.5-2 V) that ignores both inputs, that wire is most likely on the wrong
%    node / floating -- reseat it.
%
%  HOW WE READ A PWM SIGNAL ON AN ANALOG PIN
%    readVoltage() takes one ~100 us snapshot, so a raw ~490 Hz PWM line reads as
%    a random 0 V / 5 V. Averaging MANY unsynchronised snapshots recovers the
%    LEVEL: mean(readVoltage) ~= (fraction HIGH)*VCC. For a NAND-chopped line that
%    is (1-duty)*VCC. Add a simple RC low-pass (10k + 1 uF) per analog input for
%    crisp single-shot reads; then NAVG can drop to ~10.
%
%  NOTE: creating arduino() re-flashes MATLAB's server firmware over the MSE312
%  firmware -- fine here. Re-upload the .hex afterwards to restore motor control.
% ==========================================================================


% ==========================================================================
%  1) SETTINGS  -- edit these, then just press Run
% ==========================================================================
PORT   = 'COM5';        % Arduino USB port (arduino() auto-detects the Mega here).
                        % Verify with serialportlist("available") if it moves.
BOARD  = 'Mega2560';    % Arduino Mega 2560

% --- Pin map (must match how you jumpered the NAND board) -----------------
PWM_PIN   = 'D9';       % Arduino PWM OUTPUT -> NAND "PWM in"
DIR_PIN   = 'D8';       % Arduino DIGITAL OUTPUT -> NAND "DIR in"
PWM_BACK  = 'A0';       % reads the PWM after the board (buffered readback)
IN1_PIN   = 'A1';       % reads NAND output IN1
IN2_PIN   = 'A2';       % reads NAND output IN2

% --- Supply / measurement -------------------------------------------------
VCC       = 5.0;        % logic-HIGH rail (V). MEASURE your 5 V rail and put it
                        % here so the level percentages are accurate.

% --- Test grid ------------------------------------------------------------
DUTIES    = 0:0.1:1;    % PWM duty sweep (0..1). 0 and 1 = pure logic 0/1 corners.
DIRS      = [1 0];      % test DIR = HIGH then DIR = LOW
NAVG      = 150;        % analog snapshots averaged per reading (see note above).
                        % With an RC filter on the analog pins, ~10 is plenty.
SETTLE_S  = 0.05;       % settle time after changing the inputs, before sampling

% --- What to do with the data ---------------------------------------------
CHARACTERIZE = true;    % classify each node in plain English (the useful part)
CHECK_MODEL  = false;   % also pass/fail against the expected model in section 2.
                        % Leave OFF until you have confirmed the real wiring from
                        % the characterization -- otherwise a healthy inverting
                        % board reports FAIL everywhere.
ABS_TOL_V = 0.35;       % model-check tolerance: absolute slack (V) ...
REL_TOL   = 0.10;       % ... + this fraction of the expected level

% --- Housekeeping ---------------------------------------------------------
CSV_FILE  = 'nand_debug.csv';   % where the raw measurements are saved


% ==========================================================================
%  2) EXPECTED MODEL  (only used when CHECK_MODEL = true)
% --------------------------------------------------------------------------
%  Default = the ACTIVE-LOW / INVERTING SN7400 behaviour confirmed on the bench:
%    * A0  = PWM buffered straight through (non-inverting)  -> duty*VCC
%    * IN active line = NOT PWM                             -> (1-duty)*VCC
%    * IN idle   line = HIGH                                -> VCC
%  Set which DIR state makes IN1 the active line with DIR_ACTIVE_HI below, then
%  turn CHECK_MODEL on. If your board turns out non-inverting, swap the (1-duty)
%  terms for duty and the VCC idle term for 0.
% ==========================================================================
DIR_ACTIVE_HI = 1;      % DIR value that makes IN1 the active (PWM-chopped) line
expA0  = @(duty, dir)  duty * VCC;                                   % PWM buffer
expIN1 = @(duty, dir)  ternary(dir == DIR_ACTIVE_HI, (1-duty)*VCC, VCC);
expIN2 = @(duty, dir)  ternary(dir ~= DIR_ACTIVE_HI, (1-duty)*VCC, VCC);


% ==========================================================================
%  3) CONNECT
% ==========================================================================
fprintf('Connecting to Arduino on %s ...\n', PORT);
a = arduino(PORT, BOARD);                 % re-flashes MATLAB server firmware
configurePin(a, DIR_PIN, 'DigitalOutput');
writeDigitalPin(a, DIR_PIN, 0);           % start in a known, safe state
writePWMDutyCycle(a, PWM_PIN, 0);
fprintf('Connected.\n\n');

fprintf('=== SN7400 NAND board characterization ===\n');
fprintf('  %s = PWM in,  %s = DIR in\n', PWM_PIN, DIR_PIN);
fprintf('  %s = PWM readback,  %s = IN1,  %s = IN2\n', PWM_BACK, IN1_PIN, IN2_PIN);
fprintf('  VCC = %.2f V,  NAVG = %d  (inverting/active-low expected)\n\n', VCC, NAVG);


% ==========================================================================
%  4) SWEEP the inputs and measure every output
% ==========================================================================
nPts = numel(DIRS) * numel(DUTIES);
res  = struct('dir',cell(nPts,1),'duty',[],'vA0',[],'vA1',[],'vA2',[]);
k = 0;

fprintf('%-14s %5s |   %-6s   %-6s   %-6s\n', 'DIR', 'duty', PWM_BACK, IN1_PIN, IN2_PIN);
fprintf('%s\n', repmat('-', 1, 52));
try
    for dir = DIRS
        writeDigitalPin(a, DIR_PIN, dir);
        for duty = DUTIES
            writePWMDutyCycle(a, PWM_PIN, duty);
            pause(SETTLE_S);

            vA0 = readAvgVoltage(a, PWM_BACK, NAVG);
            vA1 = readAvgVoltage(a, IN1_PIN,  NAVG);
            vA2 = readAvgVoltage(a, IN2_PIN,  NAVG);

            k = k + 1;
            res(k).dir = dir;  res(k).duty = duty;
            res(k).vA0 = vA0;  res(k).vA1  = vA1;  res(k).vA2 = vA2;

            dirName = ternary(dir == 1, 'DIR=HI', 'DIR=LO');
            fprintf('%-14s %4.0f%% |  %5.2f    %5.2f    %5.2f\n', ...
                    dirName, 100*duty, vA0, vA1, vA2);
        end
        fprintf('%s\n', repmat('-', 1, 52));
    end
catch runErr
    fprintf(2, '\n!! Aborted mid-run: %s\n', runErr.message);
end

safeIdle(a, PWM_PIN, DIR_PIN);      % always leave outputs idle (PWM off, DIR low)
res = res(1:k);                     % drop unused slots if aborted early


% ==========================================================================
%  5) CHARACTERIZE -- say, in plain English, what each probe is doing
% ==========================================================================
if CHARACTERIZE
    fprintf('\n=== What each probe actually reads ===\n');
    pinNames = {PWM_BACK, IN1_PIN, IN2_PIN};
    fields   = {'vA0', 'vA1', 'vA2'};
    for p = 1:numel(pinNames)
        fprintf('  %s:\n', pinNames{p});
        for dir = DIRS
            mask = ([res.dir]' == dir);
            dd   = [res(mask).duty]';
            vv   = [res(mask).(fields{p})]';
            fprintf('     DIR=%d : %s\n', dir, describeNode(dd, vv, VCC));
        end
    end
    fprintf(['\n  Reminder: a healthy SN7400 gives NON-inverting PWM on the\n' ...
             '  buffered readback, but INVERTED PWM (falls with duty) on the\n' ...
             '  active IN line, HIGH on the idle IN line, and a clean NOT-DIR on\n' ...
             '  the direction node. A node stuck at a fixed mid voltage that\n' ...
             '  ignores both inputs is almost certainly a loose/misplaced probe.\n']);
end


% ==========================================================================
%  6) OPTIONAL model pass/fail  (only meaningful once wiring is confirmed)
% ==========================================================================
if CHECK_MODEL
    fprintf('\n=== Model check (expected = section 2) ===\n');
    anyFail = false;
    for idx = 1:numel(res)
        d = res(idx).duty; dir = res(idx).dir;
        p0 = within(res(idx).vA0, expA0(d,dir),  ABS_TOL_V, REL_TOL);
        p1 = within(res(idx).vA1, expIN1(d,dir), ABS_TOL_V, REL_TOL);
        p2 = within(res(idx).vA2, expIN2(d,dir), ABS_TOL_V, REL_TOL);
        pass = p0 && p1 && p2;
        anyFail = anyFail || ~pass;
        if ~pass
            fprintf(2, '  FAIL  DIR=%d duty=%3.0f%%  A0 %.2f/%.2f  A1 %.2f/%.2f  A2 %.2f/%.2f\n', ...
                dir, 100*d, res(idx).vA0, expA0(d,dir), res(idx).vA1, expIN1(d,dir), ...
                res(idx).vA2, expIN2(d,dir));
        end
    end
    if anyFail
        fprintf(2, 'Model check: FAIL -- see rows above (or the model in section 2 is wrong).\n');
    else
        fprintf('Model check: PASS -- outputs match the expected truth table.\n');
    end
end


% ==========================================================================
%  7) SAVE + PLOT
% ==========================================================================
T = table([res.dir]', [res.duty]', [res.vA0]', [res.vA1]', [res.vA2]', ...
    'VariableNames', {'dir','duty','A0_V','A1_V','A2_V'});
writetable(T, CSV_FILE);
fprintf('\nSaved raw measurements to %s\n', CSV_FILE);

figure('Name','NAND board characterization','NumberTitle','off');
for j = 1:numel(DIRS)
    dir  = DIRS(j);
    mask = ([res.dir]' == dir);
    d    = 100*[res(mask).duty]';
    subplot(numel(DIRS), 1, j); hold on; grid on
    plot(d, [res(mask).vA0]', 'o-', 'LineWidth',1.2, 'DisplayName',[PWM_BACK ' PWM']);
    plot(d, [res(mask).vA1]', 's-', 'LineWidth',1.2, 'DisplayName',[IN1_PIN ' IN1']);
    plot(d, [res(mask).vA2]', '^-', 'LineWidth',1.2, 'DisplayName',[IN2_PIN ' IN2']);
    ylim([-0.3 VCC+0.3]); xlabel('commanded PWM duty (%)'); ylabel('avg voltage (V)');
    title(sprintf('DIR = %d', dir)); legend('Location','best');
end

fprintf('\nDone.\n');


% ==========================================================================
%  LOCAL FUNCTIONS
% ==========================================================================

function v = readAvgVoltage(a, pin, N)
% Average N unsynchronised readVoltage() snapshots. For a steady DC level this
% denoises; for a PWM/chopped line the average gives (fraction HIGH)*VCC.
    acc = 0;
    for n = 1:N
        acc = acc + readVoltage(a, pin);
    end
    v = acc / N;
end

function s = describeNode(duty, v, VCC)
% Classify what one output node does across the PWM duty sweep (fixed DIR).
%   tracks PWM      : voltage rises ~linearly with duty  (non-inverting)
%   tracks /PWM     : voltage FALLS with duty            (inverting NAND -- normal)
%   constant HIGH/LOW: flat near VCC / near 0
%   stuck mid-rail  : flat at an in-between voltage -> likely floating/misprobed
    duty = duty(:); v = v(:);
    p      = polyfit(duty, v, 1);       % slope of V over duty 0..1
    slope  = p(1);
    vmin   = min(v); vmax = max(v); vrange = vmax - vmin; vmean = mean(v);
    frac   = vmean / VCC;
    if slope > 0.6*VCC && vrange > 0.5*VCC
        s = sprintf('tracks PWM (rises %.2f->%.2f V) -- non-inverting', vmin, vmax);
    elseif slope < -0.6*VCC && vrange > 0.5*VCC
        s = sprintf('tracks /PWM (falls %.2f->%.2f V) -- inverting NAND (normal)', vmax, vmin);
    elseif vrange < 0.6                 % essentially flat vs duty
        if     frac > 0.70, s = sprintf('constant HIGH (~%.2f V)', vmean);
        elseif frac < 0.12, s = sprintf('constant LOW  (~%.2f V)', vmean);
        else,               s = sprintf('STUCK ~%.2f V, ignores PWM -- floating/misprobed? check this wire', vmean);
        end
    else
        s = sprintf('indeterminate (%.2f..%.2f V, slope %.2f)', vmin, vmax, slope);
    end
end

function tf = within(meas, expected, absTol, relTol)
% True if a measurement is within (absTol + relTol*|expected|) of expected.
    tf = abs(meas - expected) <= (absTol + relTol*abs(expected));
end

function out = ternary(cond, a, b)
% Tiny inline if-else for readable one-liners.
    if cond, out = a; else, out = b; end
end

function safeIdle(a, PWM_PIN, DIR_PIN)
% Leave the board outputs in a safe, quiet state.
    try
        writePWMDutyCycle(a, PWM_PIN, 0);
        writeDigitalPin(a, DIR_PIN, 0);
    catch
    end
end
