function out = projectile_sim(v0, p)
%PROJECTILE_SIM  45-degree launch with quadratic aerodynamic drag.
%   out = projectile_sim(v0, p) simulates a ball launched from height
%   p.h_launch at fixed angle p.angle (default 45 deg) with muzzle speed v0,
%   and returns where it is when it reaches the target's horizontal distance.
%
%   Params struct p (SI units):
%     p.h_launch   launch height              [m]
%     p.h_target   target height              [m]
%     p.distance   horizontal dist to target  [m]
%     p.mass       ball mass                   [kg]
%     p.diameter   ball diameter              [m]
%     p.Cd         drag coefficient (~0.47 sphere)
%     p.angle      launch angle (default 45)  [deg]
%     p.rho        air density (default 1.204)[kg/m^3]
%     p.g          gravity (default 9.81)     [m/s^2]
%
%   out fields:
%     out.y_at_target  height when x = distance [m]  (NaN if it falls short)
%     out.miss         y_at_target - h_target   [m]  (0 = perfect hit)
%     out.traj         [t x y vx vy] trajectory rows
%     out.apex         max height reached       [m]
%
%   Example:
%     p = struct('h_launch',0.5,'h_target',0.5,'distance',1.5, ...
%                'mass',0.008,'diameter',0.040,'Cd',0.47);
%     projectile_sim(4.0, p)

    if ~isfield(p,'angle'), p.angle = 45; end
    if ~isfield(p,'rho'),   p.rho   = 1.204; end
    if ~isfield(p,'g'),     p.g     = 9.81; end

    A = pi*(p.diameter/2)^2;            % frontal area
    k = 0.5*p.rho*p.Cd*A/p.mass;        % drag / mass  (a = -k|v|v)
    th = deg2rad(p.angle);
    s0 = [0; p.h_launch; v0*cos(th); v0*sin(th)];   % [x y vx vy]

    rhs = @(t,s) [ s(3); s(4); ...
                  -k*hypot(s(3),s(4))*s(3); ...
                  -p.g - k*hypot(s(3),s(4))*s(4) ];

    % stop when x reaches the target distance (rising crossing)
    optEv = odeset('Events', @(t,s) reach_x(t,s,p.distance), ...
                   'RelTol',1e-7,'AbsTol',1e-9,'MaxStep',2e-3);
    [t,y,te,ye] = ode45(rhs, [0 5], s0, optEv);

    out.traj = [t y];
    out.apex = max(y(:,2));
    if isempty(te)
        out.y_at_target = NaN;          % never travelled far enough
    else
        out.y_at_target = ye(1,2);
    end
    out.miss = out.y_at_target - p.h_target;
end

function [val,isterminal,direction] = reach_x(~,s,d)
    val = s(1) - d; isterminal = 1; direction = 1;
end
