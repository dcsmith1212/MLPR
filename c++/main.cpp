#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

using cv::Mat;

int main(int argc, char** argv) {

/* OPTIMIZATION TO-DO LIST
	* Convert all doubles to floats
	* Consider linked lists or arrays instead of vectors
*/
// g++ -std=c++11 main.cpp `pkg-config --cflags --libs opencv`^C

	Mat rawFrame;
	Mat grayFrame;
	Mat sobel1;
	Mat sobel2;
	cv::VideoCapture cap;

	const int WHICH_IMAGE = 0;
	if (WHICH_IMAGE == 0) { 
		string filename = "yakvid.mp4";
		cv::VideoCapture pac(filename);
		cap = pac;
	} else if (WHICH_IMAGE == 1) {
		cout << "HEY" << endl;
    	cv::VideoCapture pac(1); 		// open the default camera
    	if(!pac.isOpened())  		// check if capture was successful
        	return -1;
		cap = pac;
	} else if (WHICH_IMAGE == 3) {
		rawFrame = cv::imread("yakxterra.jpg", CV_LOAD_IMAGE_COLOR);   // Read the file

		if(!rawFrame.data )                              // Check for invalid input
		{
		    cout <<  "Could not open or find the image" << endl ;
		    return -1;
		}
	}

	cv::namedWindow("Finalists", 0);

	const Mat SE1(8,  16, CV_8U, cv::Scalar(1));
	const Mat SE2(40, 80, CV_8U, cv::Scalar(1));
	const Mat SE3(15, 30, CV_8U, cv::Scalar(1));

	
	Mat finalistsDisplay;
	Mat originalFrame;

	
    while (true) {
		vector< vector<double> > validBoundingBoxes;
	    if ((WHICH_IMAGE == 0) || (WHICH_IMAGE == 1)) cap >> rawFrame; 	// get a new frame from camera

		Mat newTest = rawFrame.clone();

		if (WHICH_IMAGE == 1) {
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


		originalFrame = rawFrame.clone();
		finalistsDisplay = rawFrame.clone();

		const vector<int> IDEAL_WIDTHS = {1280, 820, 512};
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
	
			Mat frame(rawFrame.rows, rawFrame.cols, CV_32FC1);
			rawFrame.convertTo(rawFrame, CV_32FC1, 1.0/255.0);		

		    cv::cvtColor(rawFrame, grayFrame, CV_BGR2GRAY);
			cv::Sobel(grayFrame, sobel1, -1, 1, 0, 3, 1.0/2.0);	
			cv::Sobel(grayFrame, sobel2, -1, 0, 1, 3, 1.0/2.0);	
			cv::magnitude(sobel1, sobel2, frame);
			cv::namedWindow("FRM",0); cv::imshow("FRM", frame);
			cv::morphologyEx(frame, frame, cv::MORPH_CLOSE, SE1);
			cv::morphologyEx(frame, frame, cv::MORPH_TOPHAT, SE2);
			cv::morphologyEx(frame, frame, cv::MORPH_OPEN, SE1);
			cv::GaussianBlur(frame, frame, cv::Size(0,0), 3, 3);
			frame = 255.0 * frame;
			cv::threshold(frame, frame, 140, 255, cv::THRESH_BINARY);
			cv::morphologyEx(frame, frame, cv::MORPH_OPEN, SE3);
			cv::morphologyEx(frame, frame, cv::MORPH_DILATE, SE1);
			frame.convertTo(frame, CV_8UC1, 1.0);	

		    vector< vector<cv::Point> > contours;
			cv::findContours(frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

			Mat contoursImg = rawFrame.clone();

			Mat bbImg = rawFrame.clone();
			cv::Point pt1, pt2;
			vector<cv::Rect> boundingBoxes;


			double imageSize = frame.rows * frame.cols;
			cv::Rect bbox;
			double bbArea, areaRatio, aspectRatio;
			bool validBox;
			// Use range-based for loop or perhaps erase/delete idiom if speed is an issue
			for (int i = 0; i < contours.size(); i++) {
				bbox = cv::boundingRect(contours[i]);
				bbArea = bbox.width * bbox.height;
				areaRatio = bbArea / contourArea(contours[i]);
				aspectRatio = bbox.width / bbox.height;
				validBox = (areaRatio > 0.45) && (aspectRatio > 0.98) 
										&& (bbArea / imageSize > 2.7e-4);
				if (validBox)
					boundingBoxes.push_back(bbox);
			}


			cv::Scalar color2(0,255,0);
			Mat subImage;

			for (int i = 0; i < boundingBoxes.size(); i++) {
				cv::Point pt1, pt2;
				pt1.x = boundingBoxes[i].x;
				pt1.y = boundingBoxes[i].y;
				pt2.x = boundingBoxes[i].x + boundingBoxes[i].width;
				pt2.y = boundingBoxes[i].y + boundingBoxes[i].height;

				cv::Rect testBb = boundingBoxes[i];
				double x0 = testBb.x;
				double y0 = testBb.y;
				double w = testBb.width;
				double h = testBb.height;
			
				cv::Mat graySub;
				cv::Mat binarySub;

				subImage = rawFrame(boundingBoxes[i]);	
				cv::cvtColor(subImage, graySub, CV_BGR2GRAY);
				graySub.convertTo(graySub, CV_8UC1, 1.0);
				
				cv::threshold(graySub, binarySub, 0, 255, CV_THRESH_BINARY_INV + CV_THRESH_OTSU);

				Mat binarySub_CLONE = binarySub.clone();
			
				vector< vector<cv::Point> > subContours;
				cv::findContours(binarySub_CLONE, subContours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);	

				// Attempt at a simpler filtering algorithm than the one used with matlab
				int validityScore = 0;
				for (int k = 0; k < subContours.size(); k++) {
					cv::Rect subBox = cv::boundingRect(subContours[k]);
					double subAspectRatio = (double)subBox.width / (double)subBox.height;	

					if (subAspectRatio >= 0.3 && subAspectRatio <= 0.5) validityScore++;
				}
			
				if (validityScore > 5) {
					double x0 = boundingBoxes[i].x / scaleFactor;
					double y0 = boundingBoxes[i].y / scaleFactor;
					double w = boundingBoxes[i].width / scaleFactor;
					double h = boundingBoxes[i].height / scaleFactor;
					vector<double> validScaled = {x0, y0, w, h};
					validBoundingBoxes.push_back(validScaled);
				}
			}
		}


		cv::Point pt3, pt4;
		if (!validBoundingBoxes.empty()) {	
			std::sort(validBoundingBoxes.begin(), validBoundingBoxes.end(), 
				[](const vector<double>& vec1, const vector<double>& vec2) {return vec1[0] < vec2[0];} );
			
			double maxArea = validBoundingBoxes[0][2] * validBoundingBoxes[0][3];
			int maxIndex   = 0;

			for (int i = 1; i < validBoundingBoxes.size(); i++) {
				cv::Vec<double,2> prevPt, currPt;
				currPt[0] = validBoundingBoxes[i][0];
				currPt[1] = validBoundingBoxes[i][1];
				prevPt[0] = validBoundingBoxes[i-1][0];
				prevPt[1] = validBoundingBoxes[i-1][1];

				if ( norm(prevPt - currPt) > (double)originalFrame.rows / 6.0 ) {	// Un-harcode this!!
					pt3.x = validBoundingBoxes[maxIndex][0];
					pt3.y = validBoundingBoxes[maxIndex][1];
					pt4.x = validBoundingBoxes[maxIndex][0] + validBoundingBoxes[maxIndex][2];
					pt4.y = validBoundingBoxes[maxIndex][1] + validBoundingBoxes[maxIndex][3];
					cv::rectangle(finalistsDisplay, pt3, pt4, CV_RGB(0,0,255), 7);

					maxArea = validBoundingBoxes[i][2] * validBoundingBoxes[i][3];
					maxIndex = i;
				}
				else {
					if ( (double)validBoundingBoxes[i][2] * (double)validBoundingBoxes[i][3] > maxArea ) {
						maxArea = (double)validBoundingBoxes[i][2] * (double)validBoundingBoxes[i][3];
						maxIndex = i;
					}
				}
			}

			pt3.x = validBoundingBoxes[maxIndex][0];
			pt3.y = validBoundingBoxes[maxIndex][1];
			pt4.x = validBoundingBoxes[maxIndex][0] + validBoundingBoxes[maxIndex][2];
			pt4.y = validBoundingBoxes[maxIndex][1] + validBoundingBoxes[maxIndex][3];
			cv::rectangle(finalistsDisplay, pt3, pt4, CV_RGB(0,0,255), 7);
		}

		cv::imshow("Finalists", finalistsDisplay);

		if (WHICH_IMAGE == 3) {
			cv::waitKey(0);
			break;
		}
		else if(cv::waitKey(30) >= 0) break;
    }

    return 0;
}
