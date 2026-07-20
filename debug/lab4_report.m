%% MSE 312 Lab 4 -- Motor System ID, speed PID, and position control
%
% This report loads the encoder step-test data captured by |lab4_sysID_2.m|,
% identifies the first-order motor model, and then DESIGNS and SIMULATES two
% controllers:
%
% # a *speed* controller (PI) designed directly from the model, and
% # a *position* controller (PD) built by adding an integrator to the plant,
%   using the *same design method and controller code*, with the target angle
%   bounded to |[0, 2*pi)|.
%
% Because the model is already identified, the gains are chosen DIRECTLY (pick the
% desired closed-loop speed / damping and solve) -- a tuner is optional. A
% Ziegler-Nichols option is kept for comparison via |DESIGN_METHOD = 'zn'|.
%
% *Important:* this lab board's digital output pins are dead (see
% |MSE312_MCU_Protection.pdf|), so the motor is driven by an external driver and
% the board only logs the encoder. The controllers below are therefore designed
% and validated *in simulation* against the identified model -- they are meant
% for the replacement board, not this one.
%
% To store this as a report:  |publish('lab4_report.m')|  (HTML) or
% |publish('lab4_report.m','pdf')|.  To work interactively, open it and use the
% section breaks (|%%|) as a Live Script, or "Save As" a |.mlx|.

clear; clc; close all

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])

% Do not delete the above lines, they are used to run this script


%% 1. Load the captured step-test data
% We analyse the CSV that |lab4_sysID_2.m| saved (time, encoder count, angle,
% angular velocity, and the reconstructed input voltage). Run that script first
% if the file is missing.
CSV_FILE = 'lab4_sysID_2.csv';
if exist(CSV_FILE,'file') ~= 2
    error('Data file %s not found. Run lab4_sysID_2.m first to capture a step test.', CSV_FILE);
end
D     = readtable(CSV_FILE);
t     = D.t_s;
omega = D.omega_rad_s;
vin   = D.vin_V;

figure('Name','Captured step test');
subplot(2,1,1); plot(t, vin,   'LineWidth',1.2); grid on
    ylabel('V_{in} (V)'); title('Input voltage (reconstructed step)');
subplot(2,1,2); plot(t, omega, 'LineWidth',1.2); grid on
    ylabel('\omega (rad/s)'); xlabel('time (s)'); title('Measured angular velocity');


%% 2. Identify the first-order model  omega/V = K/(tau*s + 1)
% |K| is the DC gain (steady speed per volt) and |tau| the time constant. We use
% the step-response hand calc always, and the System Identification Toolbox
% (|tfest|) when it is available -- preferring the |tfest| numbers for the model.
[K_hand, tau_hand] = manualFirstOrderFit(t, vin, omega);
fprintf('Hand calc : K = %.4f (rad/s)/V,  tau = %.4f s\n', K_hand, tau_hand);

K = K_hand;  tau = tau_hand;                 % fallback = hand calc
Ts = median(diff(t));                        % actual sample time from the data
if license('test','Identification_Toolbox') && exist('tfest','file') == 2
    tu   = (t(1):Ts:t(end))';
    omU  = interp1(t, omega, tu, 'linear');
    vinU = interp1(t, vin,   tu, 'linear');
    sysId = tfest(iddata(omU, vinU, Ts), 1, 0);
    K   = dcgain(sysId);
    tau = -1/real(pole(sysId));
    fprintf('tfest fit : K = %.4f (rad/s)/V,  tau = %.4f s\n', K, tau);
end

G = tf(K, [tau 1]);                          % identified plant: omega/V
disp('Identified plant G(s) = omega/V:'); G  %#ok<NOPTS>


%% 3. Speed controller -- direct model-based design (ZN tuner optional)
% With the model in hand we pick the gains DIRECTLY: choose how fast you want the
% closed loop, and solve for Kp, Ki. No tuner needed. Ziegler-Nichols is kept only
% as an optional empirical alternative for comparison.
if ~license('test','Control_Toolbox')
    error('Control System Toolbox is required for the controller design sections.');
end

DESIGN_METHOD = 'direct';    % 'direct' = model-based (recommended) | 'zn' = Ziegler-Nichols

% We still model the digital loop's latency (ZOH + velocity-filter lag + one
% compute step ~ 2 samples). It barely affects the direct design, but it makes
% the simulation realistic -- and it is what gives ZN a finite ultimate gain.
LOOP_DELAY = 2*Ts;                           % modeled loop latency (s)
[nDelay, dDelay] = pade(LOOP_DELAY, 1);      % 1st-order Pade of the delay (num, den)
Delay = tf(nDelay, dDelay);                  % as a transfer function for margin/step/feedback

% Direct-design target for the speed loop:
LAMBDA_SPEED = tau/3;        % desired speed-loop time constant (s): smaller = faster.
                            % Keep it a few x LOOP_DELAY so it stays achievable.

switch lower(DESIGN_METHOD)
    case 'direct'
        [KpS, KiS, KdS] = designSpeedPI(K, tau, LAMBDA_SPEED);
        fprintf('\nSPEED PI (direct, lambda = %.4f s):  Kp = %.4f, Ki = %.4f\n', ...
                LAMBDA_SPEED, KpS, KiS);
    case 'zn'
        [KpS, KiS, KdS, KuS, PuS] = znTunePID(G*Delay);
        fprintf('\nSPEED PID (ZN):  Ku = %.3f, Pu = %.4f  ->  Kp = %.4f, Ki = %.4f, Kd = %.5f\n', ...
                KuS, PuS, KpS, KiS, KdS);
    otherwise
        error('DESIGN_METHOD must be ''direct'' or ''zn''.');
end

Cspeed = pid(KpS, KiS, KdS);
Tspeed = feedback(Cspeed*G*Delay, 1);        % closed-loop speed response (delay included)

figure('Name','Speed controller -- closed-loop step');
step(Tspeed); grid on
title(sprintf('Speed loop (%s):  K_p=%.3g, K_i=%.3g, K_d=%.3g', DESIGN_METHOD, KpS, KiS, KdS));
ylabel('\omega / \omega_{ref}');

siS = stepinfo(Tspeed);
fprintf('  rise = %.3f s, settle = %.3f s, overshoot = %.1f %%\n', ...
        siS.RiseTime, siS.SettlingTime, siS.Overshoot);

%%
% For a first-order plant the direct PI *cancels the plant pole* (Ti = tau) and
% places one closed-loop pole at 1/lambda -- a clean first-order response with no
% overshoot, at a speed you set directly. No derivative term is used: the plant
% is first order, and D would only amplify the noise already in the differentiated
% encoder velocity.


%% 4. Position controller -- add an integrator, reuse the SAME structure + method
% Position is the integral of speed, so the position plant is the speed plant in
% series with an integrator:  Gp(s) = K / ( s*(tau*s + 1) ). We design it with the
% SAME method chosen above (direct pole-placement, or the same ZN tuner).
Gp = G * tf(1, [1 0]);                       % K / (s(tau s + 1))

% Direct-design targets for the position loop:
ZETA_POS = 0.8;             % closed-loop damping (0.7-1 -> little/no overshoot)
WN_POS   = 0.3/LOOP_DELAY;  % closed-loop natural frequency (rad/s); the loop delay
                           % caps how high this can go before the loop rings.

switch lower(DESIGN_METHOD)
    case 'direct'
        [KpP, KiP, KdP] = designPositionPD(K, tau, ZETA_POS, WN_POS);
        fprintf('\nPOSITION PD (direct, zeta = %.2f, wn = %.2f rad/s):  Kp = %.4f, Kd = %.5f\n', ...
                ZETA_POS, WN_POS, KpP, KdP);
    case 'zn'
        [KpP, KiP, KdP, KuP, PuP] = znTunePID(Gp*Delay);
        fprintf('\nPOSITION PID (ZN):  Ku = %.3f, Pu = %.4f  ->  Kp = %.4f, Ki = %.4f, Kd = %.5f\n', ...
                KuP, PuP, KpP, KiP, KdP);
end

Cpos = pid(KpP, KiP, KdP);
Tpos = feedback(Cpos*Gp*Delay, 1);           % closed-loop position response

%%
% The commanded angle is bounded to one revolution, |[0, 2*pi)|. Pick a target in
% range (we clamp it defensively) and simulate the move from 0.
POS_REF = pi;                                % target angle (rad)
POS_REF = min(max(POS_REF, 0), 2*pi - eps);  % enforce 0 <= target < 2*pi

[yUnit, tt] = step(Tpos, 8*max(1/WN_POS, tau));  % unit-reference response
yPos = POS_REF * yUnit;                          % scale to the actual target

figure('Name','Position controller -- move to bounded target');
plot(tt, yPos, 'LineWidth',1.4); grid on; hold on
yline(POS_REF, 'k--', 'target');
yline(2*pi, 'r:', '2\pi bound');  yline(0, 'r:');
ylabel('\theta (rad)'); xlabel('time (s)');
title(sprintf('Position loop (%s): move to %.3f rad  (bounded [0, 2\\pi))', DESIGN_METHOD, POS_REF));

siP = stepinfo(Tpos);
fprintf('  rise = %.3f s, settle = %.3f s, overshoot = %.1f %%\n', ...
        siP.RiseTime, siP.SettlingTime, siP.Overshoot);

%%
% *Note on the integrator.* The position plant already contains one integrator
% (speed -> position), so a P-D controller reaches a step target with zero
% steady-state error WITHOUT adding a second integrator -- which is why the direct
% design uses Ki = 0 and cannot wind up. (The ZN option does add integral action,
% making the loop type-2 and more prone to overshoot; if you use it on the real
% board, keep the setpoint in [0, 2*pi) and enable anti-windup -- the firmware's
% PID already clamps its integrator.)


%% 5. Summary
fprintf('\n================ SUMMARY ================\n');
fprintf('Identified model : K = %.4f (rad/s)/V,  tau = %.4f s\n', K, tau);
fprintf('Design method    : %s\n', DESIGN_METHOD);
fprintf('Modeled loop delay: %.4f s (%.1f samples)\n', LOOP_DELAY, LOOP_DELAY/Ts);
fprintf('Speed  controller: Kp=%.4f  Ki=%.4f  Kd=%.5f\n', KpS, KiS, KdS);
fprintf('Pos.   controller: Kp=%.4f  Ki=%.4f  Kd=%.5f\n', KpP, KiP, KdP);
fprintf('=========================================\n');


%% Local functions
% (the reusable math -- identification, direct design, and the optional ZN tuner)

function [Kp, Ki, Kd] = designSpeedPI(K, tau, lambda)
% Direct (IMC / lambda) design for the first-order plant G = K/(tau*s+1).
% A PI controller whose zero cancels the plant pole gives a first-order closed
% loop of time constant lambda:
%     Ti = tau,  Kp = tau/(K*lambda),  Ki = Kp/Ti = 1/(K*lambda),  Kd = 0.
% No derivative term -- the plant is first order and D would only amplify the
% noise in the differentiated encoder velocity.
    Kp = tau / (K*lambda);
    Ki = 1   / (K*lambda);
    Kd = 0;
end

function [Kp, Ki, Kd] = designPositionPD(K, tau, zeta, wn)
% Direct pole-placement for the position plant Gp = K/(s(tau*s+1)) with a PD
% controller C = Kp + Kd*s. The closed-loop characteristic polynomial is
%     tau*s^2 + (1 + K*Kd)*s + K*Kp = 0,
% matched to the standard  s^2 + 2*zeta*wn*s + wn^2 :
%     Kp = tau*wn^2 / K,   Kd = (2*zeta*wn*tau - 1) / K,   Ki = 0.
% No integral term -- the plant already integrates, so P-D gives zero steady-state
% error to a step target without any risk of integrator windup.
    Kp = tau*wn^2 / K;
    Kd = (2*zeta*wn*tau - 1) / K;
    Ki = 0;
    if Kd < 0
        warning(['designPositionPD: wn is low enough that the plant already exceeds ' ...
                 'the target damping (Kd<0). Clamping Kd=0 (P-only); raise WN_POS for PD.']);
        Kd = 0;
    end
end

function [Kp, Ki, Kd, Ku, Pu] = znTunePID(Gloop)
% OPTIONAL Ziegler-Nichols ultimate-gain tuning (empirical; not needed once the
% model is known). Gloop is the open-loop process under proportional control,
% INCLUDING the modeled loop delay. The ultimate gain Ku is the gain that makes
% the loop oscillate -- exactly the gain margin of Gloop -- and Pu = 2*pi/Wcg.
    [Gm, ~, Wcg] = margin(Gloop);
    if ~isfinite(Gm) || ~isfinite(Wcg) || Wcg <= 0
        error(['No finite ultimate gain: this loop cannot be driven to sustained ' ...
               'oscillation, so Ziegler-Nichols does not apply. Add loop delay/lag.']);
    end
    Ku = Gm;                 % ultimate (marginal-stability) proportional gain
    Pu = 2*pi / Wcg;         % ultimate oscillation period

    % Classic ZN table for a parallel PID:  Kp=0.6Ku, Ti=Pu/2, Td=Pu/8
    Kp = 0.6 * Ku;
    Ti = 0.5 * Pu;
    Td = 0.125 * Pu;
    Ki = Kp / Ti;            % = 1.2*Ku/Pu
    Kd = Kp * Td;            % = 0.075*Ku*Pu
end

function [K, tau] = manualFirstOrderFit(t, vin, omega)
% Step-response hand calc for K/(tau*s+1):
%   K   = (steady-state speed change) / (voltage step)
%   tau = time after the step to reach 63.2% of that speed change.
    dv    = diff(vin);
    kStep = find(abs(dv) > 1e-6, 1, 'first');
    if isempty(kStep)
        Vstep = vin(end) - vin(1);  tStep = t(1);
    else
        Vstep = vin(kStep+1) - vin(kStep);  tStep = t(kStep);
    end
    if abs(Vstep) < 1e-6, Vstep = 1; end

    tailIdx = t >= (t(1) + 0.8*(t(end)-t(1)));
    omegaSS = mean(omega(tailIdx));
    omega0  = mean(omega(t < tStep));
    dOmega  = omegaSS - omega0;

    K = dOmega / Vstep;

    target    = omega0 + 0.632*dOmega;
    afterStep = find(t >= tStep);
    if dOmega >= 0
        hit = afterStep(omega(afterStep) >= target);
    else
        hit = afterStep(omega(afterStep) <= target);
    end
    if isempty(hit), tau = NaN; else, tau = t(hit(1)) - tStep; end
end
