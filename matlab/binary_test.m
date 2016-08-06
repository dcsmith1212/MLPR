clear all
close all


% Goal here is to find plate and numbers by finding bounding boxes with ARs
% similar to plate that contain multiple bounding boxes with ARs similar to
% those for numbers



warning off images:initSize:adjustingMag

% Import raw image (will replace with camera feed)
rawImg = imread('cali.jpg');

% Constants
IDEAL_WIDTH = 516;
MIN_BLOB_AREA = round( IDEAL_WIDTH );              % / 13 for numbers
MAX_BLOB_AREA = round( IDEAL_WIDTH / 0.3) ;               % / 3.5 for numbers
MIN_TOT_ASPECT = 0.2;
MAX_TOT_ASPECT = 0.6;

MAX_THRESH_TOL = 40;
MIN_HORIZ_DIST = 5;
MAX_HORIZ_DIST = 10;

% Scale down the image to decrease runtime
if size(rawImg,2) > IDEAL_WIDTH
    rawImg = imresize(rawImg, IDEAL_WIDTH/size(rawImg,2));
end
imshow(rawImg,[])

% Range of threshold values for making binary image
threshVals = 10:10:240;

tic
allBlobs = struct([]);
for i = 1:length(threshVals)
    % Creates a binary image with the given threshold
    % Image is complemented so that license numbers are white on black
    % bacground
	binImg = im2bw(rawImg,threshVals(i)/255);
    %imshow(binImg,[])
    
    % Finds connected components in the binary image
	[L,M_indiv] = bwlabel(binImg);
	stats = regionprops(L,'Area','Centroid','BoundingBox');    
    
    for j = 1:M_indiv
        % Neglects blobs that are obviously too small
        if stats(j).Area > MIN_BLOB_AREA && stats(j).Area < MAX_BLOB_AREA ...
                && stats(j).BoundingBox(4) / stats(j).BoundingBox(3) < MAX_TOT_ASPECT ...
                && stats(j).BoundingBox(4) / stats(j).BoundingBox(3) > MIN_TOT_ASPECT
            % Store the blobs that fit the initial criteria
            stats(j).Thresh = threshVals(i);
            allBlobs = [allBlobs; stats(j)];
        end
    end
end
toc

% Display bounding boxes of all valid blobs
for i = 1:length(allBlobs)
    rectangle('Position', allBlobs(i).BoundingBox, 'EdgeColor', 'r', 'LineWidth', 2)
end