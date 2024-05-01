#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


////////////////////////////////////////////////////////////////////////////////
///                             Preprocessing                                ///
////////////////////////////////////////////////////////////////////////////////

// Preprocess image: apply Gaussian blur, convert to grayscale, apply
// thresholding, image morphology, and convert to binary image.
cv::Mat preprocessImg(cv::Mat inputImg);

// Correct perspective: get the perspective transform corresponding to 
// the image and warp the perspective for a flat PCB.
cv::Mat correctPerspective(cv::Mat inputImg);
