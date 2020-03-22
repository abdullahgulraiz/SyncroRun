%get data from text files
pos_tracking=textscan(fopen('tracking_position.txt'),'%f,%f,%f');
pos_detection=textscan(fopen('detection_position.txt'),'%f,%f,%f');

%read data into vectors
pos_d_x = pos_detection{1}; pos_d_y = pos_detection{2}; pos_d_t = pos_detection{3};
pos_t_x = pos_tracking{1}; pos_t_y = pos_tracking{2}; pos_t_t = pos_tracking{3};

%remove outliers for face detection
pos_d_x = pos_d_x(pos_d_x ~= -1);
pos_d_y = pos_d_y(pos_d_y ~= -1);

%PLOT FACE DETECTION
%skipping initial values to minimize error (plotting values 10-21)
count = 10; 
while (1) 
    if (count >= 21)
       break; 
    end
    plot(pos_d_t(10:count), pos_d_x(10:count), 'b-o');
    title('Face Detection');
    xlabel('Time (s)');
    ylabel('CenterX (m)');
    legend('Trajectory 2');
    count = count + 1;
    pause(1);
end
hold on;
%plotting next trajectory
while (1) 
    if (count >= 40)
       break; 
    end
    plot(pos_d_t(21:count), pos_d_x(21:count), 'g-o');
    legend('Trajectory 2', 'Trajectory 3');
    count = count + 1;
    pause(1);
end

%PLOT FACE TRACKING
%skipping initial values to minimize error (plotting values 25-48)
figure;
count = 25;
while (1) 
    if (count >= 48)
       break; 
    end
    plot(pos_t_t(25:count), pos_t_x(25:count), 'r-o');
    title('Face Tracking');
    xlabel('Time (s)');
    ylabel('CenterX (m)');
    legend('Trajectory 2');
    count = count + 1;
    pause(1);
end
hold on;
%plotting next trajectory
count = 48;
while (1) 
    if (count >= 73)
       break; 
    end
    plot(pos_t_t(48:count), pos_t_x(48:count), 'c-o');
    legend('Trajectory 2', 'Trajectory 3');
    count = count + 1;
    pause(1);
end