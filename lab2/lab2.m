clear all; clc

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])
   
%cd CompileFolder % move to CompileFolder on start (optional)

N = 500; % Pulses Per Revolution (PPR)
Q = 4;   % Quadrature multiplier
G = 6.3; % Gear ratio

% Calculate total counts per 360-degree revolution of the output shaft
total_counts_per_rev = N * Q * G; 

% Factor to convert a single count into degrees
factor = 360 / total_counts_per_rev; 

% Initialize Arduino and Encoder
a = arduino();
configurePin(a, 'D3');
configurePin(a, 'D2', 'Interrupt');
configurePin(a, 'D3', 'Interrupt');

encoder = rotaryEncoder(a, 'D2', 'D3', N);
resetCount(encoder);

% Use a finite loop or a flag so you can safely stop the script
keep_running = true; 
fprintf('Reading encoder position. Press Ctrl+C in the Command Window to stop.\n\n');

while keep_running
    % Read the raw accumulated count
    count = readCount(encoder);
    
    % Convert raw count to absolute accumulated degrees
    total_degrees = count * factor;
    
    % Wrap the position strictly between 0 and 360
    position = mod(total_degrees, 360);

    % Print the results
    fprintf("Position (deg/rad): %.4f // %.4f\n", position, deg2rad(position));
    
    % Pause to prevent flooding the command line
    pause(0.1); 
end

