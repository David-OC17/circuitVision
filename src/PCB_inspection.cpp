#include <vector>
#include <iostream>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/PCB_inspection.hpp"
#include "../include/aux_functions.hpp"

////////////////////////////////////////////////////////////////////////////////
///                             Preprocessing                                ///
////////////////////////////////////////////////////////////////////////////////

cv::Mat preprocessImg(cv::Mat inputImg){
  // Apply Gaussian blur
  cv::Mat bluredImg;
  cv::GaussianBlur(inputImg, bluredImg, cv::Size(3, 3), 0, cv::BORDER_DEFAULT);

  // Convert to grayscale
  cv::Mat bluredGrayImg;
  cv::cvtColor(bluredImg, bluredGrayImg, cv::COLOR_BGR2GRAY);

  // Apply thresholding
  cv::Mat threshBluredGrayImg;
  cv::threshold(bluredGrayImg, threshBluredGrayImg, 127, 255, cv::THRESH_BINARY_INV);

  // Apply closign morphological operation 
  cv::Mat finalImg;
  cv::morphologyEx(threshBluredGrayImg, finalImg, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

  return finalImg;
}

cv::Mat correctPerspective(cv::Mat inputImg, std::vector<cv::Point> corners, int width, int height){
  // Cast points
  std::vector<cv::Point2f> srcCorners = {static_cast<cv::Point2f>(corners[0]), static_cast<cv::Point2f>(corners[1]), static_cast<cv::Point2f>(corners[2]), static_cast<cv::Point2f>(corners[3])};

  // Destination corner
  std::vector<cv::Point2f> destCorners = {
    cv::Point2f(0,0),
    cv::Point2f(width - 1, 0),
    cv::Point2f(width - 1, height - 1),
    cv::Point2f(0, height - 1)
  };

  // Calculate transform from set of given corners to window corners (fit to all the window)
  cv::Mat imgTransform; 
  cv::getPerspectiveTransform(srcCorners, destCorners); 

  // Apply the transformation
  cv::Mat noPersp;
  cv::Size img_size(width, height);
  // cv::warpPerspective(inputImg, noPersp, imgTransform, img_size);

  return inputImg;
}

////////////////////////////////////////////////////////////////////////////////
///                             Fault finding                                ///
////////////////////////////////////////////////////////////////////////////////
 
void noise_removal(cv::Mat &XOR_img, int closure_iterations, int ind_operation_iterations){
  cv::Mat closure_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
  cv::Mat XOR_eroded_img;
  cv::Mat XOR_dialated_img;

  // Apply closure via erossion and dilation
  cv::erode(XOR_img, XOR_eroded_img, closure_kernel, cv::Point(-1, -1), ind_operation_iterations);
  cv::dilate(XOR_eroded_img, XOR_dialated_img, closure_kernel, cv::Point(-1, -1), ind_operation_iterations);

  XOR_img = XOR_dialated_img;
}

std::vector<cv::Point> findLargestContour(cv::Mat inputImg){
    // Detect the contours in the image
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(inputImg, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Find the largest contour
    int largestArea = 0;
    int largestContourIndex = -1;
    for (int i = 0; i < contours.size(); i++) {
        double area = cv::contourArea(contours[i]);
        if (area > largestArea) {
            largestArea = area;
            largestContourIndex = i;
        }
    }

    // Approximate the largest contour to a polygon
    std::vector<cv::Point> approx;
    cv::approxPolyDP(contours[largestContourIndex], approx, 0.01 * cv::arcLength(contours[largestContourIndex], true), true);

    // Check if the polygon has 4 vertices and a sufficiently large area
    if (approx.size() == 4 && largestArea > 1000) {
      // Give the coordinates of the corners of the largest rectangle
      std::cout << approx[0] << "\n";
      std::cout << approx[1] << "\n";
      std::cout << approx[2] << "\n";
      std::cout << approx[3] << "\n";

      return approx;

    }

    std::cout << "No large rectangle found in the image." << std::endl;
    std::vector<cv::Point> emptyRet;
    return emptyRet;
}

void fillPCBholes(cv::Mat &inputImg){
    // Invert the grayscale image
    cv::Mat des = 255 - inputImg;

    // Find contours in the inverted image
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(des, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);

    // Draw filled contours on the inverted image
    for (int i = 0; i < contours.size(); i++) {
        cv::drawContours(des, contours, i, cv::Scalar(255), -1);
    }

    // Invert the image back to get the final result
    inputImg = des;
}

cv::Mat getPCBmask(cv::Mat inputImg, std::vector<int> lowerLims, std::vector<int> upperLims){
  // Convert to HSV
  cv::Mat hsvImg;
  cv::cvtColor(inputImg, hsvImg, cv::COLOR_BGR2HSV);

  // Define the bound of the color to filter
  cv::Scalar lower_green(lowerLims[0], lowerLims[1], lowerLims[2]);
  cv::Scalar upper_green(upperLims[0], upperLims[1], upperLims[2]);

  // Create mask
  cv::Mat mask;
  cv::inRange(inputImg, lower_green, upper_green, mask);
  // Repair mask (remove noise and holes)
  closeMask(mask, 7);
  // floodMask(mask);
  display_img(mask, true);

  return mask;
}

cv::Mat colorFilterHSV(cv::Mat inputImg, std::vector<int> lowerLims, std::vector<int> upperLims){
  // Convert to HSV
  cv::Mat hsvImg;
  cv::cvtColor(inputImg, hsvImg, cv::COLOR_BGR2HSV);

  // Define the bound of the color to filter
  cv::Scalar lower_green(lowerLims[0], lowerLims[1], lowerLims[2]);
  cv::Scalar upper_green(upperLims[0], upperLims[1], upperLims[2]);

  // Create mask
  cv::Mat mask;
  cv::inRange(inputImg, lower_green, upper_green, mask);
  // Repair mask (remove noise and holes)
  closeMask(mask, 2);
  display_img(mask, true);


  // Aplly AND with the mask
  cv::Mat filteredImg;
  cv::bitwise_and(inputImg, inputImg, filteredImg, mask);

  // Convert back to BGR
  cv::Mat result;
  cv::cvtColor(filteredImg, result, cv::COLOR_HSV2BGR);

  return result;
}
