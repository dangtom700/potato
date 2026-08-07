%SYSID_LEAK  Leak-down characterisation (separates cooling from true leak).
%
%  Procedure: pump to a pressure, STOP the motor, keep the valve closed, log
%  P(t) as it falls. Repeat from several start pressures. The fast early drop is
%  adiabatic COOLING of freshly compressed gas (a transient, not a leak); the
%  slow tail is the TRUE leak. This script fits both and reports the leak model.
%
%  Data: CSV 'leakdown.csv' with columns  t[s], P[PSI], run[int]
%        (run = index so multiple decays can share a file). Falls back to demo.
%
%  Leak model fitted on the slow tail:  dP/dt = -c * P    (first-order),
%  reported as tau_leak = 1/c. (If your data looks like -c*sqrt(P) you have an
%  orifice leak -- see note at bottom.)

clear; clc;
DATAFILE='leakdown.csv'; T_THERMAL=1.0;   % [s] skip first second (cooling)
if isfile(DATAFILE)
    D=readmatrix(DATAFILE); t=D(:,1); P=D(:,2); run=D(:,3);
else
    warning('No %s -- generating synthetic demo data.', DATAFILE);
    [t,P,run]=demo_leak();
end

figure; hold on; grid on; runs=unique(run); cmap=lines(numel(runs));
cAll=[];
for r=1:numel(runs)
    idx=run==runs(r); tk=t(idx)-t(find(idx,1)); Pk=P(idx);
    plot(tk,Pk,'.','Color',cmap(r,:),'DisplayName',sprintf('run %d',runs(r)));
    tail = tk>T_THERMAL;                       % slow leak region
    % fit ln(P) linear in t  ->  P = P0*exp(-c t),  c = leak rate
    good = tail & Pk>0.05*max(Pk);
    if nnz(good)>5
        cc=polyfit(tk(good), log(Pk(good)), 1); % slope = -c
        c=-cc(1); cAll(end+1,1)=c; %#ok<SAGROW>
        plot(tk(good), exp(polyval(cc,tk(good))),'k-','HandleVisibility','off');
    end
end
xline(T_THERMAL,'r--','end of cooling');
xlabel('time after pump stop [s]'); ylabel('P [PSI]');
title('Leak-down: fast cooling + slow true-leak tail'); legend('Location','northeast');

c_leak = median(cAll);
fprintf('\n=== Leak model (slow tail) ===\n');
fprintf('dP/dt = -c*P,  c = %.4f 1/s,  tau_leak = %.1f s\n', c_leak, 1/c_leak);
fprintf('Interpretation: pressure holds ~ e-fold every %.1f s -> fire-on-the-fly, do not hold.\n', 1/c_leak);
assignin('base','leakModel',struct('c',c_leak,'tau',1/c_leak));

% ================= helpers =================
function [t,P,run]=demo_leak()
    dt=0.05; c=0.02; tcool=0.4; t=[];P=[];run=[];
    for r=1:3
        P0=[10 15 20]; P0=P0(r); n=round(30/dt); tk=(0:n-1)'*dt;
        cool = 0.9 + 0.1*exp(-tk/tcool);        % fast ~10% thermal droop
        Pk = P0.*cool.*exp(-c*tk) + 0.1*randn(n,1);
        t=[t;tk]; P=[P;Pk]; run=[run;r*ones(n,1)]; %#ok<AGROW>
    end
end
% NOTE: if a semilog plot of the tail is CURVED (not straight), the leak is an
% orifice: fit dP/dt = -k*sqrt(P) instead (choked flow), i.e. sqrt(P) linear in t.
