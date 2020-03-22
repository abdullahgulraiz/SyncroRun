clear;

fps = 60;

%get data from text files
bb = textscan(fopen('BoundingBoxes.csv'),'%f,%f,%f,%f,%f');
center = textscan(fopen('CenterLocationError.csv'),'%f,%f');

%read data into vectors
bb_t = bb{1}; bb_x = bb{2}; bb_y = bb{3}; bb_w = bb{4}; bb_h = bb{5};
center_t = center{1}; center_error = center{2};

bb_initial = [bb_x(1) bb_y(1) bb_w(1) bb_h(1)];
bb_tracking = [bb_x bb_y bb_w bb_h];

overlapRatios = bboxOverlapRatio(bb_initial,bb_tracking);
frames = bb_t * fps;

plot(frames,overlapRatios);
title('Overlap Ratios vs Frames');
figure;
plot(frames,center_error);
title('Center Location Error vs Frames');

