clear all; clc; close all

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])

%cd CompileFolder % move to CompileFolder on start (optional)

% Do not delete the above lines, they are used to run this script

% ==========================================================================
%  lab4_sysID_2 -- System ID with AUTO step-onset detection (no stopwatch)
% --------------------------------------------------------------------------
%  Same as lab4_sysID.m, but you no longer have to flip the driver at an exact
%  time. This version finds the step start ITSELF from the first shaft motion:
%  since this motor has NO transport dead time (it begins turning the instant
%  voltage is applied), the first non-zero encoder movement IS the moment the
%  step happened. So the operator just flips the switch whenever, and the script
%  reconstructs the step at the detected instant.
%
%  WHAT THIS DOES
%    The board is an ENCODER LOGGER only (its output pins are dead). The motor is
%    driven by a SEPARATE external driver + supply, isolated except for the shaft.
%    This script logs the encoder, detects when the shaft first moves, treats that
%    as t_step, saves a CSV, and fits the first-order model
%
%          omega(s)     K
%          -------- = -------      (angular velocity per volt)
%           V(s)      tau*s + 1
%
%  HOW TO USE (step test -- no timing needed)
%    1. Wire the encoder to the Arduino only:  A->D2, B->D3, Vcc->5V, GND->GND.
%    2. Pre-set the external driver to V_STEP volts, motor power switch OFF.
%    3. Run this script. Let the motor sit STILL for ~1 s (this captures a rest
%       baseline), then flip the driver ON whenever you like -- no exact time.
%    4. When logging ends, switch the driver OFF. The script auto-detects the step
%       start, reconstructs the input, saves the CSV, and fits K and tau.
%
%  NOTE: creating the arduino() object below re-flashes MATLAB's server firmware
%  onto the board, overwriting the custom MSE312 firmware -- fine here, we only
%  need to read the encoder. Re-upload the .hex later to get the firmware back.
% ==========================================================================


% ==========================================================================
%  1) SETTINGS  -- edit these, then just press Run
% ==========================================================================
PORT        = 'COM5';        % the board is on COM5 (close any serial monitor first)
BOARD       = 'Mega2560';    % Arduino Mega 2560

% --- Encoder + gearbox (encoder is on the MOTOR shaft, before the gearbox) ---
ENC_LINES   = 500;           % encoder lines (counts/rev on one channel)
ENC_QUAD    = 4;             % X4 quadrature decoding
GEAR_RATIO  = 6.3;           % gearbox ratio (motor : output shaft)
COUNTS_PER_REV = ENC_QUAD * ENC_LINES * GEAR_RATIO;   % = 12600 edges per OUTPUT-shaft rev

% --- Logging ---
SAMPLE_RATE = 100;           % target samples/sec (100 Hz). USB may run slower; that's OK,
                             % every sample is time-stamped so the math uses the REAL dt.
DURATION    = 12;            % seconds to log (rest baseline + full spin-up transient)
VEL_FILTER_C = 0.01;         % velocity filter time constant (s), matches the firmware

% --- How we know the motor's INPUT voltage --------------------------------
%   'step' = known step, start time AUTO-DETECTED from the first shaft motion.
%   'A0'   = measure the drive voltage on A0 through a divider (needs shared GND).
INPUT_MODE  = 'step';

% ...settings for INPUT_MODE = 'step'
V_STEP      = 4.0;           % voltage you step the external driver to. Any KNOWN voltage
                             % gives K and tau -- it need not be the full 12 V. Keep it
                             % moderate: a hard step to full 12 V spins the motor to ~4500 rpm
                             % (~150k encoder edges/s), which floods MATLAB's USB link and
                             % drops the board ("Unable to receive data"). Raise only after
                             % hardening. (Partial PWM duty? real voltage = duty*12.)
MOTION_COUNTS = 2;           % shaft must move at least this many encoder counts to count as
                             % "the step started" -- 2 rejects a lone noise edge while still
                             % catching motion within the first millisecond (no dead time).
                             % Set to 1 for literally the first non-zero count.

% ...settings for INPUT_MODE = 'A0'  (motor up to 12 V -> divide >=3x to stay <=5 V)
A0_DIVIDER_RATIO = 3.0;      % actual volts = readVoltage * this ratio

% --- Coast-down test: infer tau with NO known input voltage ---------------
%   Spin the motor up to any speed, then CUT the external power partway through
%   the log. The motor coasts down as omega(t)=omega0*exp(-t/tau), so tau falls
%   straight out of the velocity DECAY -- the input voltage never enters.
COASTDOWN_TEST = false;      % true = also estimate tau from the coast-down decay

% --- Output + fitting ---
CSV_FILE    = 'lab4_sysID_2.csv'; % where the raw data is saved
DO_FIT      = true;               % fit K/(tau*s+1) after logging (needs the input)


% ==========================================================================
%  2) CONNECT to the board and the encoder
% ==========================================================================
fprintf('Connecting to Arduino on %s ...\n', PORT);
a = arduino(PORT, BOARD);                                  % re-flashes MATLAB server firmware
encoder = rotaryEncoder(a, 'D2', 'D3', ENC_LINES);         % quadrature encoder on D2/D3
resetCount(encoder);                                       % call the current position "0"
fprintf('Connected. Encoder zeroed.\n\n');

useA0 = strcmpi(INPUT_MODE, 'A0');


% ==========================================================================
%  3) LOG the encoder while the motor spins
% ==========================================================================
dt      = 1 / SAMPLE_RATE;                 % target seconds between samples
Nmax    = ceil(DURATION / dt) + 100;       % preallocate a bit extra, we trim later
t       = zeros(Nmax, 1);                   % time stamp (s)
count   = zeros(Nmax, 1);                   % raw encoder count (signed, continuous)
vinRaw  = zeros(Nmax, 1);                   % raw A0 reading (V at the pin), if used

fprintf('=== Get ready ===\n');
if useA0
    fprintf('INPUT_MODE = A0: logging the drive voltage measured on A0.\n');
    fprintf('Start the external driver whenever you like.\n\n');
else
    fprintf('INPUT_MODE = step: pre-set the driver to %.1f V, motor switch OFF.\n', V_STEP);
    fprintf('Logging now -- let it REST ~1 s, then flip the driver ON whenever.\n');
    fprintf('(No timing needed: the step start is found from the first shaft motion.)\n\n');
end

i        = 0;             % how many samples we have stored
t0       = tic;           % start the clock
try
    while toc(t0) < DURATION && i < Nmax
        i        = i + 1;
        t(i)     = toc(t0);                        % real elapsed time for THIS sample
        count(i) = readCount(encoder);            % accumulated x4 count (continuous, signed)
        if useA0
            vinRaw(i) = readVoltage(a, 'A0');     % 0..5 V at the pin (scaled to real volts later)
        end
        pause(dt);        % ~target rate; drains the encoder stream so it can't overflow
    end
catch loopErr
    % Lost the board mid-run -- almost always the full-speed encoder interrupt
    % rate or the motor turn-on transient knocking out the USB link. Keep the
    % samples we already have instead of throwing the whole run away.
    i = i - 1;            % the sample being read when it dropped is incomplete
    fprintf(2, '\n!! Board stopped responding mid-run: %s\n', loopErr.message);
    fprintf(2, ['   Likely the motor turn-on transient / full-speed interrupts.\n' ...
                '   Lower V_STEP or ramp up gently, then before rerunning:\n' ...
                '     clear a encoder\n\n']);
end

% Trim the preallocated arrays down to the samples we actually collected.
i      = max(i, 0);
t      = t(1:i);
count  = count(1:i);
vinRaw = vinRaw(1:i);
if i < 2
    error(['Only %d usable sample(s) -- nothing to identify. See the message ' ...
           'above, lower V_STEP, and rerun after:  clear a encoder'], i);
end
fprintf('\nLogged %d samples over %.2f s (average %.0f Hz).\n', i, t(end), i/t(end));
if ~useA0
    fprintf('You can switch the external driver OFF now.\n');
end


% ==========================================================================
%  3.5) DETECT the step onset from the first shaft motion (step mode)
% --------------------------------------------------------------------------
%  With no transport dead time, the first sample where the shaft has moved marks
%  the instant the step voltage was applied -- far more accurate than trusting a
%  hand-timed switch. tStep is that instant.
% ==========================================================================
if useA0
    tStep = t(1);                                   % not used to reconstruct input in A0 mode
else
    kStep = detectStepOnset(count, MOTION_COUNTS);
    if isempty(kStep)
        error(['The shaft never moved (count stayed near 0). Did the driver switch ' ...
               'on?\n Check the switch / stiction deadband, then rerun after: clear a encoder']);
    end
    tStep = t(kStep);
    fprintf('Detected step start at t = %.3f s (sample %d, moved %d counts).\n', ...
            tStep, kStep, MOTION_COUNTS);
    if kStep <= 2
        warning(['Motion began at the very first sample -- no rest baseline captured.\n' ...
                 'Next time let it sit still ~1 s before flipping the driver.']);
    end
end


% ==========================================================================
%  4) TURN COUNTS INTO ANGLE + ANGULAR VELOCITY
% ==========================================================================
% Continuous (unwrapped) output-shaft angle, in radians:  theta = count*2*pi/12600
theta = count * 2*pi / COUNTS_PER_REV;

% Angular velocity = filtered derivative of the CONTINUOUS angle (never the
% wrapped one). Same "dirty derivative" s/(c*s+1) the firmware uses, but here we
% use each sample's REAL dt so USB timing jitter doesn't distort the speed.
omega = filteredVelocity(t, theta, VEL_FILTER_C);

% Wrapped angle in [0, 2*pi) -- for DISPLAY only, computed after the derivative.
thetaWrap = mod(theta, 2*pi);

% The motor INPUT voltage over time.
if useA0
    vin = vinRaw * A0_DIVIDER_RATIO;                 % measured, scaled back up
else
    vin = buildStepInput(t, tStep, V_STEP);          % reconstructed at the DETECTED onset
end


% ==========================================================================
%  5) SAVE the raw data to CSV (columns: t_s, count, theta_rad, omega_rad_s, vin_V)
% ==========================================================================
T = table(t, count, theta, omega, vin, ...
    'VariableNames', {'t_s','count','theta_rad','omega_rad_s','vin_V'});
writetable(T, CSV_FILE);
fprintf('Saved raw data to %s\n', CSV_FILE);


% ==========================================================================
%  6) PLOT what we recorded  (dotted line marks the detected step onset)
% ==========================================================================
figure('Name','lab4 system ID -- raw log','NumberTitle','off');
subplot(3,1,1); plot(t, vin,       'LineWidth',1.2); grid on
    if ~useA0, xline(tStep, 'k:', 'step start'); end
    ylabel('V_{in} (V)');  title('Motor input voltage (step at detected onset)');
subplot(3,1,2); plot(t, thetaWrap, 'LineWidth',1.2); grid on
    ylabel('\theta (rad)'); title('Output-shaft angle (wrapped 0..2\pi)');
subplot(3,1,3); plot(t, omega,     'LineWidth',1.2); grid on
    if ~useA0, xline(tStep, 'k:'); end
    ylabel('\omega (rad/s)'); xlabel('time (s)'); title('Angular velocity');


% ==========================================================================
%  7) FIT the first-order model  omega/V = K/(tau*s + 1)
% ==========================================================================
if DO_FIT
    fprintf('\n=== First-order model fit  omega/V = K/(tau*s + 1) ===\n');

    % --- (a) hand-calculation from the step response (always available) -----
    % K   = steady-state speed change / voltage change
    % tau = time to reach 63.2%% of that steady-state speed change
    [K_hand, tau_hand] = manualFirstOrderFit(t, vin, omega);
    fprintf('Hand calc : K = %.4f (rad/s)/V,  tau = %.4f s\n', K_hand, tau_hand);

    % --- (b) System Identification Toolbox fit (tfest), if available --------
    if license('test','Identification_Toolbox') && exist('tfest','file') == 2
        % tfest needs uniformly-sampled data; resample onto an even time grid.
        tu    = (t(1):dt:t(end))';
        omU   = interp1(t, omega, tu, 'linear');
        if useA0
            vinU = interp1(t, vin, tu, 'linear');
        else
            vinU = V_STEP * (tu >= tStep);          % exact step at the detected onset
        end

        data  = iddata(omU, vinU, dt);          % (output, input, sample time)
        sys   = tfest(data, 1, 0);              % 1 pole, 0 zeros -> K/(tau*s+1)

        K_fit   = dcgain(sys);                  % DC gain = K
        tau_fit = -1 / real(pole(sys));         % time constant = -1/pole
        fprintf('tfest fit : K = %.4f (rad/s)/V,  tau = %.4f s\n', K_fit, tau_fit);

        % Overlay the model's predicted velocity on the measured velocity.
        omHat = lsim(sys, vinU, tu);
        figure('Name','lab4 system ID -- model fit','NumberTitle','off');
        plot(t, omega, 'LineWidth',1.2); hold on
        plot(tu, omHat, '--', 'LineWidth',1.6); grid on
        if ~useA0, xline(tStep, 'k:', 'step start'); end
        xlabel('time (s)'); ylabel('\omega (rad/s)');
        legend('measured','model K/(\tau s+1)','Location','best');
        title(sprintf('Fit:  K = %.3f (rad/s)/V,  \\tau = %.3f s', K_fit, tau_fit));
    else
        fprintf(['(System Identification Toolbox not found -- showing the hand ' ...
                 'calc only.\n Install it for the tfest fit, or use K/tau above.)\n']);
    end
end


% ==========================================================================
%  8) COAST-DOWN estimate of tau  (uses ONLY the encoder -- no input voltage)
% ==========================================================================
if COASTDOWN_TEST
    fprintf('\n=== Coast-down estimate (input-free) ===\n');
    [tau_coast, R2, tPeak, sPeak] = coastDownTau(t, omega);
    if isnan(tau_coast)
        fprintf(['No clear coast-down found. Spin the motor up, then CUT the ' ...
                 'power\n partway through the log so it can decelerate freely.\n']);
    else
        fprintf('tau (coast-down) = %.4f s   (log-fit R^2 = %.3f)\n', tau_coast, R2);

        figure('Name','lab4 system ID -- coast-down','NumberTitle','off');
        plot(t, abs(omega), 'LineWidth',1.2); hold on
        td = t(t >= tPeak) - tPeak;
        plot(t(t >= tPeak), sPeak*exp(-td/tau_coast), '--', 'LineWidth',1.6); grid on
        xlabel('time (s)'); ylabel('|\omega| (rad/s)');
        legend('measured |\omega|','fit \omega_{peak} e^{-t/\tau}','Location','best');
        title(sprintf('Coast-down:  \\tau = %.3f s', tau_coast));
    end
end

fprintf('\nDone.\n');


% ==========================================================================
%  LOCAL FUNCTIONS  (the reusable math -- you rarely need to touch these)
% ==========================================================================

function kStep = detectStepOnset(count, motionCounts)
% First sample where the shaft has moved at least motionCounts encoder counts
% away from its resting value. With no transport dead time this IS the instant
% the step voltage was applied. Returns [] if the shaft never moved.
    moved = abs(count - count(1));                  % counts of motion since rest
    kStep = find(moved >= motionCounts, 1, 'first');
end

function omega = filteredVelocity(t, theta, c)
% Filtered derivative of angle -> angular velocity, using the "dirty derivative"
% transfer function  H(s) = s / (c*s + 1)  (differentiate, then low-pass by c).
% Backward-Euler step, evaluated with each sample's real dt:
%     omega(k) = ( c*omega(k-1) + (theta(k)-theta(k-1)) ) / (c + dt)
    omega = zeros(size(theta));
    for k = 2:numel(theta)
        dtk        = t(k) - t(k-1);
        dtheta     = theta(k) - theta(k-1);
        omega(k)   = (c*omega(k-1) + dtheta) / (c + dtk);
    end
end

function vin = buildStepInput(t, tStep, vStep)
% Reconstruct the known step: 0 V before tStep, vStep V after.
    vin = zeros(size(t));
    vin(t >= tStep) = vStep;
end

function [K, tau] = manualFirstOrderFit(t, vin, omega)
% Classic step-response hand calc for K/(tau*s+1):
%   K   = (steady-state speed change) / (voltage step)
%   tau = time after the step to reach 63.2%% of that speed change.
% Robust to which direction the motor turns (uses signed magnitudes).

    % When and how big was the voltage step?
    dv      = diff(vin);
    kStep   = find(abs(dv) > 1e-6, 1, 'first');     % first sample where V changes
    if isempty(kStep)
        Vstep = vin(end) - vin(1);                  % A0 mode: overall change
        tStep = t(1);
    else
        Vstep = vin(kStep+1) - vin(kStep);
        tStep = t(kStep);
    end
    if abs(Vstep) < 1e-6, Vstep = 1; end            % avoid divide-by-zero

    % Steady-state speed = average of the last 20%% of the record.
    tailIdx = t >= (t(1) + 0.8*(t(end)-t(1)));
    omegaSS = mean(omega(tailIdx));
    omega0  = mean(omega(t < tStep));               % speed before the step (~0)
    dOmega  = omegaSS - omega0;

    K = dOmega / Vstep;

    % tau = first time AFTER the step that omega crosses 63.2%% of its final change.
    target = omega0 + 0.632*dOmega;
    afterStep = find(t >= tStep);
    if dOmega >= 0
        hit = afterStep(omega(afterStep) >= target);
    else
        hit = afterStep(omega(afterStep) <= target);
    end
    if isempty(hit)
        tau = NaN;                                  % never reached 63.2%% (record too short)
    else
        tau = t(hit(1)) - tStep;
    end
end

function [tau, R2, tPeak, sPeak] = coastDownTau(t, omega)
% Estimate tau from a free coast-down using ONLY the velocity samples -- no
% input voltage. During coast the motor obeys  omega(t) = omega_peak*exp(-t/tau),
% so ln|omega| falls linearly at slope -1/tau. We find the peak speed, then fit
% that line over the clean part of the decay (between 10%% and 90%% of the peak),
% staying off the flat top and the noisy near-zero tail.
%
% Note: real friction has a Coulomb (constant-drag) part, so the very end of the
% decay bends away from a pure exponential -- fitting the UPPER portion (as we
% do here) captures the viscous time constant, which is what tau means.
    s = abs(omega);
    [sPeak, kPeak] = max(s);
    tPeak = t(kPeak);

    idx = ((1:numel(s))' > kPeak) & (s < 0.9*sPeak) & (s > 0.1*sPeak);
    tt  = t(idx) - tPeak;
    ss  = s(idx);
    if numel(ss) < 5                    % no clear decay in the record
        tau = NaN; R2 = NaN; return
    end

    p   = polyfit(tt, log(ss), 1);      % slope p(1) = -1/tau
    tau = -1 / p(1);

    yhat = polyval(p, tt);              % goodness of fit of the log-linear model
    R2   = 1 - sum((log(ss)-yhat).^2) / sum((log(ss)-mean(log(ss))).^2);
end
