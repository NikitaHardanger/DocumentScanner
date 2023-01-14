#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;


Mat imgOriginal, imgGray, imgBlur, imgCanny, imgThreshold, imgDilation, imgErode, imgWarp, imgCrop;
vector<Point> initialPoints, documentPoints;

float width = 420, height = 596;

Mat preProcessing(Mat img) {

	cvtColor(img, imgGray, COLOR_BGR2GRAY);			// Images to Gray
	GaussianBlur(img, imgBlur, Size(3, 3), 3, 0);	// Blur Images
	Canny(imgBlur, imgCanny, 50, 150);				// Edges Images

	Mat kernel = getStructuringElement(MORPH_RECT, Size(1, 1));
	dilate(imgCanny, imgDilation, kernel);
	//erode(imgDilation, imgErode, kernel);

	return imgDilation;
}

vector<Point> getContours(Mat image) {

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;


	findContours(image, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

	vector<vector<Point>> conPoly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point> biggest;								// Vector for biggest area in screen

	int maxArea = 0;

	for (int i = 0; i < contours.size(); i++) {

		int area = contourArea(contours[i]);
		cout << area << endl;

		string objectType;

		if (area > 1000) {
			float peri = arcLength(contours[i], true);
			approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);

			if (area > maxArea && conPoly[i].size() == 4) {

				drawContours(imgOriginal, conPoly, i, Scalar(255, 0, 255), 5);
				biggest = { conPoly[i][0], conPoly[i][1], conPoly[i][2], conPoly[i][3] };
				maxArea = area;
			}
		}
	}
	return biggest;
}

void drawPoints(vector<Point> points, Scalar color) {

	for (int i = 0; i < points.size(); i++) {

		circle(imgOriginal, points[i], 7, color, FILLED);
		putText(imgOriginal, to_string(i + 1), points[i], FONT_HERSHEY_PLAIN, 3, color, 3);
	}
}

vector<Point> reorder(vector<Point> points) {
	vector<Point> newPoints;
	vector<int> sumPoints, subPoints;

	for (int i = 0; i < 4; i++) {
		sumPoints.push_back(points[i].x + points[i].y);
		subPoints.push_back(points[i].x - points[i].y);
	}

	newPoints.push_back(points[min_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // Number 1 for point
	newPoints.push_back(points[max_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); // Number 2 for point
	newPoints.push_back(points[min_element(subPoints.begin(), subPoints.end()) - subPoints.begin()]); // Number 3 for point
	newPoints.push_back(points[max_element(sumPoints.begin(), sumPoints.end()) - sumPoints.begin()]); // Number 4 for point

	return newPoints;
}

Mat getWarp(Mat img, vector<Point> points, float width, float height) {
	Point2f src[4] = { points[0], points[1], points[2], points[3] };
	Point2f dst[4] = { {0.0f,0.0f}, {width, 0.0f},{0.0f, height}, {width,height} };
	Mat matrix = getPerspectiveTransform(src, dst);
	warpPerspective(img, imgWarp, matrix, Point(width, height));

	return imgWarp;
}

int main() {

	string path = "Resources/document.jpg";
	imgOriginal = imread(path);
	//resize(imgOriginal, imgOriginal, Size(), 0.5, 0.5);   // resize original image and scale him

	// Preprocessing
	imgThreshold = preProcessing(imgOriginal);

	// Get contours - biggest
	initialPoints = getContours(imgThreshold);
	documentPoints = reorder(initialPoints);

	// Warp
	imgWarp = getWarp(imgOriginal, documentPoints, width, height);

	// Crop
	int cropValue = 10;
	Rect roi(cropValue, cropValue, width - (2 * cropValue), height - (2 * cropValue));
	imgCrop = imgWarp(roi);

	imshow("Image", imgOriginal);
	imshow("Image Dilation", imgThreshold);
	imshow("Image Warp", imgWarp);				// Show image without Crop
	imshow("Image Сrop", imgCrop);
	waitKey(0);

	return 0;
}