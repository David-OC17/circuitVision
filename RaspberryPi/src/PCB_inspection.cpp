/**
 * @file
 * @author David Ortiz
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * @section DESCRIPTION
 *
 * This file contains the implementation of functions for
 * preprocessing of images, for the process of automatic PCB
 * optical inspection.
 */

#include <algorithm>
#include <iostream>
#include <vector>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "../include/PCB_inspection.hpp"
#include "../include/aux_functions.hpp"
#include "../include/csv.hpp"

/************************************************
 *              Preprocessing
 ***********************************************/

cv::Mat preprocessImg(cv::Mat inputImg) {
  // Apply Gaussian blur
  cv::Mat bluredImg;
  cv::GaussianBlur(inputImg, bluredImg, cv::Size(3, 3), 0, cv::BORDER_DEFAULT);

  // Convert to grayscale
  cv::Mat bluredGrayImg;
  cv::cvtColor(bluredImg, bluredGrayImg, cv::COLOR_BGR2GRAY);

  // Apply thresholding
  cv::Mat threshBluredGrayImg;
  cv::threshold(bluredGrayImg, threshBluredGrayImg, 127, 255,
                cv::THRESH_BINARY_INV);

  // Apply closign morphological operation
  cv::Mat finalImg;
  cv::morphologyEx(threshBluredGrayImg, finalImg, cv::MORPH_CLOSE,
                   cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

  return finalImg;
}

cv::Mat correctPerspective(cv::Mat inputImg, std::vector<cv::Point> corners,
                           int width, int height) {
  // Sort source corners to match destination corners in the transformation
  // calculation
  std::sort(corners.begin(), corners.end(), comparePoints);

  // Cast points
  std::vector<cv::Point2f> srcCorners = {static_cast<cv::Point2f>(corners[0]),
                                         static_cast<cv::Point2f>(corners[1]),
                                         static_cast<cv::Point2f>(corners[2]),
                                         static_cast<cv::Point2f>(corners[3])};

  // Destination corner
  std::vector<cv::Point2f> dstCorners = {
      cv::Point2f(0, 0), cv::Point2f(width - 1, 0),
      cv::Point2f(width - 1, height - 1), cv::Point2f(0, height - 1)};

  // Calculate transform from set of given corners to window corners (fit to all
  // the window)
  cv::Mat imgTransform;
  imgTransform = cv::getPerspectiveTransform(srcCorners, dstCorners);

  // std::cout << "imgTransform type: " << imgTransform.type() << std::endl;
  // std::cout << "imgTransform size: " << imgTransform.rows << "x" <<
  // imgTransform.cols << std::endl;

  // Apply the transformation
  cv::Size img_size(width, height);
  cv::Mat noPersp(img_size, inputImg.type());
  cv::warpPerspective(inputImg, noPersp, imgTransform, img_size);

  return noPersp;
}

/************************************************
 *               Fault finding
 ***********************************************/

void noise_removal(cv::Mat &XOR_img, int closure_iterations,
                   int ind_operation_iterations) {
  cv::Mat closure_kernel =
      cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
  cv::Mat XOR_eroded_img;
  cv::Mat XOR_dialated_img;

  // Apply closure via erossion and dilation
  cv::erode(XOR_img, XOR_eroded_img, closure_kernel, cv::Point(-1, -1),
            ind_operation_iterations);
  cv::dilate(XOR_eroded_img, XOR_dialated_img, closure_kernel,
             cv::Point(-1, -1), ind_operation_iterations);

  XOR_img = XOR_dialated_img;
}

std::vector<boxCorners> getBoundingBoxCorners(io::CSVReader<5> &csvData) {
  std::vector<boxCorners> corners;
  std::string comp_name;
  int pos_x, pos_y, size_x, size_y;

  while (csvData.read_row(comp_name, pos_x, pos_y, size_x, size_y)) {
    boxCorners temp_row;

    temp_row.comp_name = comp_name;
    temp_row.topLeft = cv::Point(pos_x, pos_y);
    temp_row.topRight = cv::Point(pos_x + size_x, pos_y);
    temp_row.bottomLeft = cv::Point(pos_x, pos_y + size_y);
    temp_row.bottomRight = cv::Point(pos_x + size_x, pos_y + size_y);

    corners.push_back(temp_row);
  }

  return corners;
}

std::vector<cv::Point> findLargestContour(cv::Mat inputImg, bool print) {
  // Detect the contours in the image
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(inputImg, contours, hierarchy, cv::RETR_TREE,
                   cv::CHAIN_APPROX_SIMPLE);

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
  cv::approxPolyDP(contours[largestContourIndex], approx,
                   0.01 * cv::arcLength(contours[largestContourIndex], true),
                   true);

  // Check if the polygon has 4 vertices and a sufficiently large area
  if (approx.size() == 4 && largestArea > 1000) {
    // Give the coordinates of the corners of the largest rectangle
    if (print) {
      std::cout << approx[0] << "\n";
      std::cout << approx[1] << "\n";
      std::cout << approx[2] << "\n";
      std::cout << approx[3] << "\n";
    }

    return approx;
  }

  std::cout << "No large rectangle found in the image." << std::endl;
  std::vector<cv::Point> emptyRet;
  return emptyRet;
}

cv::Mat getPCBmask(cv::Mat inputImg, std::vector<int> lowerLims,
                   std::vector<int> upperLims) {
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
  // display_img(mask, true);

  return mask;
}

std::vector<cv::Mat> createImgBoxes(cv::Mat inputImg,
                                    io::CSVReader<5> &compBoundBox) {
  std::vector<cv::Mat> boxImages;

  std::string comp_name;
  int topLeft_x, topLeft_y, size_x, size_y;

  while (
      compBoundBox.read_row(comp_name, topLeft_x, topLeft_y, size_x, size_y)) {
    cv::Mat tempClone = inputImg(cv::Range(topLeft_y, topLeft_y + size_y),
                                 cv::Range(topLeft_x, topLeft_x + size_x));

    // Convert to BGR to be able to use countNonZero pixels
    cv::Mat tempClone_RGB;
    cv::cvtColor(tempClone, tempClone_RGB, cv::COLOR_GRAY2BGR);
      
    boxImages.push_back(tempClone);
  }

  return boxImages;
}

std::vector<std::pair<std::string, int>>
verifyComponents(std::vector<cv::Mat> &compImgs,
                 io::CSVReader<2> &maxLighting) {
  // True: component found, False: component not found
  std::vector<std::pair<std::string, int>> results;

  std::string compName;
  float maxLitPixels; // Max accepted percentage of lit pixels
  int i = 0;        // Keep count of current component index

  // Verify by calculating average intensity of each binary image and comparing
  // to the gives max values
  while (maxLighting.read_row(compName, maxLitPixels)) {
    // Calculate average number of lit pixels
    int nonZeroPixels = cv::countNonZero(compImgs[i]);
    int totalPixels = compImgs[i].rows * compImgs[i].cols;
    double percentageLitPixels = static_cast<double>(nonZeroPixels) / totalPixels * 100;

    bool tempResult = percentageLitPixels <= maxLitPixels;
    std::pair<std::string, int> tempResultPair = {compName, tempResult};
    results.push_back(tempResultPair);
    i++;
  }

  return results;
}
