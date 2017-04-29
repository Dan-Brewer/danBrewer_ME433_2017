function [data] = Hw10Client()
%   modified from motor control project in 333: provided a menu for accessing PIC32 motor control functions
%
%   Hw10Client(port)
%
%   Input Arguments:
%       port - the name of the com port.  This should be the same as what
%               you use in screen or putty in quotes ' '
%
%   Example:
%       client('/dev/ttyUSB0') (Linux/Mac)
%       client('COM3') (PC)
%
%   For convenience, you may want to change this so that the port is hardcoded.
   
% Opening COM connection
port = 'COM10'; %Hardcoded for convienience

if ~isempty(instrfind)
    fclose(instrfind);
    delete(instrfind);
end

fprintf('Opening port %s....\n',port);

% settings for opening the serial port. baud rate 230400, hardware flow
% control (baudrate may need some modification)
% wait up to 120 seconds for data before timing out 
mySerial = serial(port, 'BaudRate', 9600); %10 second timeout for debugging
% opens serial connection
fopen(mySerial);
% closes serial port when function exits
clean = onCleanup(@()fclose(mySerial));                                 

has_quit = false;
% menu loop
while ~has_quit
    fprintf('PIC32 DSP INTERFACE\n\n');
    % display the menu options; this list will grow
    fprintf(' a:get current in ADC counts  b:get current in mA c: read encoder count     d: Read encoder in 1/10 degree   e: reset encoder count  f:set PWM  g: set current gains   h: get current gains p:unpower motor  i:set position gains  j:get position gains  k:test current gains  l:test position gains  m:set step trajectory  n:set cubic trajectory   o:track trajectory   r:get mode     q: Quit\n');
    % read the user's choice (should be r)
    selection = input('\nENTER COMMAND: ', 's');
     
    % send the command to the PIC32
    fprintf(mySerial,'%c\n',selection);
    
    % take the appropriate action
    switch selection
        case 'r' %read filtered data
            data = zeros(100,4);
            for ii=1:100
                data(ii, :) = fscanf(mySerial, '%d %d %d %d');
            end
            
            %plotting
            x = 1:100;
            plot(x, data(:,1), x, data(:,2), x, data(:,3), x, data(:,4));
            legend('Raw', 'IIR', 'MAF', 'FIR');
            
            figure();
            Fs = 100; %100 hz sampling rate
            T = 1/Fs; %sampling period
            L = 100; %length of signal
            Y1 = fft(data(:,1)); %fft of raw data
            Y2 = fft(data(:,2)); %fft of IIR 
            Y3 = fft(data(:,3)); %fft of MAF
            Y4 = fft(data(:,4)); %fft of MAF
            P1_2 = abs(Y1/L); %math from matlab example, not sure whats happening here
            P1_1 = P1_2(1:L/2+1);
            P1_1(2:end - 1) = 2*P1_1(2:end - 1);
            P2_2 = abs(Y2/L); %math from matlab example, not sure whats happening here
            P2_1 = P2_2(1:L/2+1);
            P2_1(2:end - 1) = 2*P2_1(2:end - 1);
            P3_2 = abs(Y3/L); %math from matlab example, not sure whats happening here
            P3_1 = P3_2(1:L/2+1);
            P3_1(2:end - 1) = 2*P3_1(2:end - 1);
            P4_2 = abs(Y4/L); %math from matlab example, not sure whats happening here
            P4_1 = P4_2(1:L/2+1);
            P4_1(2:end - 1) = 2*P4_1(2:end - 1);
            f = Fs*(0:(L/2))/L;
            hold on
            plot(f, P1_1);
            plot(f, P2_1);
            plot(f, P3_1);
            plot(f, P4_1);
            legend('Raw', 'IIR', 'MAF', 'FIR');
            xlabel('f (Hz)');
            
            has_quit = true; %exit client
        case 'q'
            has_quit = true;             % exit client
        otherwise
            fprintf('Invalid Selection %c\n', selection);
    end
end

end

