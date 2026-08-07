%PARAMS  Load all launcher control parameters into the base workspace.
%   Run this before the Simulink model (or set it as the model InitFcn:
%   Model Properties -> Callbacks -> InitFcn: params). It stitches the fitted
%   system-ID structs into the exact variable names the blocks reference, and
%   falls back to safe defaults (with a warning) for anything not yet fitted.
%
%   Populate the fitted structs first by running:
%     sysid_motor_speed;  sysid_pressure_build;  sysid_leak;  fit_plant_VtoP;
%     build_inversion_table;
%   (they leave motorModel / plantModel / leakModel / InversionTables in base).

%% ---------- solver / timing ----------
Ts          = 0.01;      % [s] fixed step -- match every block & the solver
FIRE_DWELL  = 0.20;      % [s] solenoid one-shot pulse length
DEBOUNCE_S  = 0.05;      % [s] IR-sensor debounce
COMPRESS_TIMEOUT = 8.0;  % [s] abort COMPRESS if P_target never reached

%% ---------- pin map (documentation; set the same in each block dialog) ----------
PIN_PWM   = 8;    % motor PWM  (D8)
PIN_DIR   = 7;    % motor direction (D7)
PIN_FIRE  = 12;   % solenoid  (D12)
PIN_IR1   = 22;   % ball sensor 1 -> WOODEN  (set to your wired pin)
PIN_IR2   = 24;   % ball sensor 2 -> BOUNCY
PIN_LED_WOOD   = 26;
PIN_LED_BOUNCY = 28;
ADC_PRESS = 2;    % analog A2

%% ---------- pressure sensor ADC -> PSI (220 ohm sense) ----------
R_SENSE   = 220;                 % [ohm]
ADC_VREF  = 5.0;  ADC_MAX = 1023;
% PSI = ((raw*ADC_VREF/ADC_MAX)/R_SENSE*1000 - 4)/16*100
% (use these constants inside the MATLAB Function conversion block)

%% ---------- plant model (voltage -> pressure) ----------
if exist('plantModel','var')
    kv  = plantModel.kv;  Vdb = plantModel.Vdb;  c = plantModel.c;
else
    warning('params:noPlant','plantModel not found -- using DEFAULTS. Run fit_plant_VtoP first.');
    kv = 0.9;  Vdb = 2.0;  c = 0.05;      % PSI/s per V, V, 1/s
end

%% ---------- actuator limits & duty mapping ----------
U_MAX     = 12;                  % [V] working voltage limit (set 6 if you cap lower)
DUTY_MAX  = 255;
V_TO_DUTY = DUTY_MAX / U_MAX;    % volts command -> PWM counts

%% ---------- PI starting gains (IMC on first-order kv/(s+c)) ----------
% plant DC gain Kdc = kv/c, time constant tau_p = 1/c.
% IMC-PI:  Ti = tau_p,  Kp = tau_p/(Kdc*lambda) = 1/(kv*lambda),  Ki = Kp/Ti.
lambda = 1/max(c,1e-3);          % desired closed-loop time constant (~open loop; make smaller = faster)
Kp = 1/(max(kv,1e-3)*lambda);
Ki = Kp * c;                     % = Kp/tau_p
%   >>> STARTING POINT ONLY -- tune on hardware (raise Kp for speed, watch overshoot).

%% ---------- safety ----------
P_MAX = 38;                      % [PSI] hard clamp (schedule-40 pipe rated 40; design ~20)

%% ---------- pressure targets per ball type (from the inversion table) ----------
DEMO_DISTANCE = 1.5;             % [m] competition distance for this run
P_target_wood   = lookup_ptarget('wooden', DEMO_DISTANCE, 20);   % default 20 PSI
P_target_bouncy = lookup_ptarget('bouncy', DEMO_DISTANCE, 12);   % default 12 PSI
P_target_wood   = min(P_target_wood,   P_MAX);
P_target_bouncy = min(P_target_bouncy, P_MAX);

%% ---------- report ----------
fprintf('\n--- params loaded ---\n');
fprintf('Ts=%.3f  U_MAX=%.0fV  V_TO_DUTY=%.1f\n', Ts, U_MAX, V_TO_DUTY);
fprintf('plant: kv=%.3f  Vdb=%.2f  c=%.3f (tau=%.1fs)\n', kv, Vdb, c, 1/c);
fprintf('PI (start): Kp=%.3f  Ki=%.3f  (lambda=%.2fs) -- TUNE ON HW\n', Kp, Ki, lambda);
fprintf('P_target: wood=%.1f  bouncy=%.1f PSI (clamp %d)  at %.2f m\n', ...
        P_target_wood, P_target_bouncy, P_MAX, DEMO_DISTANCE);

%% ---------- helper: fetch P_target from InversionTables in base ----------
function P = lookup_ptarget(ball, dist, defaultPSI)
    P = defaultPSI;
    try
        IT = evalin('base','InversionTables');
        if isfield(IT, ball)
            T = IT.(ball);
            P = interp1(T.dist_m, T.P_gauge_PSI, dist, 'linear', 'extrap');
        else
            warning('params:noBall','No %s in InversionTables -- default %g PSI.', ball, defaultPSI);
        end
    catch
        warning('params:noTable','InversionTables not found -- default %g PSI for %s. Run build_inversion_table.', defaultPSI, ball);
    end
end
