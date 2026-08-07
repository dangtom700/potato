%BUILD_INVERSION_TABLE  Distance -> muzzle speed -> release pressure, at 45 deg.
%
%  Stage 1 (physics): for each target distance and ball type, root-find the
%  muzzle speed v0 that makes the 45-deg drag trajectory pass through the
%  target point (distance, h_target) from (0, h_launch).  Uses projectile_sim.
%
%  Stage 2 (pneumatics, CALIBRATED): convert required ball KE to a tank
%  release pressure via a simple energy model
%        KE_ball = eta * P_gauge * V_swept
%  so    P_gauge = KE_ball / (eta * V_swept).
%  eta (launch efficiency) and V_swept (effective swept volume) are fit from a
%  few real shots -- start with the guesses below and refine with
%  fit_plant_VtoP / your shot data.  Report distances 1..2 m (competition).
%
%  Outputs: a printed table + a plot, and struct arrays T_wood / T_bouncy.

clear; clc;

% ---- fixed geometry (measure yours) ----
geom.h_launch = 0.50;      % [m] barrel exit height
geom.h_target = 0.50;      % [m] target centre height (hoop rim ~ level here)
geom.angle    = 45;        % hard constraint

% ---- ball types: {name, mass[kg], diameter[m], Cd} ----
balls = { 'wooden', 0.030, 0.040, 0.47; ...
          'bouncy', 0.008, 0.040, 0.47 };

% ---- pneumatic calibration params (REFINE with real shots) ----
pneu.eta     = 0.35;       % launch efficiency (pneumatic energy -> ball KE)
pneu.V_swept = 8e-6;       % [m^3] effective swept volume (8 cc placeholder)
PSI_PER_PA   = 1/6894.76;  % Pa -> PSI

dists = 1.0:0.1:2.0;       % target distances [m]

fprintf('\n45-deg inversion table  (h_launch=%.2f, h_target=%.2f)\n', geom.h_launch, geom.h_target);
fprintf('eta=%.2f  V_swept=%.1f cc\n', pneu.eta, pneu.V_swept*1e6);

figure; hold on; grid on; colors = lines(size(balls,1));
Tout = struct();
for b = 1:size(balls,1)
    name = balls{b,1};
    p = geom;
    p.mass = balls{b,2}; p.diameter = balls{b,3}; p.Cd = balls{b,4};
    v0 = zeros(size(dists)); KE = v0; Pg_psi = v0;
    fprintf('\n%s (m=%.0f g, dia=%.0f mm, Cd=%.2f)\n', name, p.mass*1e3, p.diameter*1e3, p.Cd);
    fprintf('  dist[m]  v0[m/s]  KE[mJ]  P_gauge[PSI]\n');
    for i = 1:numel(dists)
        p.distance = dists(i);
        v0(i)  = solve_v0(p);
        KE(i)  = 0.5*p.mass*v0(i)^2;
        Pg_pa  = KE(i)/(pneu.eta*pneu.V_swept);      % gauge pressure [Pa]
        Pg_psi(i) = Pg_pa*PSI_PER_PA;
        fprintf('   %4.1f    %5.2f   %5.1f     %5.1f\n', dists(i), v0(i), KE(i)*1e3, Pg_psi(i));
    end
    Tout.(name) = table(dists(:), v0(:), KE(:)*1e3, Pg_psi(:), ...
                        'VariableNames', {'dist_m','v0_mps','KE_mJ','P_gauge_PSI'});
    plot(dists, Pg_psi, '-o', 'Color', colors(b,:), 'DisplayName', name, 'LineWidth',1.4);
end
xlabel('target distance [m]'); ylabel('release pressure P_{gauge} [PSI]');
title('Inversion table: distance -> release pressure (per ball type)');
legend('Location','northwest'); yline(40,'r--','40 PSI limit');

% expose tables
assignin('base','InversionTables',Tout);
fprintf('\nSaved tables to workspace struct  InversionTables.<ball>\n');
fprintf('NOTE: P values are physics x pneumatic-guess. Fit eta & V_swept to real shots to trust them.\n');

% -------- helper: root-find v0 for one target --------
function v0 = solve_v0(p)
    f = @(v) miss(v,p);
    % bracket: light launch (short) to a strong launch (long)
    v0 = fzero(f, [0.5 20]);
end
function m = miss(v0,p)
    o = projectile_sim(v0,p);
    if isnan(o.y_at_target), m = -10; else, m = o.miss; end  % short -> negative
end
