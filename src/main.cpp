#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/PCB_inspection.hpp"
#include "../include/aux_functions.hpp"


int main(){
  // Import image and create gray copy
  cv::Mat ref_img_RGB = cv::imread("../imgs/best_conditions/ELYOS_PCB_0.jpg");

  // Filter with mask that only lets green through
  // Filter white - HSL 
  cv::Mat filteredImg;
  cv::Mat mask;
  std::vector<int> lowerLimits = {50, 12, 15};
  std::vector<int> upperLimits = {120, 225, 120};
  // filteredImg = colorFilterHSV(ref_img_RGB, lowerLimits, upperLimits);
  mask = getPCBmask(ref_img_RGB, lowerLimits, upperLimits);
  
  // Get corners for transformation
  std::vector<cv::Point> corners;
  corners = findLargestContour(mask);

  // Correct perspective 
  cv::Mat noPersp;
  noPersp = correctPerspective(mask, corners);

  // Display
  display_img(noPersp, true);
  // display_img(filteredImg, true);

  // Preprocess reference and evaluate image
  // cv::Mat final_ref_img = preprocessImg(ref_img_RGB);

  cv::destroyAllWindows();

  return EXIT_SUCCESS;		
}
