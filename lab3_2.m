clear all; clc

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])

%cd CompileFolder % move to CompileFolder on start (optional)

% Do not delete the above lines, they are used to run this script

% ==========================================================================
%  lab3_2 -- ENCODER as a "potentiometer" dimming an LED on D11
% --------------------------------------------------------------------------
%  Same idea as lab3.m (turn a knob -> dim an LED) but the shaft ANGLE from
%  the quadrature encoder stands in for the pot: one shaft revolution
%  (0..360 deg) is rescaled to a 0..255 PWM level and written to the LED on
%  D11 -- the same mapping as the firmware's angleToPwm().
%
%  NOTE: creating the arduino() object below re-flashes MATLAB's server
%  firmware onto the board, overwriting the custom MSE312 firmware. That's
%  fine here -- MATLAB does the dimming. Re-upload the .hex to get it back.
% ==========================================================================

% Connect. The rotaryEncoder add-on library must be enabled to read the
% encoder, so it is requested explicitly here (bare arduino() won't have it).
% Change 'COM5' if the board enumerates on a different port.
a = arduino('COM5', 'Mega2560', 'Libraries', 'rotaryEncoder')

% Encoder: 500-line quadrature on D2/D3, x4-decoded -> 2000 counts/motor-rev,
% mounted on the MOTOR shaft before a 6.3:1 gearbox. So one OUTPUT-shaft
% revolution is 4*500*6.3 = 12600 counts (matches the firmware).
%   To dim over one MOTOR-shaft turn instead (full sweep per hand-turn),
%   set COUNTS_PER_REV = 4*500 = 2000.
COUNTS_PER_REV = 4 * 500 * 6.3;          % = 12600 (output shaft)

encoder = rotaryEncoder(a, 'D2', 'D3', 500);
resetCount(encoder);                     % call the current position "0 deg"

writePWMDutyCycle(a, 'D11', 0);          % start the LED dark

% Keep this loop lean: the rotaryEncoder object streams count data in the
% background, so anything slow in here (e.g. disp() every pass) lets that stream
% outrun the reads and overflow MATLAB's buffer. Print at most occasionally.
printEvery = 10;                         % show the angle every 10th pass (~1 Hz)
k = 0;

time = 200;
while time > 0
    count = readCount(encoder);                       % accumulated x4 counts
    angle = mod(count / COUNTS_PER_REV * 360, 360);   % wrap to [0, 360) deg
    pwm   = round(angle / 360 * 255);                 % [0, 255], like analogWrite
    writePWMDutyCycle(a, 'D11', pwm / 255);           % dim the LED (duty = pwm/255)

    k = k + 1;
    if mod(k, printEvery) == 0, fprintf('angle = %6.1f deg\n', angle); end

    time = time - 0.05;
    pause(0.05);                                      % ~20 Hz; drains the buffer faster
end

writePWMDutyCycle(a, 'D11', 0);          % leave the LED off on exit
