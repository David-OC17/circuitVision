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
 * This document provides the implementation of auxiliary functions
 * to the main AOI PCB inspection system.
 */

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <utility>

#include "../include/aux_functions.hpp"
#include "../include/common.hpp"
#include "../include/csv.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#define NUM_IMAGES_CANVAS 65
#define NUM_ROWS_CANVAS 9 
#define NUM_COLS_CANVAS 8
#define SQUARE_SIZE_CANVAS 100

/************************************************
 *              Other aux functions
 ***********************************************/

bool comparePoints(const cv::Point &a, const cv::Point &b) {
  if (a.x == b.x) {
    return a.y < b.y;
  }
  return a.x < b.x;
}

void saveResultsCSV(std::vector<std::pair<std::string, int>> &results,
                    std::string filename) {
  std::ofstream file(filename);

  file << "ComponentName,InspectionResult" << std::endl;

  for (const auto &pair : results) {
    file << pair.first << "," << pair.second << std::endl;
  }

  file.close();
}

int searchFile(const std::string &filePath) {
  if (!std::filesystem::exists(filePath)) {
    return 1;
  }
  return 0;
}

void searchCommand(std::string command){
  std::string checkCommand = "which " + command;

  if(system(checkCommand.c_str())) throw NotifyError("Command " + command + " not found in command line.");
}

int findLastImageNumber(std::string boardType) {
  std::string imgsDirectory;
  int highest_x = 0;

  if (boardType == "ELYOS")
    imgsDirectory = "../imgs/ELYOS/eval/";
  // TODO: Add more board to the options
  else
    throw NotifyError("Error occurred while finding last index for taken "
                      "images. Board type not valid.");

  std::regex imgPattern("ELYOS_([0-9]+)\\.jpg");

  for (const auto &entry : std::filesystem::directory_iterator(imgsDirectory)) {
    std::smatch match;
    std::string filename = entry.path().string();
    if (std::regex_search(filename, match, imgPattern)) {
      int number = std::stoi(match[1]);
      if (number > highest_x) {
        highest_x = number;
      }
    }
  }

  return highest_x;
}

/************************************************
 *                Picture taking
 ***********************************************/

std::vector<std::string> takeRowPictures(std::string boardType, const std::string path = "../imgs/ELYOS/eval/",
                                         const std::string imgExtension = ".jpg") {
  std::vector<std::string> imgPaths;
  std::vector<std::string> filenames;
  int imgNumber = findLastImageNumber(boardType);

  for (int i = 0; i < 3; i++) {
    imgNumber++;
    filenames.push_back(boardType + "_" +
                        std::to_string(imgNumber) + imgExtension);
  }
  for (int i = 0; i < 3; i++) {
    imgPaths.push_back(path + filenames[i]);
  }

  std::cout << filenames[0] << std::endl;
  std::cout << filenames[1] << std::endl;
  std::cout << filenames[2] << std::endl;

  searchCommand("libcamera-still");
  std::string cameraComamnd = "libcamera-still --immediate -o ";

  const char *commandPath_char;
  std::string commandPath;
  int returnCode;
  for (const auto &path : imgPaths) {
    commandPath = cameraComamnd + path;
    commandPath_char = commandPath.c_str();
    returnCode = system(commandPath_char);
  }

  return filenames;
}

std::string takePicture(std::string boardType, const std::string path = "../imgs/ELYOS/eval/",
                        const std::string imgExtension = ".jpg") {
  std::string imgPath;
  std::string imgNumber = std::to_string(findLastImageNumber(boardType));
  std::string filename = boardType + "_" + imgNumber + imgExtension;
  imgPath = path + filename;

  std::cout << filename << std::endl;

  searchCommand("libcamera-still");
  std::string cameraComamnd = "libcamera-still --immediate -o ";

  const char *commandPath_char;
  std::string commandPath = cameraComamnd + imgPath;
  int returnCode;
  commandPath_char = commandPath.c_str();
  returnCode = system(commandPath_char);

  return filename;
}

/************************************************
 *                   PCB mask
 ***********************************************/

void printResults(
    std::vector<std::pair<std::string, int>> &componentSearchResults) {
  for (int i = 0; i < componentSearchResults.size(); i++) {
    std::cout << componentSearchResults[i].first << " "
              << componentSearchResults[i].second << "\n";
  }
}

void fillPCBholes(cv::Mat &inputImg) {
  cv::Mat des = 255 - inputImg;

  std::vector<std::vector<cv::Point>> contours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(des, contours, hierarchy, cv::RETR_CCOMP,
                   cv::CHAIN_APPROX_SIMPLE);

  for (int i = 0; i < contours.size(); i++) {
    cv::drawContours(des, contours, i, cv::Scalar(255), -1);
  }

  inputImg = des;
}

void floodMask(cv::Mat &inputImg, int width, int height) {
  cv::Point center(width / 2, height / 2);

  cv::Mat im_floodfill = inputImg.clone();
  cv::floodFill(im_floodfill, center, cv::Scalar(255));

  cv::Mat im_floodfill_inv;
  cv::bitwise_not(im_floodfill, im_floodfill_inv);
}

void closeMask(cv::Mat &inputImg, int repetitions) {
  int morph_size = 25;

  cv::Mat element = getStructuringElement(
      cv::MORPH_RECT, cv::Size(2 * morph_size + 1, 2 * morph_size + 1),
      cv::Point(morph_size, morph_size));

  cv::Mat output;

  while (repetitions) {
    cv::morphologyEx(inputImg, output, cv::MORPH_CLOSE, element,
                     cv::Point(-1, -1), 2);

    repetitions--;
    inputImg = output;
  }
}

/************************************************
 *            Displaying and printing
 ***********************************************/

void display_img(cv::Mat &original_img, bool resize, int width, int height) {
  if (resize) {
    cv::Mat display_original_img;
    cv::resize(original_img, display_original_img, cv::Size(width, height), 0,
               0, cv::INTER_LINEAR);

    cv::imshow("Original image", display_original_img);
    cv::waitKey(0);

  } else {
    cv::imshow("Original image", original_img);
    cv::waitKey(0);
  }
}

void display_imgs(cv::Mat &original_img, cv::Mat &preprocessed_img, bool resize,
                  int width, int height) {
  if (resize) {
    cv::Mat display_original_img;
    cv::resize(original_img, display_original_img, cv::Size(width, height), 0,
               0, cv::INTER_LINEAR);

    cv::Mat display_preprocessed_img;
    cv::resize(preprocessed_img, display_preprocessed_img,
               cv::Size(width, height), 0, 0, cv::INTER_LINEAR);

    cv::imshow("Original image", display_original_img);
    cv::waitKey(0);
    cv::imshow("Preprocessed image", display_preprocessed_img);
    cv::waitKey(0);

  } else {
    cv::imshow("Original image", original_img);
    cv::waitKey(0);
    cv::imshow("Preprocessed image", preprocessed_img);
    cv::waitKey(0);
  }
}

void printComponentPlace(io::CSVReader<3> comp_placement) {
  std::string comp_name;
  float pos_x;
  float pos_y;

  while (comp_placement.read_row(comp_name, pos_x, pos_y)) {
    std::cout << "Component name: " << comp_name << "\n";
    std::cout << "X position: " << pos_x << "\n";
    std::cout << "Y position: " << pos_y << "\n\n";
  }
}

void printCompBoundBoxes(io::CSVReader<5> &compBoundBox) {
  std::string comp_name;
  int topLeft_x, topLeft_y, size_x, size_y;

  while (
      compBoundBox.read_row(comp_name, topLeft_x, topLeft_y, size_x, size_y)) {
    std::cout << "Component name: " << comp_name << "\n";
    std::cout << "Top left corner: " << topLeft_x << "\n";
    std::cout << "Top right corner: " << topLeft_y << "\n";
    std::cout << "X box size: " << size_x << "\n";
    std::cout << "Y box size: " << size_y << "\n";
  }
}

void displayAllCompBoxes(std::vector<cv::Mat> compImgs) {
  int numImages = NUM_IMAGES_CANVAS; 
  int numRows = NUM_ROWS_CANVAS;
  int numCols = NUM_COLS_CANVAS;
  int squareSize = SQUARE_SIZE_CANVAS;

  const cv::Scalar whiteBackground(255, 255, 255);
  cv::Mat largeImage(numRows * squareSize, numCols * squareSize, CV_8UC3,
                     whiteBackground);

  int count = 0;
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      if (count < numImages) {
        cv::Mat smallImage = compImgs[count];
        resize(smallImage, smallImage,
               cv::Size(squareSize, squareSize));

        cv::Rect regionOfInter(j * squareSize, i * squareSize, squareSize,
                     squareSize);
        smallImage.copyTo(
            largeImage(regionOfInter));
        count++;
      }
    }
  }

  cv::imshow("Grid of Images", largeImage);
  cv::waitKey(0);
}

cv::Mat makeCanvas(std::vector<cv::Mat> &vecMat, int windowHeight, int nRows) {
  int N = vecMat.size();
  nRows = nRows > N ? N : nRows;
  const int edgeThickness = 10;
  const int imagesPerRow = ceil(double(N) / nRows);
  int resizeHeight =
      floor(2.0 *
            ((floor(double(windowHeight - edgeThickness) / nRows)) / 2.0)) -
      edgeThickness;
  int maxRowLength = 0;

  std::vector<int> resizeWidth;
  for (int i = 0; i < N;) {
    int thisRowLen = 0;
    for (int k = 0; k < imagesPerRow; k++) {
      double aspectRatio = double(vecMat[i].cols) / vecMat[i].rows;
      int temp = int(ceil(resizeHeight * aspectRatio));
      resizeWidth.push_back(temp);
      thisRowLen += temp;
      if (++i == N)
        break;
    }

    if ((thisRowLen + edgeThickness * (imagesPerRow + 1)) > maxRowLength) {
      maxRowLength = thisRowLen + edgeThickness * (imagesPerRow + 1);
    }
  }

  int windowWidth = maxRowLength;
  cv::Scalar backgroundColor(204, 52, 244);
  cv::Mat canvasImage(windowHeight, windowWidth, CV_8UC3, backgroundColor);

  for (int k = 0, i = 0; i < nRows; i++) {
    int y = i * resizeHeight + (i + 1) * edgeThickness;
    int x_end = edgeThickness;

    for (int j = 0; j < imagesPerRow && k < N; k++, j++) {
      int x = x_end;
      cv::Rect roi(x, y, resizeWidth[k], resizeHeight);
      cv::Size s = canvasImage(roi).size();
      cv::Mat target_ROI(s, CV_8UC3);
      if (vecMat[k].channels() != canvasImage.channels()) {
        if (vecMat[k].channels() == 1) {
          cv::cvtColor(vecMat[k], target_ROI, cv::COLOR_GRAY2BGR);
        }
      } else {
        vecMat[k].copyTo(target_ROI);
      }
      cv::resize(target_ROI, target_ROI, s);
      if (target_ROI.type() != canvasImage.type()) {
        target_ROI.convertTo(target_ROI, canvasImage.type());
      }
      target_ROI.copyTo(canvasImage(roi));
      x_end += resizeWidth[k] + edgeThickness;
    }
  }
  return canvasImage;
}
