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
 * This header file specifies functions available as auxiliaries
 * to the ones specified in PCB_inspection.hpp
 */

#ifndef AUX_FUNCTIONS_H
#define AUX_FUNCTIONS_H

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "csv.hpp"

/**
 * @brief Comparator function for std::vector<cv::Point>
 */
bool comparePoints(const cv::Point &a, const cv::Point &b);

/**
 * @brief Save the result pairs to a CSV with columns ComponentName,
 * IspectionResult.
 */
void saveResultsCSV(std::vector<std::pair<std::string, int>> &results,
                    std::string filename = "../results.csv");

/************************************************
 *                   PCB mask
 ***********************************************/

/**
 * @brief Fill the holes that remain in binary representation of mask.
 */
void fillPCBholes(cv::Mat &inputImg);

/**
 * @brief Fill holes of PCB outline, starting from the center, creating a solid
 * rectangle.
 */
void floodMask(cv::Mat &inputImg, int width = 2560, int height = 1600);

/**
 * @brief Close holes in mask by morphological closing operation.
 *
 * @param inputImg Object to which to apply closing operations.
 * @param repetitions Number of times to repeat closing operation
 * (default = 1)
 */
void closeMask(cv::Mat &inputImg, int repetitions = 1);

/************************************************
 *            Displaying and printing
 ***********************************************/

/**
 * @brief Display the results of the processing to terminal.
 */
void printResults(
    std::vector<std::pair<std::string, int>> &componentSearchResults);

/**
 * @brief Resize image and display to screen.
 *
 * @param original_img Image to resize and display
 * @param resize Whether to resize or not.
 * @param width Width of image to display, in pixels.
 * @param height Height of image to display, in pixels.
 */
void display_img(cv::Mat &original_img, bool resize = true, int width = 2560,
                 int height = 1600);

/**
 * @brief Resize images and display to two separate windows.
 *
 * @param original_img Image to resize and display
 * @param preprocessed_img Preprocessed image to resize and display.
 * @param resize Whether to resize or not.
 * @param width Width of image to display, in pixels.
 * @param height Height of image to display, in pixels.
 */
void display_imgs(cv::Mat &original_img, cv::Mat &preprocessed_img, bool resize,
                  int width = 2560, int height = 1600);

/**
 * @brief Print to stdio components with their respective placement in mm
 *
 * @param comp_placement CSVReader object containing information from CSV to
 * print.
 */
void printComponentPlace(io::CSVReader<3> comp_placement);

/**
 * @brief Print to stdio components with their bounding boxes, with the
 * data of their bounding boxes. The data is (topLeft_x, topLeft_y, size_x,
 * size_y).
 *
 * @param corners CSVReader object containing information from CSV to
 * print.
 */
void printCompBoundBoxes(io::CSVReader<5> &corners);

/**
 * @brief Display all 65 component images to a large grid
 */
void displayAllCompBoxes(std::vector<cv::Mat> compImgs);

/**
 * @brief makeCanvas Makes composite image from the given images
 *
 * Credit to vinvinod from StackOverflow for the implementation of this
 * function. See:
 * https://stackoverflow.com/questions/5089927/show-multiple-2-3-4-images-in-the-same-window-in-opencv
 *
 * @param vecMat Vector of Images.
 * @param windowHeight The height of the new composite image to be formed.
 * @param nRows Number of rows of images. (Number of columns will be calculated
 *              depending on the value of total number of images).
 * @return new composite image.
 */
cv::Mat makeCanvas(std::vector<cv::Mat> &vecMat, int windowHeight, int nRows);

#endif // AUX_FUNCTIONS_H
