#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

using cv::Mat;
using cv::namedWindow;
using cv::imshow;

int main(int argc, char** argv) {

	Mat rawFrame;
	Mat grayFrame;
	cv::VideoCapture cap;

	// Decides if I'm using a .jpg, .mp4, or video feed from USB
	const int WHICH_IMAGE = 3;
	if (WHICH_IMAGE == 0) { 
		string filename = "yakvid.mp4";
		cv::VideoCapture pac(filename);
		cap = pac;
	} else if (WHICH_IMAGE == 1) {
    	cv::VideoCapture pac(1); 		// open the default camera
    	if(!pac.isOpened())  		// check if capture was successful
        	return -1;
		cap = pac;
	} else if (WHICH_IMAGE == 3) {
		rawFrame = cv::imread("yakxterra.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file
		if(!rawFrame.data ) {                             // Check for invalid input
		    cout <<  "Could not open or find the image" << endl ;
		    return -1;
		}
	}


	while (true) {
	    if ((WHICH_IMAGE == 0) || (WHICH_IMAGE == 1)) cap >> rawFrame; 	// get a new frame from camera

		namedWindow("RAW", 0); imshow("RAW", rawFrame);






		const IDEAL_WIDTH = 1280; // {1280, 820, 512};
		double rawWidth = rawFrame.cols;	
		for (int newWidth : IDEAL_WIDTHS) {
			// Could define newWidth as constant in app
			double scaleFactor;
			if (rawWidth > newWidth) {
				scaleFactor = (double)newWidth / rawWidth;
				cv::Size newSize( newWidth, (int)round( rawFrame.rows * newWidth / rawFrame.cols) );
				cv::resize(rawFrame, rawFrame, newSize, 0, 0);
			}			
			else scaleFactor = 1;













		if (WHICH_IMAGE == 3) {
			cv::waitKey(0);
			break;
		} else if(cv::waitKey(30) >= 0) break;
    }

    return 0;
}
