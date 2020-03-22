clear;
myVideo = VideoReader('video.mp4');
n = myVideo.NumberOfFrames;
x = nan(n,1);
y = nan(n,1);
for i = 1:n
  img = read(myVideo,i);
  imshow(img);
  [x(i),y(i)] = ginput(1);
end
disp([x,y]);