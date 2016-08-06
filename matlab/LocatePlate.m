function LocatePlate(rawInput)

warning off images:initSize:adjustingMag

if sum(size(rawInput)) == 2
    isVideo = 1;
    video = rawInput;
else
    isVideo = 0;
    video = VideoReader('yakvid.mp4');
end

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

while hasFrame(video)
    tic
    rawImg = readFrame(video);
    if ~isVideo; rawImg = rawInput; end 

    % For storing plate finalists
    validBoundingBoxes = [];
    validAreas = [];
    MIN_SEPARATION = size(rawImg,1) / 6;
    
    % Scale down the image to decrease runtime
    for l = 1:length(IDEAL_WIDTHS)
        scaleFactor = IDEAL_WIDTHS(l)/size(rawImg,2);
        if size(rawImg,2) > IDEAL_WIDTHS(l)
            scaledImg = imresize(rawImg, scaleFactor);
        else
            scaledImg = rawImg;
            scaleFactor = 1;
        end
        
        size(rawImg)
        size(scaledImg)

        grayImg = rgb2gray(scaledImg);
        gradientImg = imgradient(grayImg,'Sobel');
        closedImg = imclose(gradientImg, SE1);
        thCB = imtophat(closedImg, SE2);
        opened = imopen(thCB, SE1);
        blurred = imgaussfilt(opened,SIGMA);
        binary = blurred > 220;
        openedB = imopen(binary,SE3);
        dilatedB = imdilate(openedB,SE1);
        
        figure(1), imshow(grayImg,[])
        figure(2), imshow(gradientImg,[])
        figure(3), imshow(closedImg,[])
        figure(4), imshow(thCB,[])  
        figure(5), imshow(opened,[])    
        figure(6), imshow(blurred,[])   
        figure(7), imshow(binary,[])      
        figure(8), imshow(openedB,[])     
        figure(9), imshow(dilatedB,[])
        
        pause
        
        [L,nBlobs] = bwlabel(dilatedB);
        blobs = regionprops(L,'Area','Centroid','BoundingBox'); 
                
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
        aspectEdges = 0.1:0.1:0.8;
        
        % Plus one on second dim allows for the buffer that's used later
        % when taking maxes
        allAspectRatios = zeros( nBlobs, length(aspectEdges)+1 );

        for i = 1:nBlobs
            x0 = blobs(i).BoundingBox(1);
            y0 = blobs(i).BoundingBox(2);
            w  = blobs(i).BoundingBox(3);
            h  = blobs(i).BoundingBox(4);
            testBlob = grayImg(y0:y0+h-1, x0:x0+w-1);

            testBlob = ~im2bw(testBlob, graythresh(testBlob));

            [Lsub,nSubBlobs] = bwlabel(testBlob);
            subBlobs = regionprops(Lsub,'Area','Centroid','BoundingBox');        
            
            subAspectRatios = zeros(1,nSubBlobs);
            
            for k = 1:nSubBlobs
                subAspectRatios(k) = subBlobs(k).BoundingBox(3) / subBlobs(k).BoundingBox(4);                
            end
            
            [aspectCounts,~] = histcounts( subAspectRatios, aspectEdges );
            allAspectRatios(i,2:end-1) = aspectCounts;
        end
        
        for i = 1:nBlobs
            [~, maxAspectIndex] = max(allAspectRatios(i,:));

            % The if statement here is to prevent the case where no buckets are
            % filled (all sub blobs have big aspect ratios and aren't
            % considered), so maxIndex is 1
            if sum( allAspectRatios(i,:) ) ~= 0
                maxAspectGrouping = sum( allAspectRatios( i, maxAspectIndex-1:maxAspectIndex+1 ) );
                
                if maxAspectGrouping > NUM_OF_CHARS_CUTOFF && any([4,5] == maxAspectIndex)
                    validBoundingBoxes = [validBoundingBoxes; blobs(i).BoundingBox / scaleFactor];
                end
            end
        end
    end
    
    
    figure(199), imshow(rawImg,[])
    
    validBoundingBoxes = sortrows(validBoundingBoxes, 1);
    validAreas = validBoundingBoxes(:,3) .* validBoundingBoxes(:,4);
    
    if ~isempty(validAreas)
        maxArea = validAreas(1);
        maxIndex = 1;
        for i = 2:length(validAreas)
            twoPts = [ validBoundingBoxes(i,1:2); validBoundingBoxes(i-1,1:2) ];
            if pdist(twoPts, 'euclidean') > MIN_SEPARATION
                rectangle('Position', validBoundingBoxes(maxIndex,:), 'EdgeColor', 'g', 'LineWidth', 2)
                maxArea = validAreas(i);
                maxIndex = i;
            else
                if validAreas(i) > maxArea
                    maxArea = validAreas(i);
                    maxIndex = i;
                end
            end
        end
        rectangle('Position', validBoundingBoxes(maxIndex,:), 'EdgeColor', 'g', 'LineWidth', 2)
    end
    
    toc
    if ~isVideo
        break;
    end
    pause(0.001)
end

end