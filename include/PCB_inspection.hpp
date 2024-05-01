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

// Find which of the provided contours corresponds to the PCB outline
int getMaxAreaContourId(std::vector<std::vector<cv::Point>> contours);

// Receive RGB image as input, get position of the edges of the PCB board as output
std::vector<cv::Point> findPCBcountours(cv::Mat inputImg); 


////////////////////////////////////////////////////////////////////////////////
///                           Auxiliary functions                            ///
////////////////////////////////////////////////////////////////////////////////

void printComponentPlace(io::CSVReader <3> comp_placement);

