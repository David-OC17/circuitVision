#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>


////////////////////////////////////////////////////////////////////////////////
///                             Preprocessing                                ///
////////////////////////////////////////////////////////////////////////////////

// Preprocess image: apply Gaussian blur, convert to grayscale, apply
// thresholding, image morphology operation, and convert to binary image.
cv::Mat preprocessImg(cv::Mat inputImg){
// Apply Gaussian blur
cv::Mat bluredImg;
cv::GaussianBlur(inputImg, bluredImg, cv::Size(3, 3), 0, cv::BORDER_DEFAULT);

// Convert to grayscale
cv::Mat bluredGrayImg;
cv::cvtColor(bluredImg, bluredGrayImg, cv::COLOR_BGR2GRAY);

// Apply thresholding
cv::Mat threshBluredGrayImg;
cv::threshold(bluredGrayImg, threshBluredGrayImg, 127, 255, cv::THRESH_BINARY_INV);

// Apply closign morphological operation 
cv::Mat finalImg;
cv::morphologyEx(threshBluredGrayImg, finalImg, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));

return finalImg;
}

// Correct perspective: get the perspective transform corresponding to 
// the image and warp the perspective for a flat PCB.
cv::Mat correctPerspective(cv::Mat inputImg){
  cv::Mat perspectiveTransform;
  cv::getPerspectiveTransform(inputImg, perspectiveTransform); 

  cv::Mat finalImg;
  cv::Size img_size = inputImg.size();
  cv::warpPerspective(inputImg, finalImg, perspectiveTransform, img_size);

  return finalImg;
}
