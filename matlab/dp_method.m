clear all
close all
warning off images:initSize:adjustingMag

% Import raw image (will replace with camera feed)
rawImg = imread('cali.jpg');

% Constants
IDEAL_WIDTH = 516;
MIN_BLOB_AREA = round( IDEAL_WIDTH / 13 );
MAX_BLOB_AREA = round( IDEAL_WIDTH / 3.5) ;
MIN_BB_ASPECT = 0.2;
MAX_BB_ASPECT = 1;
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
	binImg = ~im2bw(rawImg,threshVals(i)/255);
    %imshow(binImg,[])
     
    % Finds connected components in the binary image
	[L,M_indiv] = bwlabel(binImg);
	stats = regionprops(L,'Area','Centroid','BoundingBox');    
    
    for j = 1:M_indiv
        % Neglects blobs that are obviously too small
        if stats(j).Area > MIN_BLOB_AREA && stats(j).Area < MAX_BLOB_AREA ...
                && stats(j).BoundingBox(3) / stats(j).BoundingBox(4) < MAX_BB_ASPECT ...
                && stats(j).BoundingBox(3) / stats(j).BoundingBox(4) > MIN_BB_ASPECT
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



% Finding four consecutive blobs
N = 3;
M = length(allBlobs);
S = zeros(N,M);
B = zeros(N,M);   B(1,:) = 1:M;

% Minimization constants
ALPHA = 0.1;
BETA = 0.1;
GAMMA = 0.01;

% Three components of energy function (for minimization)
E_h = @(v1,v2) ALPHA*abs( v1.BoundingBox(4) - v2.BoundingBox(4) );
E_r = @(v1,v2)  BETA*abs( v1.Centroid(2) - v2.Centroid(2) );
E_t = @(v1,v2) GAMMA*abs( v1.Thresh - v2.Thresh );
E_TOTAL = @(v1,v2) E_h(v1,v2) + E_r(v1,v2) + E_t(v1,v2);

% Tests if two blobs are within the valid horizontal distance range of one
% another
in_h_range = @(v1,v2) abs( v1.Centroid(1) - v2.Centroid(1) ) < MAX_HORIZ_DIST ...
                    && abs( v1.Centroid(1) - v2.Centroid(1) ) > MIN_HORIZ_DIST;
                
% % Display the valid canditate blobs based on the horizontal distance range
% for i = 1:M
%     figure(42)
%     imshow(rawImg,[])
%     rectangle('Position',allBlobs(i).BoundingBox, 'EdgeColor', 'r', 'LineWidth', 2)
%     'iiwoubviwuebvowuebviwuevbiwevbiwuebv'
%     i
%     for j = 1:M
%         if i ~= j
%             if in_h_range( allBlobs(i), allBlobs(j) )
%                 rectangle('Position',allBlobs(j).BoundingBox, 'EdgeColor', 'c', 'LineWidth', 2)
%                 j
%             end
%         end
%     end
%     pause
% end


tic
allEnergies = zeros(M,M);
validHorizontals = ones(M,M);
minergies = zeros(N-1,M);
mindices  = zeros(N-1,M);

% Finds the energies between each pair of blobs
for i = 1:M
    for j = 1:M
        allEnergies(i,j) = E_TOTAL( allBlobs(i), allBlobs(j) );
        if ~in_h_range( allBlobs(i), allBlobs(j) )
            validHorizontals(i,j) = Inf;
        end
    end
end

% Sets those blobs that are compared to themselves to infinite energy (so
% minimum works out)
allEnergies = allEnergies + diag(Inf*ones(1,M));
allEnergies = allEnergies.*validHorizontals;

% Stores the three closest blobs to each blob (based on energy), recording
% the energies between them and the indices of those three blobs
for i = 1:N-1
    [mins, inds] = min(allEnergies);
    minergies(i,:) = mins;
    mindices(i,:)  = inds;

    for k = 1:M
        allEnergies(inds(k),k) = Inf;
    end
end
toc


B(2,:) = mindices(1,:);
S(2,:) = minergies(1,:);

tic
for n = 3:N
    for m = 1:M
        index = B(n-1,m);
        for k = 1:N-1
            if ~any( B(:,m) == mindices(k,index) )
                B(n,m) = mindices(k,index);
                S(n,m) = minergies(k,index);
                break;
            end
        end
    end
end

totalEnergies = sum(S,1);
[~,optimalIndex] = min(totalEnergies);
optimalBlobs = B(:,optimalIndex);
figure(321)
imshow(rawImg,[])

figure(444)
imshow(rawImg,[])
rectangle('Position',allBlobs(optimalBlobs(1)).BoundingBox, 'EdgeColor', 'm', 'LineWidth', 2)
rectangle('Position',allBlobs(optimalBlobs(2)).BoundingBox, 'EdgeColor', 'c', 'LineWidth', 2)
rectangle('Position',allBlobs(optimalBlobs(3)).BoundingBox, 'EdgeColor', 'g', 'LineWidth', 2)


toc









