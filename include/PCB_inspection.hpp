#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/csv.hpp"

////////////////////////////////////////////////////////////////////////////////
///                             Preprocessing                                ///
////////////////////////////////////////////////////////////////////////////////

// Preprocess image: apply Gaussian blur, convert to grayscale, apply
// thresholding, image morphology, and convert to binary image.
cv::Mat preprocessImg(cv::Mat inputImg);

// Correct perspective: get the perspective transform corresponding to 
// the image and warp the perspective for a flat PCB.
cv::Mat correctPerspective(cv::Mat inputImg);

////////////////////////////////////////////////////////////////////////////////
///                             Fault finding                                ///
////////////////////////////////////////////////////////////////////////////////

// Resize to display, show two images to the screen
void display_img(cv::Mat &original_img, cv::Mat &preprocessed_img, bool resize, int width=2560, int height=1600);

// Remove noise from image via closure operation (erosion + dilation)
void noise_removal(cv::Mat &XOR_img, int closure_iterations=3, int ind_operation_iterations=3);


////////////////////////////////////////////////////////////////////////////////
///                           Auxiliary functions                            ///
////////////////////////////////////////////////////////////////////////////////

// Print to stdio components with their respective placement in mm
void printComponentPlace(io::CSVReader <3> comp_placement);

