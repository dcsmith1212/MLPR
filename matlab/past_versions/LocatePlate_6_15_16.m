clear all
close all
warning off MATLAB:colon:nonIntegerIndex
warning off images:initSize:adjustingMag

IDEAL_WIDTHS = [1280 820 512];

% Should automate this size based on how big the plate is thought to be
% For multiple-plate detection this would require using different strels
% for different parts of the image
SE1_height = 8;
SE1 = strel('rectangle', SE1_height*[1 2]);
SE2_height = 40;
SE2 = strel('rectangle', SE2_height*[1 2]);
SE3_height = 15;
SE3 = strel('rectangle', SE3_height*[1 2]);

SIGMA = 4;

bigBlurs = [0.01 2 4];
MIN_SUB_BLOB_AR = 0.3;
MAX_SUB_BLOB_AR = 0.8;
NUM_OF_CHARS_CUTOFF = 5;

video = VideoReader('yakvid.mp4');


while hasFrame(video)
    tic
    rawImg = readFrame(video);
    %rawImg = imread('rav4.jpg');
    rawGray = rgb2gray(rawImg);

    % For storing plate finalists
    validBoundingBoxes = [];

            
    % Scale down the image to decrease runtime
    for l = 1:length(IDEAL_WIDTHS)
        if size(rawImg,2) > IDEAL_WIDTHS(l)
            scaledImg = imresize(rawImg, IDEAL_WIDTHS(l)/size(rawImg,2));
        else
            scaledImg = rawImg;
        end


        imshow(scaledImg,[])

        grayImg = rgb2gray(scaledImg);
        %figure(1), imshow(grayImg,[])

        gradientImg = imgradient(grayImg,'Sobel');
        %figure(2), imshow(gradientImg,[])

        closedImg = imclose(gradientImg, SE1);
        %figure(3), imshow(closedImg,[])

        thCB = imtophat(closedImg, SE2);
        %figure(4), imshow(thCB,[])

        opened = imopen(thCB, SE1);
        %figure(5), imshow(opened,[])

        blurred = imgaussfilt(opened,SIGMA);
        %figure(6), imshow(blurred,[])

        binary = blurred > 220;
        %figure(7), imshow(binary,[])

        openedB = imopen(binary,SE3);
        %figure(8), imshow(openedB,[])

        dilatedB = imdilate(openedB,SE1);
        %figure(9), imshow(dilatedB,[])

        
        [L,nBlobs] = bwlabel(dilatedB);
        blobs = regionprops(L,'Area','Centroid','BoundingBox'); 

        %%%%
        invalidBlobs = [];
        for i = 1:nBlobs
            bbArea = blobs(i).BoundingBox(3) * blobs(i).BoundingBox(4);
            areaRatio = blobs(i).Area / bbArea;
            aspectRatio = blobs(i).BoundingBox(3) / blobs(i).BoundingBox(4);
            if areaRatio < 0.45 || aspectRatio < 1.2 || bbArea / numel(scaledImg) < 2.7e-04 
                invalidBlobs = [invalidBlobs; i];
            end
        end

        blobs(invalidBlobs) = [];
        nBlobs = length(blobs);

%         % Display bounding boxes on sub-sampled raw image
%         for i = 1:nBlobs
%             rectangle('Position', blobs(i).BoundingBox, 'EdgeColor', 'r', 'LineWidth', 2)
%         end
        %%%%

        % Initialization of cell arrays that hold maps for aspect ratios
        % and areas        
        aspectEdges = 0.1:0.1:0.8;
        areaEdges   = 10:20:300;
        
        % Plus one on second dim allows for the buffer that's used later
        % when taking maxes
        allAspectRatios = zeros( nBlobs, length(aspectEdges)+1 );
        allAreas        = zeros( nBlobs, length(areaEdges)+1 );

        %%%
        % Do this stuff in the above loop instead of removing invalid blobs
        % first

        for i = 1:nBlobs
            x0 = blobs(i).BoundingBox(1);
            y0 = blobs(i).BoundingBox(2);
            w  = blobs(i).BoundingBox(3);
            h  = blobs(i).BoundingBox(4);
            testBlob = grayImg(y0:y0+h-1, x0:x0+w-1);

            testBlob = ~im2bw(testBlob, graythresh(testBlob));
            %figure(123), imshow(testBlob,[])

            [Lsub,nSubBlobs] = bwlabel(testBlob);
            subBlobs = regionprops(Lsub,'Area','Centroid','BoundingBox');        
            
            subAspectRatios = zeros(1,nSubBlobs);
            subAreas = [subBlobs.Area];
            
            for k = 1:nSubBlobs
                subAspectRatios(k) = subBlobs(k).BoundingBox(3) / subBlobs(k).BoundingBox(4);                

    %             % This code attempts to separate invalid blobs from those
    %             bbArea2 = subBlobs(k).BoundingBox(3) * subBlobs(k).BoundingBox(4);           
    %             that contain numbers using aspect ratios and the solidity of
    %             the blob. Perhaps this will be useful after the correct
    %             macro-blob has been found.
    %             if subAspectRatio > MIN_SUB_BLOB_AR && subAspectRatio < MAX_SUB_BLOB_AR && ...
    %                     subBlobs(k).Area / bbArea2 < 0.7 
    %                 rectangle('Position', subBlobs(k).BoundingBox, 'EdgeColor', 'g', 'LineWidth', 2)
    %             else
    %                 rectangle('Position', subBlobs(k).BoundingBox, 'EdgeColor', 'r', 'LineWidth', 2)
    %             end
    
            end
            
            [aspectCounts,~] = histcounts( subAspectRatios, aspectEdges );
            [areaCounts,~]   = histcounts( subAreas, areaEdges );

            allAspectRatios(i,2:end-1) = aspectCounts;
            allAreas(i,2:end-1) = areaCounts;
        end
        
        for i = 1:nBlobs
            [~, maxAspectIndex] = max(allAspectRatios(i,:));
            [~, maxAreaIndex]   = max(allAreas(i,:));

            % The if statement here is to prevent the case where no buckets are
            % filled (all sub blobs have big aspect ratios and aren't
            % considered), so maxIndex is 1. Might be a better way to do it.
            if sum( allAspectRatios(i,:) ) ~= 0 && sum( allAreas(i,:) ) ~= 0
                maxAspectGrouping = sum( allAspectRatios( i, maxAspectIndex-1:maxAspectIndex+1 ) );
                maxAreaGrouping   = sum( allAreas( i, maxAreaIndex-1:maxAreaIndex+1 ) );
                
                if maxAspectGrouping > NUM_OF_CHARS_CUTOFF && any([4,5] == maxAspectIndex) %&& maxAreaGrouping > NUM_OF_CHARS_CUTOFF
                    validBoundingBoxes = [validBoundingBoxes; blobs(i).BoundingBox * size(rawImg,2) / IDEAL_WIDTHS(l)];
                    rectangle('Position', blobs(i).BoundingBox, 'EdgeColor', 'g', 'LineWidth', 2)
                end
            end
        end
        pause
    end
end
