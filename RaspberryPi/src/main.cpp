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
 * This document contains the main implementation for the
 * developed AOI system for PCB. The system is setup to work with
 * a specific PCB board, and must be modified in order to support
 * others.
 */

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "../include/PCB_inspection.hpp"
#include "../include/aux_functions.hpp"

int main() {
  // Import image and create gray copy
  cv::Mat ref_img_RGB = cv::imread("../imgs/best_conditions/ELYOS_PCB_0.jpg");
  cv::Mat eval_img_RGB =
      cv::imread("../imgs/best_conditions/ELYOS_PCB_4_missing_comp2.jpg");

  // Detect PCB and create mask
  cv::Mat ref_mask;
  cv::Mat eval_mask;
  std::vector<int> lowerLimits = {50, 12, 15};
  std::vector<int> upperLimits = {120, 225, 120};

  ref_mask = getPCBmask(ref_img_RGB, lowerLimits, upperLimits);
  eval_mask = getPCBmask(eval_img_RGB, lowerLimits, upperLimits);

  // Get corners for transformation
  std::vector<cv::Point> ref_corners;
  ref_corners = findLargestContour(ref_mask);

  std::vector<cv::Point> eval_corners;
  eval_corners = findLargestContour(eval_mask);

  // Correct perspective on reference and evaluate image
  cv::Mat noPersp_ref, noPersp_eval;
  noPersp_ref = correctPerspective(ref_img_RGB, ref_corners);
  noPersp_eval = correctPerspective(eval_img_RGB, eval_corners);

  // Apply preprocessing to both images
  cv::Mat preprocessed_ref = preprocessImg(noPersp_ref);
  cv::Mat preprocessed_eval = preprocessImg(noPersp_eval);

  // XOR evaluate and reference image
  cv::Mat XOR_result;
  cv::bitwise_xor(preprocessed_ref, preprocessed_eval, XOR_result);
  noise_removal(XOR_result);

  // Read bounding boxes
  io::CSVReader<5> compBoundBoxes("../board/ELYOS_component_boxes.csv");
  // printCompBoundBoxes(compBoundBoxes);

  // Create bounding box images
  std::vector<cv::Mat> imgBoxes;
  imgBoxes = createImgBoxes(XOR_result, compBoundBoxes);

  // Display all image boxes
  cv::Mat boxCanvas;
  boxCanvas = makeCanvas(imgBoxes, 1600, 8);
  display_img(boxCanvas);

  // Read max lighting values for boxes
  io::CSVReader<2> maxLighitingVal("../board/ELYOS_component_maxLighting.csv");

  // Check if components are missing from their respective bounding boxes 
  std::vector<std::pair<std::string, int>> compSearchResults;
  compSearchResults = verifyComponents(imgBoxes, maxLighitingVal);

  // Display reference and evaluate iamge, XOR result, and results 
  // display_img(noPersp_ref);
  // display_img(noPersp_eval);
  // display_img(XOR_result);
  // printResults(compSearchResults);
 
  // Save results to CSV file  
  saveResultsCSV(compSearchResults);

  cv::destroyAllWindows();

  return EXIT_SUCCESS;
}