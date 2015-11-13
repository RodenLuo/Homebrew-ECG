clear;clc;
delete(instrfindall);
s= serial('COM7');
s.DataBits=8;
s.BaudRate=9600;
s.Parity='none';
s.StopBits=1;
s.InputBufferSize=255;
fopen(s);

%load('good_check_pattern.mat');%good_check_pattern， 120 个数
check_data_length = 250;
% time = 2;
% window = 50;
windowSize = 20;
threshold = 120;
beat_start = 0;
beat_end = 0;
beat_no = 0;
heart_max = [];

Nlen =5000;
y = [];
old_y = [];
figure(1);
set(gcf,'Position',[0 0 1800 800]);
heart_rate = 'calculating';

for i = 1:1:2000
    %取100个点之后再画图
    for j = 1:1:100
        temp = fread(s,1);
        y = [y temp];
    end
    
    %画原始图
    subplot(211);
    if (length(old_y) == Nlen)
        %后部分保留上个周期的图，前部分画到最新位置，中间间隔500个点
        plot(( length(y) + 500 ):1:5000,old_y(( length(y) + 500 ):5000),'b',1:1:length(y),y,'b');
    else
        %还没到一个完整周期，把所有的点画完
        plot(1:1:length(y),y);
    end
    axis([0 Nlen 0 300]);
    xlabel('Time(ms)');
    ylabel('ECG magnitude');
    title('Real time ECG waveform');
    
    %画滤波后的图
    subplot(212);
    notched_y = filter(ones(1,windowSize)/windowSize,1,y);%滑动平均
    notched_y = filter(Notching_50Hz, notched_y);
    if (length(old_y) == 5000)
        %对 old_y 和 y 做一次滤波后
        %再后部分保留上个周期的图，前部分画到最新位置，中间间隔500个点
        notched_old_y = filter(ones(1,windowSize)/windowSize,1,old_y);%滑动平均
        notched_old_y = filter(Notching_50Hz,notched_old_y);
        plot(( length(y) + 500 ):1:5000,notched_old_y(( length(y) + 500 ):5000),'b',1:1:length(y),notched_y,'b');
        
        %heart rate的计算并转化成char
        heart_to_now = [notched_old_y notched_y(1: floor(length(notched_y)/500)*500 )];
        
        k = 1;
        while ( k <= length(heart_to_now)-check_data_length + 1 )
            heart_max = [ heart_max  max(heart_to_now(k:k+check_data_length-1)) ];
            k = k+check_data_length;
        end
        heart_threshold = heart_max > 130;
        beat_no = sum(heart_threshold);
        heart_rate = 60*500*( beat_no - 1 )/( length(heart_to_now) ); 
        if (heart_rate > 160 || heart_rate < 40 )
            heart_rate = 'calculating';
        else
            %转化成char
            heart_rate = char(vpa(heart_rate,2));
        end
        %计数器和累加向量复位
        beat_no = 0;
        heart_max = [];
    else
        plot(1:1:length(y),notched_y);
    end
    axis([0 Nlen 0 300]);
    xlabel('Time(ms)');
    ylabel('ECG magnitude');
    title(strcat('Real time ECG waveform after filtering  Heart Rate:', heart_rate));
    drawnow;
    if (length(y) == 5000)
        old_y = y;
        y = [];
    end
end

fclose(s);
delete(s);
clear s;