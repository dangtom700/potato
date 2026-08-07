%SYSID_MOTOR_SPEED  Characterise the motor+gearbox: voltage -> output speed.
%
%  Data: CSV 'motor_steps.csv' with columns  t[s], V[V], omega[rad/s]
%        (omega = output-shaft speed from the encoder: count*2*pi/(Q*N*G) then
%         differentiated; see systemID notes. Q*N*G = 12600).
%  If the file is absent, synthetic demo data is generated so the script runs.
%
%  Outputs: steady-state map omega_ss = Kv*(V - Vdb) with stiction deadband Vdb,
%  and a first-order time constant tau from the first step. Prints params, plots.

clear; clc;
DATAFILE = 'motor_steps.csv';
G = 6.3;                            % gearbox ratio (for reference/labels)

if isfile(DATAFILE)
    D = readmatrix(DATAFILE); t=D(:,1); V=D(:,2); w=D(:,3);
else
    warning('No %s -- generating synthetic demo data.', DATAFILE);
    [t,V,w] = demo_motor();
end

% --- segment into constant-voltage levels ---
Vlev = unique(round(V,2)); Vlev = Vlev(Vlev> -50);
Vss=[]; Wss=[];
for k=1:numel(Vlev)
    idx = abs(V-Vlev(k))<1e-2;
    tk=t(idx); wk=w(idx);
    if numel(wk)<5, continue; end
    tail = tk > (tk(1)+0.7*(tk(end)-tk(1)));   % last 30% = steady state
    Vss(end+1,1)=Vlev(k); Wss(end+1,1)=mean(wk(tail)); %#ok<SAGROW>
end

% --- fit steady-state affine on the DRIVEN region (omega>0) ---
drv = Wss > 0.05*max(Wss);
c = polyfit(Vss(drv), Wss(drv), 1);   % omega = c1*V + c2
Kv  = c(1);                            % [rad/s per V]  (output shaft)
Vdb = -c(2)/c(1);                      % stiction / deadband voltage [V]

% --- first-order tau from the first non-zero step ---
tau = fit_tau(t,V,w);

fprintf('\n=== Motor + gearbox (output shaft) ===\n');
fprintf('Kv  (slope)     = %.3f rad/s per V\n', Kv);
fprintf('Vdb (deadband)  = %.2f V  (stiction floor)\n', Vdb);
fprintf('tau (1st order) = %.3f s\n', tau);
fprintf('Motor-shaft speed = %.1f x output (gear %.1f)\n', G, G);

% --- plots ---
figure;
subplot(1,2,1); plot(Vss,Wss,'o'); hold on; grid on;
Vfit=linspace(min(Vss),max(Vss),50); plot(Vfit, max(Kv*(Vfit-Vdb),0),'-');
xlabel('V [V]'); ylabel('\omega_{ss} [rad/s]');
title(sprintf('SS map: Kv=%.2f, Vdb=%.2f V',Kv,Vdb));
subplot(1,2,2); plot(t,w); grid on; xlabel('t [s]'); ylabel('\omega [rad/s]');
title(sprintf('Step response (\\tau \\approx %.2f s)',tau));

assignin('base','motorModel',struct('Kv',Kv,'Vdb',Vdb,'tau',tau));

% ================= helpers =================
function tau = fit_tau(t,V,w)
    dV=[0;diff(V)]; ki=find(dV>0.5,1);          % first up-step
    if isempty(ki), tau=NaN; return; end
    seg = t>=t(ki) & t<t(ki)+1.5;               % 1.5 s window
    ts=t(seg)-t(ki); ws=w(seg);
    wf=mean(ws(end-max(1,round(0.1*numel(ws))):end));
    if wf<=0, tau=NaN; return; end
    % omega(t)=wf*(1-exp(-t/tau)); fit tau by 1-param search
    obj=@(tt) sum((ws-wf*(1-exp(-ts/tt))).^2);
    tau=fminbnd(obj,1e-3,2);
end
function [t,V,w]=demo_motor()
    Kv=2.0; Vdb=1.5; tau=0.12; dt=0.005; Tstep=1.2;
    levels=[0 2 4 6 8 10 12]; t=[];V=[];w=[]; wprev=0; tnow=0;
    for L=levels
        n=round(Tstep/dt); tk=(0:n-1)'*dt;
        wss=max(Kv*(L-Vdb),0);
        wk=wss+(wprev-wss).*exp(-tk/tau);
        w=[w; wk+0.05*randn(n,1)]; V=[V; L*ones(n,1)]; t=[t; tnow+tk]; %#ok<AGROW>
        wprev=wss; tnow=tnow+n*dt;
    end
end
