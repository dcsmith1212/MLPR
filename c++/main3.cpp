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
using cv::namedWindow;
using cv::imshow;
using cv::imread;
using cv::Size;
using cv::resize;
using cv::Rect;
using cv::waitKey;
using cv::cvtColor;

void showim(string, cv::Mat);

int main(int argc, char** argv) {
	Mat rawFrame = imread("yakxterra.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file

	//Check for valid input image
	if(!rawFrame.data ) {
	    cout <<  "Could not open or find the image" << endl ;
	    return -1;
	}

	Mat grayFrame, binary;
	cvtColor(rawFrame, grayFrame, CV_BGR2GRAY);

	showim("Gray", grayFrame);
	for (int thresh = 10; thresh <= 220; thresh+=10) {
	}

	waitKey(0);
	return 0;
}


void showim(string windowName, cv::Mat image) {
	cv::namedWindow(windowName, 0);
	cv::imshow(windowName, image);
}

