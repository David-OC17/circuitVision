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
 * This document provides the implementation of the testing functions
 * provided in the aux_functions_test.hpp header. Refer to that file
 * for a more detailed description about the use of the provided functions.
 */

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "../include/aux_functions_test.hpp"

/************************************************
 *                Picture taking
 ***********************************************/

std::vector<std::string> takeRowPictures_test(std::string boardType) {
  std::vector<std::string> imgPaths;
  std::vector<std::string> filenames;
  std::string imgExtension = ".jpg";

  for (int i = 0; i < 3; i++) {
    filenames.push_back(boardType + "_" + std::to_string(i) + imgExtension);
  }
  for (int i = 0; i < 3; i++) {
    imgPaths.push_back("../imgs/ELYOS/eval/" + filenames[i]);
  }

  // Take pictures with OpenCV
  cv::VideoCapture camera(0);
  std::vector<cv::Mat> frames;
  for (int i = 0; i < 3; i++) {
    camera >> frames[i];
    cv::imwrite(imgPaths[i], frames[i]);
  }
  camera.release();

  return filenames;
}

std::string takePicture_test(std::string boardType) {
  std::string imgPath;
  std::string imgExtension = ".jpg";
  std::string filename = boardType + imgExtension;
  imgPath = "../imgs/ELYOS/eval/" + filename;

  // Take picture with OpenCV
  cv::VideoCapture camera(0);
  cv::Mat frame;
  camera >> frame;
  cv::imwrite(imgPath, frame);
  camera.release();

  return filename;
}
