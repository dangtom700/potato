%FIT_PLANT_VTOP  Combine build + leak into a voltage->pressure plant model.
%
%  Model (pump-with-leak, the physical picture):
%        dP/dt = kpump(V) - c*P
%  where kpump(V) = kv*max(V - Vdb, 0)  (pump strength, ~linear in voltage
%  above a deadband) and c is the leak rate from sysid_leak.
%
%  This gives, at a fixed voltage, a first-order approach to steady pressure
%  P_ss(V) = kpump(V)/c with time constant 1/c. That is exactly the local
%  P(s)/V(s) = (kv/(s+c)) transfer function you can drop into Simulink, and it
%  is intentionally simple -- the PI with anti-windup is robust to the gain
%  changing with pressure, so you do not need a perfect model.
%
%  Run sysid_pressure_build.m and sysid_leak.m first (they populate the base
%  workspace with buildData and leakModel); this script fits kpump(V) and
%  reports the model + a Simulink-ready tf.

clear; clc;

% pull upstream results if present, else use demo fits
if evalin('base','exist(''buildData'',''var'')'), bd=evalin('base','buildData'); else, bd=struct('Vg',(4:2:12)','buildGain',0.05*ones(5,1)); end
if evalin('base','exist(''leakModel'',''var'')'), lm=evalin('base','leakModel'); else, lm=struct('c',0.05); end

% kpump(V) at each level = dP/dt|_{P=0} = a from  dP/dt = a - c*P.
% From build data we captured the slope (-b ~ -c); the intercept a = kpump.
% Recompute intercept using the shared leak c for consistency:
%   a_k = mean(dP/dt + c*P) over the run  (done upstream); here use buildGain as
%   a proxy if intercepts were not stored. Prefer refitting with real data.
V = bd.Vg(:);
% If build script stored intercepts use them; otherwise approximate:
if isfield(bd,'intercept'), kp = bd.intercept(:); else, kp = bd.buildGain(:); end

% fit kpump = kv*(V - Vdb)
drv = kp>0.02*max(kp);
c1 = polyfit(V(drv), kp(drv), 1);
kv = c1(1); Vdb = -c1(2)/c1(1); c = lm.c;

fprintf('\n=== Voltage -> Pressure plant ===\n');
fprintf('dP/dt = kv*max(V-Vdb,0) - c*P\n');
fprintf('  kv  = %.4f PSI/s per V\n', kv);
fprintf('  Vdb = %.2f V\n', Vdb);
fprintf('  c   = %.4f 1/s  (leak, tau=%.1f s)\n', c, 1/c);
fprintf('Local linear model about an operating voltage:  P(s)/V(s) = kv/(s + c)\n');
fprintf('Steady pressure at V:  P_ss = kv*(V-Vdb)/c\n\n');

% Simulink-ready first-order tf (small-signal, above deadband)
if exist('tf','file')
    Gp = tf(kv,[1 c]); disp('Local plant tf  P(s)/V(s):'); Gp %#ok<NOPRT>
end

% quick validation plot: predicted P_ss(V) vs measured build intercepts
figure; plot(V, kp,'o'); hold on; grid on;
Vf=linspace(min(V),max(V),50); plot(Vf, max(kv*(Vf-Vdb),0),'-');
xlabel('V [V]'); ylabel('kpump = dP/dt|_{P=0} [PSI/s]');
title(sprintf('Pump gain fit: kv=%.3f, Vdb=%.2f V',kv,Vdb));

assignin('base','plantModel',struct('kv',kv,'Vdb',Vdb,'c',c));
fprintf('Saved plantModel to base workspace. Suggest cross-validating against a held-out run (see ANALYSIS_PLAN).\n');
