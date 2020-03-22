clear all;

%get data from text files
pos_tracking=textscan(fopen('o.csv'),'%f,%f,%f');

%read data into vectors
pos_t_x = pos_tracking{2}; pos_t_y = pos_tracking{3}; pos_t_t=pos_tracking{1};

%code to find time between two consecutive maximas
posMax1 = 0; posMax2 = 0; timeMax1 = 0; timeMax2 = 0; centerXm = pos_t_x(2); previous_centerXm = 0;
actionCounter = 1; timeTotal = 0; timeCounter = 0;
for i = 2:size(pos_t_x,1)
    centerXm = pos_t_x(i);
    previous_centerXm = pos_t_x(i-1);
    if (actionCounter == 1)
        if (centerXm < previous_centerXm)
           posMax1 = previous_centerXm;
           timeMax1 = pos_t_t(i-1);
           actionCounter = actionCounter + 1;
        end
    elseif (actionCounter == 2)
        if (centerXm > previous_centerXm)
           actionCounter = actionCounter + 1; 
        end
    elseif (actionCounter == 3)
        if (centerXm < previous_centerXm)
           posMax2 = previous_centerXm;
           timeMax2 = pos_t_t(i-1);
           timeTotal = timeTotal + (timeMax2 - timeMax1);
           timeCounter = timeCounter + 1;
           actionCounter = 1;
        end
    elseif (actionCounter == 4)
        break;
    end
end

meanTime = timeTotal / timeCounter;
frequency = 1/meanTime;