#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>


////////////////////////////////////////////////////////////////////////////////
///                             Preprocessing                                ///
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Apply Gaussian blur, convert to grayscale, apply
 * thresholding, image morphology, and convert to binary image.
 */
cv::Mat preprocessImg(cv::Mat inputImg);

/**
 * @brief MISSING DESCRIPTION
 */
cv::Mat correctPerspective(cv::Mat inputImg);


////////////////////////////////////////////////////////////////////////////////
///                             Fault finding                                ///
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Remove noise from image via closure operation (erosion + dilation)
 *
 * @param XOR_img
 * @param closure_iterations Number of times to run closure.
 * @param ind_operation_iterations 
 */
void noise_removal(cv::Mat &XOR_img, int closure_iterations=3, int ind_operation_iterations=3);


/**
 * @brief Filter specific color range and return resulting mask
 *
 * @param inputImg Image to process and generate mask.
 * @param lowerLims HSV color lower limits in the form [H, S, V].
 * @param upperLims HSV color upper limits in the form [H, S, V].
 */
cv::Mat colorFilterHSV(cv::Mat inputImg, std::vector<int> lowerLims, std::vector<int> upperLims);

