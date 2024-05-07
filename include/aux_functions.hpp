#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "csv.hpp"


////////////////////////////////////////////////////////////////////////////////
///                           Auxiliary functions                            ///
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Fill holes of PCB outline, creating a solid rectangle.
 */
void repairMask(cv::Mat &inputImg);

void closeMask(cv::Mat &inputImg);

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
