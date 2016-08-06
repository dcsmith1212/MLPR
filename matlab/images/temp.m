input = VideoReader('inputdoub.mp4');
output = VideoWriter('doublevid.avi','Uncompressed AVI');

frame = zeros(2160,3840,3);
outframe = zeros(3840,2160,3);

open(output)
while hasFrame(input)
    frame = readFrame(input);
    outframe = imrotate(frame, -90);

    writeVideo(output,outframe)
end
close(output)