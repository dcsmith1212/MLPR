clear all
close all

IDEAL_WIDTH = 516;
MIN_BLOB_AREA = round( IDEAL_WIDTH / 13 );
MAX_BLOB_AREA = round( IDEAL_WIDTH / 3.5) ;
MIN_BB_ASPECT = 0.4;
MAX_BB_ASPECT = 0.55;

video = VideoReader('vid2.mp4');
while hasFrame(video)
    frame = readFrame(video);
    %frame = im2bw(frame, graythresh(frame));
    imshow(frame,[])
    
    [L,M_indiv] = bwlabel(frame);
	stats = regionprops(L,'Area','Centroid','BoundingBox');    
    
    for j = 1:M_indiv
        % Neglects blobs that are obviously too small
        if stats(j).Area > MIN_BLOB_AREA && stats(j).Area < MAX_BLOB_AREA ...
                && stats(j).BoundingBox(4) / stats(j).BoundingBox(3) < MAX_BB_ASPECT ...
                && stats(j).BoundingBox(4) / stats(j).BoundingBox(3) > MIN_BB_ASPECT
            rectangle('Position',stats(j).BoundingBox, 'EdgeColor', 'r', 'LineWidth', 2)
        end
    end
    pause
end
