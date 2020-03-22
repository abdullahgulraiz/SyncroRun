clear all;

%get data from text files
pos = textscan(fopen('PositionLatest.csv'),'%f,%f,%f');

%read data into vectors
pos_x = pos{2}; pos_t = pos{1}; pos_y = pos{3};
freq = zeros(size(pos_t)); checkArr = zeros(size(pos_t));
time1 = 0; time2 = 0; maxX = 0; minX = false; timePeriod = -1;
for i = 1:size(pos_t,1)
    point = pos_x(i);
    if (point > maxX && time1 == 0 && time2 == 0 && minX == false)
        maxX = point;
    elseif (point < maxX && time1 == 0 && time2 == 0 && minX == false)
        time1 = pos_t(i-1);
    elseif (point < maxX && time2 == 0 && minX == false)
        maxX = point;
    elseif (point > maxX && time2 == 0 && minX == false)
        minX = true; 
    elseif (point > maxX && time2 == 0 && minX == true)
        maxX = point;
    elseif (point < maxX && time2 == 0 && minX == true) 
        time2 = pos_t(i-1); 
        timePeriod = time2 - time1;
        time1 = time2;
        time2 = 0;
        minX = false;
    end
    freq(i) = 1/timePeriod;
end

subplot(2,1,1);
plot(pos_t, pos_x);
title('PosX vs Time');
subplot(2,1,2);
plot(pos_t, freq);
ylim([1.75 2.5])
title('Freq vs Time');