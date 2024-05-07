#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/PCB_inspection.hpp"


int main(){
  // Import image and create gray copy
  cv::Mat ref_img_RGB = cv::imread("../imgs/same_position/ELYOS_PCB_reference_whiteBackground_noRefle.jpg");

  // Filter with mask that only lets green through
  // Filter white - HSL 
  cv::Mat filteredImg;
  std::vector<int> lowerLimits = {50, 12, 15};
  std::vector<int> upperLimits = {120, 225, 120};
  filteredImg = colorFilterHSV(ref_img_RGB, lowerLimits, upperLimits);

  // Display
  display_img(ref_img_RGB, true);
  display_img(filteredImg, true);

  // Preprocess reference and evaluate image
  // cv::Mat final_ref_img = preprocessImg(ref_img_RGB);

  cv::destroyAllWindows();

  return EXIT_SUCCESS;		
}
