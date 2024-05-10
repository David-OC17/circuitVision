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
 * This header file specifies the functions available for the PCB
 * AOI system, including a preprocessing and a fault finding
 * section.
 */

#ifndef PCB_INSPECTION_H
#define PCB_INSPECTION_H

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include <string>
#include <vector>

#include "csv.hpp"

/************************************************
 *              Preprocessing
 ***********************************************/

/**
 * @brief Apply Gaussian blur, convert to grayscale, apply
 * thresholding, image morphology, and convert to binary image.
 */
cv::Mat preprocessImg(cv::Mat inputImg);

/**
 * @brief Correct image perspective by calculating its transform and applying
 * it to the image.
 *
 * @param inputImg Image to correct perspective on.
 * @param corners Vector of points to the current corners of the figure to move.
 * @param width Size in x of the screen.
 * @param height Size in y of the screen.
 */
cv::Mat correctPerspective(cv::Mat inputImg, std::vector<cv::Point> corners,
                           int width = 2560, int height = 1600);

/************************************************
 *               Fault finding
 ***********************************************/

/**
 * @brief A structure for a row of a corner for bounding boxes.
 */
struct boxCorners {
  /** @name Component name */
  /*@{*/
  std::string comp_name; /**< The component name */
  /*@*/

  /** @name Corners */
  /*@{*/
  cv::Point topLeft;     /**< The top left corner */
  cv::Point topRight;    /**< The top right corner */
  cv::Point bottomLeft;  /**< The bottom left corner */
  cv::Point bottomRight; /**< The bottom right corner */
  /*@*/
};

/**
 * @brief Calculate corners corresponding to the box given by
 * the object io::CSVReader, with rows (int, int, int, int),
 * where values represent (comp_name, pos_x, pox_y, size_x, size_y).
 *
 * @return Gives a std::vector<boxCorners> back, which is a vector of
 * structs, with values std::string comp_name, and cv::Point values
 * for (TL, TR, BL, BR), which represent the corners
 * (topLeft, topRight, bottomLeft, bottomRight)
 */
std::vector<boxCorners> getBoundingBoxCorners(io::CSVReader<5> &csvData);

/**
 * @brief Remove noise from image via closure operation (erosion + dilation)
 *
 * @param XOR_img
 * @param closure_iterations Number of times to run closure.
 * @param ind_operation_iterations
 */
void noise_removal(cv::Mat &XOR_img, int closure_iterations = 3,
                   int ind_operation_iterations = 3);

/**
 * @brief Find largest rectangle in the inputImg.
 *
 * @param inputImg Binary image in which to find largest rectangle points.
 * @return A vector of the 4 resulting cv::Point.
 */
std::vector<cv::Point> findLargestContour(cv::Mat inputImg);

/**
 * @brief Filter specific color range and return resulting mask
 *
 * @param inputImg Image to process and generate mask.
 * @param lowerLims HSV color lower limits in the form [H, S, V].
 * @param upperLims HSV color upper limits in the form [H, S, V].
 */
cv::Mat getPCBmask(cv::Mat inputImg, std::vector<int> lowerLims,
                   std::vector<int> upperLims);

<<<<<<< Updated upstream
/**
 * @brief Filter specific color range and return masked original image.
 *
 * @param inputImg Image to process and generate mask.
 * @param lowerLims HSV color lower limits in the form [H, S, V].
 * @param upperLims HSV color upper limits in the form [H, S, V].
 */
cv::Mat colorFilterHSV(cv::Mat inputImg, std::vector<int> lowerLims,
                       std::vector<int> upperLims);


=======
>>>>>>> Stashed changes
#endif // PCB_INSPECTION_H
