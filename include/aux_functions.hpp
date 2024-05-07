#ifndef AUX_FUNCTIONS_H
#define AUX_FUNCTIONS_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "csv.hpp"


////////////////////////////////////////////////////////////////////////////////
///                           Auxiliary functions                            ///
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Fill holes of PCB outline, starting from the center, creating a solid rectangle.
 */
void floodMask(cv::Mat &inputImg, int width=2560, int height=1600);

/**
 * @brief Close holes in mask by morphological closing operation.
 *
 * @param inputImg Object to which to apply closing operations.
 * @param repetitions Number of times to repeat closing operation
 * (default = 1)
 */
void closeMask(cv::Mat &inputImg, int repetitions=1);

/**
 * @brief Resize image and display to screen.
 *
 * @param original_img Image to resize and display
 * @param resize Wether to resize or not.
 * @param width Width of image to display, in pixels.
 * @param height Height of image to display, in pixels.
 */
void display_img(cv::Mat &original_img, bool resize, int width=2560, int height=1600);

/**
 * @brief Resize images and display to two separate windows.
 *
 * @param original_img Image to resize and display
 * @param preprocessed_img Preprocessed image to resize and display.
 * @param resize Wether to resize or not.
 * @param width Width of image to display, in pixels.
 * @param height Height of image to display, in pixels.
 */
void display_imgs(cv::Mat &original_img, cv::Mat &preprocessed_img, bool resize, int width=2560, int height=1600);

/**
 * @brief Print to stdio components with their respective placement in mm
 *
 * @param comp_placement CSVReader object containing information from CSV to print.
 */
void printComponentPlace(io::CSVReader <3> comp_placement);

#endif // AUX_FUNCTIONS_H
