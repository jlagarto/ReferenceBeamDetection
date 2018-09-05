// SegDLL.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SegDLL.h"

using namespace cv;

// This is an example of an exported function.
SEGDLL_API int fnSegDLL(uchar *b, uchar *g, uchar *r, uchar *map_counts, uchar *map_lifetime, float lifetime, int min_thres, int max_thres, int rin, float *center_x, float *center_y, float *rout)
{
	
	int rows = 480;
	int cols = 640;
	
	// split image into rgb (b is chan[0], g is chan[1]. r is chan[2])
	Mat blue(rows, cols, CV_8U, &b[0]);
	Mat green(rows, cols, CV_8U, &g[0]);
	Mat red(rows, cols, CV_8U, &r[0]);
	Mat counts(rows, cols, CV_8U, &map_counts[0]);
	Mat taus(rows, cols, CV_8U, &map_lifetime[0]);

	Mat frame = red.clone();
	// merge channels
	Mat bgr;
	Mat merged[] = {blue, green, frame};
	merge(merged, 3, bgr);

	// only segment if a valid lifetime value is passed. lifetime = 0 is flag for non-valid lifetime value
	if (lifetime > 0.0)
	{
		
		// validate thresholds
		// set defaults
		int default_min = 230;
		int default_max = 255;
		int value_min = limit_thresholds(min_thres);
		int value_max = limit_thresholds(max_thres);

		if (value_min >= value_max)
		{
			value_min = default_min;
			value_max = default_max;
		}	
	
		// TODO: 
		// segmentation based on red channel only is prone to bugs. 
		// if light is too bright, white spots on the image due to reflection will also be segmented
		
		// threshold red channel
		threshold(frame, frame, value_min, value_max, THRESH_BINARY);
	
		// morphologic erosion (3,3 too low; 15,15 too high)
		Mat element = getStructuringElement(MORPH_ELLIPSE, Size(11, 11), Point(-1, -1)); // Create structured element of size 3
		
		// remove "noise"
		morphologyEx(frame, frame, MORPH_ERODE, element);

		// remove holes inside mask
		morphologyEx(frame, frame, MORPH_CLOSE, element);
	
		// find contours
		vector<vector<Point> > contours;
		//vector<Mat> contours;
		vector<Vec4i> hierarchy;
		findContours(frame, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

		// find region of maximum intensity (detect circle)
		Mat mask = frame.clone();
		Mat mask_inv;
		bitwise_not(frame, mask_inv);		

		// calculate average lifetime in matrix
		// previously set from 1.0 to 4.0
		float upper_lifetime = 3.0; // set upper boundary for colormap
		float lower_lifetime = 0.5; // lower boundary
	
		// normalize lifetime 
		if (lifetime > upper_lifetime)
			lifetime = upper_lifetime;
		else if (lifetime < lower_lifetime)
			lifetime = lower_lifetime;
		int corr_lifetime = 255 * (lifetime - lower_lifetime) / (upper_lifetime - lower_lifetime);

		// create segmentation mask
		Mat	local_mask = Mat::zeros(rows, cols, CV_8U);

		// set lifetime mask
		Mat local_lifetime = Mat(rows, cols, CV_8U);
		local_lifetime.setTo(Scalar(corr_lifetime), frame);
	
		if (contours.size() > 0)
		{

			// this is a small hack; 
			// when more than 1 region is found (due to artefacts eg too much light), look only for the largest region which will be defined by the true segmentation
			if (contours.size() > 1)
			{
				// sort contours by area
				sort(contours.begin(), contours.end(), compareContourAreas);

				// grab contours
				contours[0] = contours[contours.size() - 1];
			}

			vector<vector<Point>> contours_poly(contours.size());
			vector<Point2f>center(contours.size());
			vector<float>radius(contours.size());

			// draw polygon contour around detected area, with accuracy 3
			// make sure it is closed (set last parameter as true)
			int accuracy = 3;
			approxPolyDP(Mat(contours[0]), contours_poly[0], accuracy, true);
			
			// create circle enclosed polygon
			minEnclosingCircle((Mat)contours_poly[0], center[0], radius[0]);

			// adjust radius of segmentation
			float rad;
			if (rin == 0) 
			{
				// radius is automatically defined by segmentation only
				rad = radius[0];
			}
			else if (rin < 0)
			{
				// take radius as an absolute value
				rad = -1*rin;
			}
			else // rin > 0 
			{
				// relative radius
				rad = radius[0]*rin/100;
			}

			// draw circle in frame
			circle(frame, center[0], (int)rad, Scalar(255, 255, 255), CV_FILLED, 8, 0);

			// increment counts in regions where mask == 255
			local_mask.setTo(Scalar(1), frame);

			// loop through pixels in circular mask
			for (int x = center[0].x - rad; x < center[0].x + rad; x++)
			{
				// segmentation in x axis is out of range?
				if (x < 0 || x > cols)
					continue;

				for (int y = center[0].y - rad; y < center[0].y + rad; y++)
				{
					// segmentation in yaxis is out of range?
					if (y < 0 || y > rows)
						continue;

					// not mask?
					if (local_mask.at<uchar>(y, x) == 0)
						continue;

					// compute average lifetime
					taus.at<uchar>(y, x) = (taus.at<uchar>(y, x) * counts.at<uchar>(y, x) + corr_lifetime) / (counts.at<uchar>(y, x) + 1);
				}
			}

			(*center_x) = center[0].x;
			(*center_y) = center[0].y;
			(*rout) = rad;
		}
		else
		{
			(*center_x) = 0;
			(*center_y) = 0;
			(*rout) = 0;
		}
	
		// add counts in segmented regions
		add(counts, local_mask, counts);
	}
	
	// convert mask to rgb
	Mat overlay;
	applyColorMap(taus, overlay, COLORMAP_JET);

	// convert all blue pixels in image to black, to become transaparent as rgb
	Mat inverted_mask = Mat::ones(rows, cols, CV_8U);
	inverted_mask.setTo(Scalar(0), counts > 0);
	overlay.setTo(Scalar(0, 0, 0), inverted_mask);
	addWeighted(bgr, 0.5, overlay, 1, 0.0, bgr);
	
	// split
	vector<Mat> bgr_vec;
	split(bgr, bgr_vec);
	
	// copy to allocated memory
	memcpy(&b[0], &bgr_vec[0].data[0], sizeof(uchar)*cols*rows);
	memcpy(&g[0], &bgr_vec[1].data[0], sizeof(uchar)*cols*rows);
	memcpy(&r[0], &bgr_vec[2].data[0], sizeof(uchar)*cols*rows);

	return 0;
}

// comparison function object
bool compareContourAreas(vector<Point> contour1, vector<Point> contour2) {
	double i = fabs(contourArea(Mat(contour1)));
	double j = fabs(contourArea(Mat(contour2)));
	return (i < j);
}

int limit_thresholds(int value)
{
	if (value > 255)
		value = 255;
	else if (value < 0)
		value = 0;
	else 
	{
		// do nothing
	}

	return value;
}

// This is the constructor of a class that has been exported.
// see SegDLL.h for the class definition
CSegDLL::CSegDLL()
{
	return;
}
