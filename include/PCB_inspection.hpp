#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


float getSkewAngle(cv::Mat cvImage);

cv::Mat rotateImage(cv::Mat cvImage, float angle);

cv::Mat deskew(cv::Mat cvImage);
