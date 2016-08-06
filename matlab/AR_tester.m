clear all
close all

plate = imread('testplate4.jpg');
plate = ~im2bw(plate, graythresh(plate));
plate = imgaussfilt(uint8(plate), 1);

imshow(plate,[])

[L,nBlobs] = bwlabel(plate);
blobs = regionprops(L,'Area','Centroid','BoundingBox'); 

for i = 1:nBlobs
    rectangle('Position',blobs(i).BoundingBox,'EdgeColor', 'r', 'LineWidth', 2)
    aspectRatio = blobs(i).BoundingBox(3) / blobs(i).BoundingBox(4);
    pause
end

