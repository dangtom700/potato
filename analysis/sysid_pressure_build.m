%SYSID_PRESSURE_BUILD  Pressure build-rate under several drive cases.
%
%  More than a single bump test: run the pump at several constant voltages and
%  (optionally) from several starting pressures, and characterise how the build
%  rate dP/dt depends on BOTH voltage and current pressure.  This exposes the
%  nonlinearity (dP/dt falls as P rises: gas gets stiffer + leak grows + motor
%  loads down) that a single step would miss.
%
%  Data: CSV 'pressure_build.csv' with columns  t[s], V[V], P[PSI]
%        (P = analog pressure sensor; run in the compression region only).
%  Multiple runs may be concatenated; the script segments by voltage level.
%  Falls back to synthetic demo data if the file is absent.
%
%  Outputs: dP/dt vs P curves per voltage, and the build-gain vs voltage.

clear; clc;
DATAFILE = 'pressure_build.csv';
if isfile(DATAFILE)
    D=readmatrix(DATAFILE); t=D(:,1); V=D(:,2); P=D(:,3);
else
    warning('No %s -- generating synthetic demo data.', DATAFILE);
    [t,V,P]=demo_build();
end

Vlev = unique(round(V,1)); Vlev = Vlev(Vlev>0.5);
figure; hold on; grid on; cmap=lines(numel(Vlev));
buildGain=[]; Vg=[];
for k=1:numel(Vlev)
    idx = abs(V-Vlev(k))<0.2;
    tk=t(idx); Pk=P(idx);
    if numel(Pk)<10, continue; end
    dPdt = filt_deriv(tk,Pk);                    % smoothed dP/dt
    % keep the rising, in-range portion
    good = dPdt>0 & Pk<0.95*max(P);
    plot(Pk(good), dPdt(good), '.', 'Color', cmap(k,:), ...
         'DisplayName', sprintf('%.0f V',Vlev(k)));
    % build gain = dP/dt extrapolated to P->0 (pump strength at that voltage)
    if nnz(good)>5
        cc=polyfit(Pk(good),dPdt(good),1);       % dP/dt ~ a - b*P
        buildGain(end+1,1)=cc(2); Vg(end+1,1)=Vlev(k); %#ok<SAGROW>
    end
end
xlabel('tank pressure P [PSI]'); ylabel('dP/dt [PSI/s]');
title('Build rate vs pressure, per drive voltage'); legend('Location','northeast');

figure; plot(Vg,buildGain,'o-'); grid on;
xlabel('V [V]'); ylabel('build gain dP/dt|_{P=0} [PSI/s]');
title('Pump strength vs voltage');

assignin('base','buildData',struct('Vlev',Vlev,'Vg',Vg,'buildGain',buildGain));
fprintf('Segmented %d voltage levels. Use fit_plant_VtoP.m to combine with leak.\n',numel(Vlev));

% ================= helpers =================
function d = filt_deriv(t,x)
    % centered difference + light moving-average smoothing
    d=gradient(x(:),t(:));
    w=5; k=ones(w,1)/w; d=conv(d,k,'same');
end
function [t,V,P]=demo_build()
    % dP/dt = kpump(V) - leak(P);  kpump=0.9*(V-2)+, leak=0.05*P
    dt=0.02; levels=[4 6 8 10 12]; t=[];V=[];P=[]; tnow=0;
    for L=levels
        Pp=0; n=round(6/dt); tk=(0:n-1)'*dt; Pk=zeros(n,1);
        kp=max(0.9*(L-2),0);
        for i=1:n
            dP=kp-0.05*Pp; Pp=max(Pp+dP*dt,0); Pk(i)=Pp+0.15*randn;
        end
        t=[t;tnow+tk]; V=[V;L*ones(n,1)]; P=[P;Pk]; tnow=tnow+n*dt; %#ok<AGROW>
    end
end
