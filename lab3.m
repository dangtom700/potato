clear all; clc;

a = arduino()

addpath([fileparts(mfilename('fullpath')) '\RASPlib'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\src'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\include'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\examples'],...
		[fileparts(mfilename('fullpath')) '\RASPlib\blocks'])
   
%cd CompileFolder % move to CompileFolder on start (optional)

time = 200
while time > 0
    voltage = readVoltage(a, 'A0')
    writePWMVoltage(a, 'D11', voltage)
    time = time - 0.1;
    pause(0.1);
end