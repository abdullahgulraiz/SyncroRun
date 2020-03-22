%get data from text files
pos_tracking=textscan(fopen('pointInfo2.txt'),'%f,%f,%f');
pos_detection=textscan(fopen('pointInfo.txt'),'%f,%f,%f');

%read data into vectors
pos_d_x = pos_detection{1}; pos_d_y = pos_detection{2}; pos_d_t = pos_detection{3};
pos_t_x = pos_tracking{1}; pos_t_y = pos_tracking{2}; pos_t_t = pos_tracking{3};

%remove outliers for face detection
previous_y = pos_d_y(1);
previous_x = pos_d_x(1);
for i = 1:size(pos_d_y,1)
    if (pos_d_y(i) == 0.361925)
        pos_d_y(i) = previous_y;
    end
    if (pos_d_x(i) == 0.480535)
        pos_d_x(i) = previous_x;
    end
    previous_y = pos_d_y(i);
    previous_x = pos_d_x(i);
end

%plot centerYm vs time for both tracking and detection
plot(pos_t_t(1:100), pos_t_y(1:100), 'r');
hold on;
plot(pos_d_t(1:100), pos_d_y(1:100), 'b');
xlabel('Time (s)');
ylabel('CenterY (m)');
legend('Tracking', 'Detection');

%plot centerXm for both tracking and detection
figure;
plot(pos_t_t(1:100), pos_t_x(1:100), 'r');
hold on;
plot(pos_d_t(1:100), pos_d_x(1:100), 'b');
xlabel('Time (s)');
ylabel('CenterX (m)');
legend('Tracking', 'Detection');

%plot error between tracking and detection for centerXm
figure;
errorX = pos_t_x(1:100) - pos_d_x(1:100);
errorY = pos_t_y(1:100) - pos_d_y(1:100);
plot(pos_t_t(1:100), errorX);
hold on;
plot(pos_t_t(1:100), errorY);
xlabel('Time (s)');
ylabel('Error (m)');
legend('Error in CenterX', 'Error in CenterY');
title('Error = Tracking - Detection');
