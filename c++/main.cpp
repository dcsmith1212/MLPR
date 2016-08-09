// g++ -std=c++11 main.cpp `pkg-config --cflags --libs opencv`
/* OPTIMIZATION TO-DO LIST
	* Convert all doubles to floats
*/

#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

using cv::Mat;

// Function initializations (add //** to a personal CV library)
std::vector<cv::Rect> find_bounding_boxes(cv::Mat image, double minAspectRatio, double minSolidity, double minAreaRatio); //**
void display_bounding_box(cv::Mat image, cv::Rect boundingBox); //**
cv::Mat pre_processing(cv::Mat rawFrame);

// Main
int main(int argc, char** argv) {
	// "Global" variables and constants
	Mat rawFrame;										// Holds raw input from the camera
	cv::VideoCapture cap;								// Object that maintains camera/video
	cv::namedWindow("Finalists", 0);					// Display and Mat for final output image
	Mat finalistsDisplay;								// (will show boxed license plate)s
	vector<cv::Rect> validBoundingBoxes;				// Holds final bounding boxes at each frame
	const vector<int> IDEAL_WIDTHS = {1280, 820, 512};	// The raw frame will be resize to all these widths for processing

	// Sets the input type (image, .mp4, or camera feed)
	const int IMAGE_TYPE = 0;
	if (IMAGE_TYPE == 0) { 
		string filename = "yakvid.mp4";		// Opens predefined video
		cv::VideoCapture pac(filename);
		cap = pac;
	} else if (IMAGE_TYPE == 1) {
		cout << "HEY" << endl;
    	cv::VideoCapture pac(1); 			// Opens the default camera
    	if(!pac.isOpened())  				// Checks if capture was successful
        	return -1;
		cap = pac;
	} else if (IMAGE_TYPE == 3) {			// Reads an image file
		rawFrame = cv::imread("yakxterra.jpg", CV_LOAD_IMAGE_COLOR);

		if(!rawFrame.data ) {				// Check for invalid input
		    cout <<  "Could not open or find the image" << endl ;
		    return -1;
		}
	}
	
	// Loops through video or passes image for plate recognition
    while (true) {
	    if ((IMAGE_TYPE == 0) || (IMAGE_TYPE == 1)) cap >> rawFrame; 	// Gets a new frame from camera

		// Increases the contrast of the video feed from USB camera
		if (IMAGE_TYPE == 1) {
			double alpha = 2;
			double beta = -120;
	 		for( int y = 0; y < rawFrame.rows; y++ ) { 
				for( int x = 0; x < rawFrame.cols; x++ ) {
		  	   		for( int c = 0; c < 3; c++ ) {
		  				rawFrame.at<cv::Vec3b>(y,x)[c] =
		     			cv::saturate_cast<uchar>( alpha*( rawFrame.at<cv::Vec3b>(y,x)[c] ) + beta );
		       		}
				}
			}
		}

		// Copies the raw frame for displaying boxed plate later
		finalistsDisplay = rawFrame.clone();

		// The processing is done for each of the sizes in IDEAL_WIDTHS
		// This helps to generalize the range at which the plate location works
		double rawWidth = rawFrame.cols;
		for (int newWidth : IDEAL_WIDTHS) {
			// Sets scale factor of image resize, so later all bounding boxes can be normalized back to original image
			double scaleFactor;
			if (rawWidth > newWidth) {
				scaleFactor = (double)newWidth / rawWidth;
				cv::Size newSize( newWidth, (int)round( rawFrame.rows * newWidth / rawFrame.cols) );
				cv::resize(rawFrame, rawFrame, newSize, 0, 0);
			}			
			else scaleFactor = 1;
	
			// Runs frame through preprocessing
			cv::Mat frame = pre_processing(rawFrame);
			// Finds bounding boxes in preprocessed frame
			vector<cv::Rect> boundingBoxes = find_bounding_boxes(frame, 0.98, 0.45, 2.7e-4);

			// Variables used to subsample image with a valid bbox and then find its contours.
			// If the subimage has >5 contours that could be plate numbers, it is sent on for further processing.
			Mat subImage;
			Mat graySub;
			Mat binarySub;

			int validityScore;
			cv::Rect subBox;
			double subAspectRatio;
			cv::Rect validScaled;

			for ( cv::Rect bbox : boundingBoxes ) {
				// Subsamples image at a valid bounding box and converts to binary using Otsu threshold
				subImage = rawFrame(bbox);	
				cv::cvtColor(subImage, graySub, CV_BGR2GRAY);
				graySub.convertTo(graySub, CV_8UC1, 1.0);
				cv::threshold(graySub, binarySub, 0, 255, CV_THRESH_BINARY_INV + CV_THRESH_OTSU);

				// Find contours of binarized subimage
				Mat binarySubClone = binarySub.clone();
				vector< vector<cv::Point> > subContours;
				cv::findContours(binarySubClone, subContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);	// Consider changing mode

				// Attempt at a simpler filtering algorithm than the one used with matlab (the Russian version)
				validityScore = 0;
				for (int k = 0; k < subContours.size(); k++) {
					subBox = cv::boundingRect(subContours[k]);
					subAspectRatio = (double)subBox.width / (double)subBox.height;	

					if (subAspectRatio >= 0.3 && subAspectRatio <= 0.5) validityScore++;
				}
			
				// If subimage has more than 5 blobs whose aspect ratios match those for plate numbers,
				// consider the subimage valid
				if (validityScore > 5) {
					validScaled.x = bbox.x / scaleFactor;
					validScaled.y = bbox.y / scaleFactor;
					validScaled.width  = bbox.width  / scaleFactor;
					validScaled.height = bbox.height / scaleFactor;
					validBoundingBoxes.push_back(validScaled);
				}
			}
		}

		// TODO: Currently the plate may be capture by more than one of the size.
		// Find a quick way to take the biggest bounding box around the plate if this happens.

		// Displays all the valid bounding boxes on the original image
		if (!validBoundingBoxes.empty()) {				
			for ( cv::Rect bbox : validBoundingBoxes ) 
				display_bounding_box(finalistsDisplay, bbox);
		}
		cv::imshow("Finalists", finalistsDisplay);

		// Clears vector for use with next frame
		validBoundingBoxes.clear();

		// Breaks loop if input is just an image
		if (IMAGE_TYPE == 3) {
			cv::waitKey(0);
			break;
		}
		else if(cv::waitKey(30) >= 0) break;
    }

    return 0;
}

//-----------------------------------------------------------------------------
// Functions

// Generalize this function by adding option to only store bboxes with certain area,
// by adding option to display bboxes
std::vector<cv::Rect> find_bounding_boxes(cv::Mat image, double minAspectRatio, double minSolidity, double minAreaRatio) {
	// Finds the contours of the given image
	std::vector< std::vector< cv::Point >> contours;
	cv::findContours(image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	
	// Finds the bounding box of each contour
	std:vector<cv::Rect> boundingBoxes;
	cv::Rect bbox;
	double bbArea, solidity, aspectRatio, imageSize, areaRatio;
	bool validBox;
	for ( std::vector<cv::Point> contour : contours ) {
		bbox = cv::boundingRect(contour);

		// Only returns the bounding box if it fits the given criteria
		bbArea      = (double)bbox.width * (double)bbox.height;
		solidity    = bbArea / (double)contourArea(contour);
		aspectRatio = bbox.width / bbox.height;
		imageSize   = image.rows * image.cols;
		areaRatio   = bbArea / imageSize;
		validBox    = (solidity > minSolidity) && (aspectRatio > minAspectRatio) 
								&& (areaRatio > minAreaRatio);
		if (validBox) boundingBoxes.push_back(bbox);
	}

	return boundingBoxes;
}


// Displays the given bounding box on the given image
void display_bounding_box(cv::Mat image, cv::Rect bbox) {
	// Must pass rectangle function the upper-left and lower-right points of bounding box
	cv::Point pt1, pt2;
	pt1.x = bbox.x;
	pt1.y = bbox.y;
	pt2.x = bbox.x + bbox.width;
	pt2.y = bbox.y + bbox.height;
	cv::rectangle(image, pt1, pt2, CV_RGB(0,0,255), 7);
}


// Filters the video frame before converting to a binary form which (ideally) highlights the plate
cv::Mat pre_processing(cv::Mat rawFrame) {
	cv::Mat grayFrame;
	cv::Mat sobel1;
	cv::Mat sobel2;

	// Structuring elements for closing, opening, and tophat
	cv::Mat strel1(8,  16, CV_8U, cv::Scalar(1));
	cv::Mat strel2(40, 80, CV_8U, cv::Scalar(1));
	cv::Mat strel3(15, 30, CV_8U, cv::Scalar(1));

	// Scales the image down for processing
	cv::Mat processedFrame(rawFrame.rows, rawFrame.cols, CV_32FC1);
	rawFrame.convertTo(rawFrame, CV_32FC1, 1.0/255.0);		

    cv::cvtColor(rawFrame, grayFrame, CV_BGR2GRAY);
	cv::Sobel(grayFrame, sobel1, -1, 1, 0, 3, 1.0/2.0);	
	cv::Sobel(grayFrame, sobel2, -1, 0, 1, 3, 1.0/2.0);	
	cv::magnitude(sobel1, sobel2, processedFrame);
	cv::morphologyEx(processedFrame, processedFrame, cv::MORPH_CLOSE, strel1);
	cv::morphologyEx(processedFrame, processedFrame, cv::MORPH_TOPHAT, strel2);
	cv::morphologyEx(processedFrame, processedFrame, cv::MORPH_OPEN, strel1);
	cv::GaussianBlur(processedFrame, processedFrame, cv::Size(0,0), 3, 3);
	processedFrame = 255.0 * processedFrame;
	cv::threshold(processedFrame, processedFrame, 140, 255, cv::THRESH_BINARY);
	cv::morphologyEx(processedFrame, processedFrame, cv::MORPH_OPEN, strel3);
	cv::morphologyEx(processedFrame, processedFrame, cv::MORPH_DILATE, strel1);
	processedFrame.convertTo(processedFrame, CV_8UC1, 1.0);

	return processedFrame;
}


