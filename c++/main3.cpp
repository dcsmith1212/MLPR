// g++ -std=c++11 main3.cpp `pkg-config --cflags --libs opencv`

#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

using cv::Mat;
using cv::Point;
using cv::threshold;
using cv::findContours;
using cv::drawContours;
using cv::namedWindow;
using cv::imshow;
using cv::imread;
using cv::Size;
using cv::resize;
using cv::Rect;
using cv::waitKey;
using cv::cvtColor;

void showim(string, cv::Mat);
std::vector<cv::Rect> findBoundingBoxes(cv::Mat image);

int main(int argc, char** argv) {
	Mat rawFrame = imread("yakxterra.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file

	//Check for valid input image
	if(!rawFrame.data ) {
	    cout <<  "Could not open or find the image" << endl ;
	    return -1;
	}

	Mat grayFrame, blurred, binary;
	cvtColor(rawFrame, grayFrame, CV_BGR2GRAY);
	cv::GaussianBlur(grayFrame, blurred, cv::Size(0,0), 3, 3);
	showim("Gray", blurred);

	Mat bbImg = rawFrame.clone();
	namedWindow("Boxes", 0);

	vector<cv::Rect> validBoundingBoxes;


	int minArea = 1500;
	int maxArea = 5000;
	// This loop iterates of thresh values, finding the bounding boxes for each resulting image that lie within the area and aspect ration constraints.
	for (int thresh = 10; thresh <= 220; thresh+=10) {
		threshold(grayFrame, binary, thresh, 255, cv::THRESH_BINARY_INV);
		vector<cv::Rect> boundingBoxes = findBoundingBoxes(binary);

		// This will all be generalized in the above function
		cv::Point pt1, pt2;
		for (int i = 0; i < boundingBoxes.size(); i++) {
			cv::Rect bbox = boundingBoxes[i];
			pt1.x = bbox.x;
			pt1.y = bbox.y;
			pt2.x = bbox.x + bbox.width;
			pt2.y = bbox.y + bbox.height;

			double bbArea = (double)bbox.width * (double)bbox.height;
			double bbRatio = (double)bbox.width / (double)bbox.height;

			// Relate these tolerances back to the image size for generality
			if ((bbArea > minArea) && (bbArea < maxArea) && (bbRatio > 0.3) && (bbRatio < 0.5)) {
				cv::rectangle(bbImg, pt1, pt2, CV_RGB(0,255,0), 4);
				validBoundingBoxes.push_back(bbox);
			}
		}
	}

	imshow("Boxes", bbImg);


	int nAreaBins = 30;
	int xBins = 15;
	int yBins = 20;

	double areaBinSize = (double)(maxArea - minArea) / (double)nAreaBins;
	vector<int> areaHist(nAreaBins);
	// This code loops through the valid bounding boxes and separates them into a 3D histogram based on x location, y lcoation, and area.

	int areaIndx;
	double bbArea;
	for ( cv::Rect bbox : validBoundingBoxes ) {
		bbArea = (double)bbox.width * (double)bbox.height;
		areaIndx = (int)((bbArea - minArea) / areaBinSize);
		areaHist[areaIndx] += 1;
	}

	
	for ( int i : areaHist )
		cout << i << endl;	



	waitKey(0);
	return 0;
}


void showim(string windowName, cv::Mat image) {
	cv::namedWindow(windowName, 0);
	cv::imshow(windowName, image);
}


// Generalize this function by adding option to only store bboxes with certain area,
// by adding option to display bboxes
std::vector<cv::Rect> findBoundingBoxes(cv::Mat image) {
	std::vector< std::vector< cv::Point >> contours;
	cv::findContours(image, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
	
	cv::Point pt1, pt2;
	std:vector<cv::Rect> boundingBoxes;
	cv::Rect bbox;

	for ( std::vector<cv::Point> contour : contours ) {
		bbox = cv::boundingRect(contour);
		boundingBoxes.push_back(bbox);
	}

	return boundingBoxes;
} 










