#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

////////////////////////////////////////////////////////////////////////////////
///                                 Deskew                                   ///
////////////////////////////////////////////////////////////////////////////////

float getSkewAngle(cv::Mat cvImage) {
    // Prep image, copy, convert to gray scale, blur, and threshold
    cv::Mat newImage = cvImage.clone();
    cv::Mat gray;
    cv::cvtColor(newImage, gray, cv::COLOR_BGR2GRAY);
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(9, 9), 0);
    cv::Mat thresh;
    cv::threshold(blur, thresh, 0, 255, cv::THRESH_BINARY_INV + cv::THRESH_OTSU);

    // Apply dilate to merge text into meaningful lines/paragraphs.
    // Use larger kernel on X axis to merge characters into single line, cancelling out any spaces.
    // But use smaller kernel on Y axis to separate between different blocks of text
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 5));
    cv::Mat dilate;
    cv::dilate(thresh, dilate, kernel, cv::Point(-1, -1), 5);

    // Find all contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(dilate, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    std::sort(contours.begin(), contours.end(), [](const std::vector<cv::Point>& a, const std::vector<cv::Point>& b) {
        return cv::contourArea(a) > cv::contourArea(b);
    });

    // Find largest contour and surround in min area box
    std::vector<cv::Point> largestContour = contours[0];
    cv::RotatedRect minAreaRect = cv::minAreaRect(largestContour);

    // Determine the angle. Convert it to the value that was originally used to obtain skewed image
    float angle = minAreaRect.angle;
    if (angle < -45) {
        angle = 90 + angle;
    }
    return -1.0 * angle;
}

cv::Mat rotateImage(cv::Mat cvImage, float angle) {
    cv::Mat newImage = cvImage.clone();
    int h = newImage.rows;
    int w = newImage.cols;
    cv::Point2f center(w / 2, h / 2);
    cv::Mat M = cv::getRotationMatrix2D(center, angle, 1.0);
    cv::Mat rotatedImage;
    cv::warpAffine(newImage, rotatedImage, M, cv::Size(w, h), cv::INTER_CUBIC, cv::BORDER_REPLICATE);
    return rotatedImage;
}

cv::Mat deskew(cv::Mat cvImage) {
    float angle = getSkewAngle(cvImage);
    return rotateImage(cvImage, -1.0 * angle);
}

////////////////////////////////////////////////////////////////////////////////
///                               Homography                                 ///
////////////////////////////////////////////////////////////////////////////////

