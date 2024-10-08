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

#define WHITE_BACKGROUND_LOWER_LIMS                                            \
  std::vector<int> { 50, 12, 15 }
#define WHITE_BACKGROUND_UPPER_LIMS                                            \
  std::vector<int> { 120, 225, 120 }

/************************************************
 *                 Main pipeline
 ***********************************************/

std::vector<std::pair<std::string, int>>
automaticEvaluation(std::string boardType, std::string filename,
                    bool displayResults) {
  std::string refImagePath, compBoxesPath, compMaxLightPath;
  std::string evalImagePath = "../imgs/ELYOS/eval/" + filename;
  if (boardType == "ELYOS") {
    refImagePath = "../imgs/ELYOS/ref/ELYOS_REF.jpg";
    compBoxesPath = "../board/ELYOS/ELYOS_component_boxes.csv";
    compMaxLightPath = "../board/ELYOS/ELYOS_component_maxLighting.csv";
  }
  // TODO: Add more PCB options
  else {
    throw NotifyError("Error occurred during evaluation. Selected board type "
                      "does not exist.");
  }

  if (searchFile(refImagePath))
    throw NotifyError(
        "Error occurred during evaluation. Could not find reference image.");
  if (searchFile(evalImagePath))
    throw NotifyError(
        "Error occurred during evaluation. Could not find evaluate image.");
  if (searchFile(compBoxesPath))
    throw NotifyError("Error occurred during evaluation. Could not find "
                      "component boxes CSV.");
  if (searchFile(compMaxLightPath))
    throw NotifyError("Error occurred during evaluation. Could not find "
                      "component max light CSV.");

  cv::Mat ref_img_RGB;
  cv::Mat eval_img_RGB;
  ref_img_RGB = cv::imread(refImagePath);
  eval_img_RGB = cv::imread(evalImagePath);

  cv::Mat ref_mask;
  cv::Mat eval_mask;
  ref_mask = getPCBmask(ref_img_RGB, WHITE_BACKGROUND_LOWER_LIMS,
                        WHITE_BACKGROUND_UPPER_LIMS);
  eval_mask = getPCBmask(eval_img_RGB, WHITE_BACKGROUND_LOWER_LIMS,
                         WHITE_BACKGROUND_UPPER_LIMS);

  std::vector<cv::Point> ref_corners;
  ref_corners = findLargestContour(ref_mask);

  std::vector<cv::Point> eval_corners;
  eval_corners = findLargestContour(eval_mask);

  cv::Mat noPersp_ref, noPersp_eval;
  noPersp_ref = correctPerspective(ref_img_RGB, ref_corners);
  noPersp_eval = correctPerspective(eval_img_RGB, eval_corners);

  cv::Mat preprocessed_ref = preprocessImg(noPersp_ref);
  cv::Mat preprocessed_eval = preprocessImg(noPersp_eval);

  cv::Mat XOR_result;
  cv::bitwise_xor(preprocessed_ref, preprocessed_eval, XOR_result);
  noise_removal(XOR_result);

  io::CSVReader<5> compBoundBoxes(compBoxesPath);

  std::vector<cv::Mat> imgBoxes;
  imgBoxes = createImgBoxes(XOR_result, compBoundBoxes);

  cv::Mat boxCanvas;
  boxCanvas = makeCanvas(imgBoxes, 1600, 8);

  io::CSVReader<2> maxLighitingVal(compMaxLightPath);

  std::vector<std::pair<std::string, int>> compSearchResults;
  compSearchResults = verifyComponents(imgBoxes, maxLighitingVal);

  if (displayResults) {
    display_img(boxCanvas);
    display_img(noPersp_ref);
    display_img(noPersp_eval);
    display_img(XOR_result);
    printResults(compSearchResults);
  }

  std::string resultsPath = "../results/" + boardType + ".csv";
  saveResultsCSV(compSearchResults, resultsPath);

  cv::destroyAllWindows();

  return compSearchResults;
}

/************************************************
 *               Operation modes
 ***********************************************/

std::string rowMode(std::string boardType) {
  std::vector<std::string> pictureNames;
  pictureNames = takeRowPictures(boardType);

  std::string pictureName(takePicture(boardType));

  std::vector<std::vector<std::pair<std::string, int>>> results;
  for (auto const &name : pictureNames) {
    results.push_back(automaticEvaluation(boardType, name, false));
  }

  std::string completionMsg("Done");
  if (checkResultsPCB(results[0]))
    completionMsg = "_E";
  else
    completionMsg = "_G";
  if (checkResultsPCB(results[1]))
    completionMsg = completionMsg + "_E";
  else
    completionMsg = completionMsg + "_G";
  if (checkResultsPCB(results[2]))
    completionMsg = completionMsg + "_E";
  else
    completionMsg = completionMsg + "_G";

  return completionMsg;
}

std::string oneMode(std::string boardType) {
  std::string pictureName(takePicture(boardType));

  std::vector<std::pair<std::string, int>> result(automaticEvaluation(boardType, pictureName, false));

  std::string completionMsg = "Done";
  if (checkResultsPCB(result))
    completionMsg = "_E";
  else
    completionMsg = "_G";

  return completionMsg + "_X_X";
}

/************************************************
 *                Preprocessing
 ***********************************************/

cv::Mat preprocessImg(cv::Mat inputImg) {
  cv::Mat bluredImg;
  cv::GaussianBlur(inputImg, bluredImg, cv::Size(3, 3), 0, cv::BORDER_DEFAULT);

  cv::Mat bluredGrayImg;
  cv::cvtColor(bluredImg, bluredGrayImg, cv::COLOR_BGR2GRAY);

  cv::Mat threshBluredGrayImg;
  cv::threshold(bluredGrayImg, threshBluredGrayImg, 127, 255,
                cv::THRESH_BINARY_INV);

  cv::Mat finalImg;
  cv::morphologyEx(threshBluredGrayImg, finalImg, cv::MORPH_CLOSE,
                   cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

  return finalImg;
}

cv::Mat correctPerspective(cv::Mat inputImg, std::vector<cv::Point> corners,
                           int width, int height) {
  std::sort(corners.begin(), corners.end(), comparePoints);

  std::vector<cv::Point2f> srcCorners = {static_cast<cv::Point2f>(corners[0]),
                                         static_cast<cv::Point2f>(corners[1]),
                                         static_cast<cv::Point2f>(corners[2]),
                                         static_cast<cv::Point2f>(corners[3])};

  std::vector<cv::Point2f> dstCorners = {
      cv::Point2f(0, 0), cv::Point2f(width - 1, 0),
      cv::Point2f(width - 1, height - 1), cv::Point2f(0, height - 1)};

  cv::Mat imgTransform;
  imgTransform = cv::getPerspectiveTransform(srcCorners, dstCorners);

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

std::pair<int, int> findLargestContourIdx(const std::vector<std::vector<cv::Point>> & contours){
  int largestArea = 0;
  int largestContourIndex = -1;
  for (int i = 0; i < contours.size(); i++) {
    double area = cv::contourArea(contours[i]);
    if (area > largestArea) {
      largestArea = area;
      largestContourIndex = i;
    }
  }

  return std::make_pair(largestArea, largestContourIndex);
}

std::vector<cv::Point> findLargestContour(cv::Mat inputImg, bool print) {
  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(inputImg, contours, hierarchy, cv::RETR_TREE,
                   cv::CHAIN_APPROX_SIMPLE);
  
  auto [largestArea,largestContourIndex] = findLargestContourIdx(contours);

  std::vector<cv::Point> approx;
  cv::approxPolyDP(contours[largestContourIndex], approx,
                   0.01 * cv::arcLength(contours[largestContourIndex], true),
                   true);

  if (print && approx.size() == 4 && largestArea > 1000) {
    std::cout << approx[0] << "\n";
    std::cout << approx[1] << "\n";
    std::cout << approx[2] << "\n";
    std::cout << approx[3] << "\n";

    return approx;
  }

  std::cout << "No large rectangle found in the image." << std::endl;
  return std::vector<cv::Point> {};
}

cv::Mat getPCBmask(cv::Mat inputImg, std::vector<int> lowerLims,
                   std::vector<int> upperLims) {
  cv::Mat hsvImg;
  cv::cvtColor(inputImg, hsvImg, cv::COLOR_BGR2HSV);

  cv::Scalar lower_green(lowerLims[0], lowerLims[1], lowerLims[2]);
  cv::Scalar upper_green(upperLims[0], upperLims[1], upperLims[2]);

  cv::Mat mask;
  cv::inRange(inputImg, lower_green, upper_green, mask);
  closeMask(mask, 7);

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
  float maxLitPixels;
  int i = 0;

  while (maxLighting.read_row(compName, maxLitPixels)) {
    // Average number of lit pixels
    int nonZeroPixels = cv::countNonZero(compImgs[i]);
    int totalPixels = compImgs[i].rows * compImgs[i].cols;
    double percentageLitPixels =
        static_cast<double>(nonZeroPixels) / totalPixels * 100;

    bool tempResult = percentageLitPixels <= maxLitPixels;
    std::pair<std::string, int> tempResultPair = {compName, tempResult};
    results.push_back(tempResultPair);
    i++;
  }

  return results;
}

/************************************************
 *              Result verification
 ***********************************************/

bool checkResultsPCB(std::vector<std::pair<std::string, int>> results) {
  for (const auto &comp : results) {
    if (!comp.second)
      return true;
  }
  return false;
}
