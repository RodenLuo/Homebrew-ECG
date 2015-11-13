function Hd = Notching_50Hz
%50HZ_NOTCHING Returns a discrete-time filter object.

% MATLAB Code
% Generated by MATLAB(R) 8.1 and the Signal Processing Toolbox 6.19.
% Generated on: 01-Jul-2015 19:27:02

% Butterworth Bandstop filter designed using FDESIGN.BANDSTOP.

% All frequency values are in Hz.
Fs = 1000;  % Sampling Frequency

N   = 10;  % Order
Fc1 = 48;  % First Cutoff Frequency
Fc2 = 52;  % Second Cutoff Frequency

% Construct an FDESIGN object and call its BUTTER method.
h  = fdesign.bandstop('N,F3dB1,F3dB2', N, Fc1, Fc2, Fs);
Hd = design(h, 'butter');

% [EOF]
