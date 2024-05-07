#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/csv.hpp"
#include "../include/PCB_inspection.hpp"
#include "../include/aux_functions.hpp"


int main(){
  // Read CSV file with componet positions
  io::CSVReader<3> comp_placement("../board/ELYOS_component_placement.csv");

  // Print components with their placement (in mm)
  // printComponentPlace(comp_placement);

  // Import image and create gray copy
  cv::Mat ref_img_RGB = cv::imread("../imgs/same_position/ELYOS_PCB_reference.jpg");
  cv::Mat eval_img_RGB = cv::imread("../imgs/same_position/ELYOS_PCB_evaluate.jpg");

  // Correct perspective differnce on both images
  cv::Mat no_persp_ref_img = correctPerspective(ref_img_RGB);
  cv::Mat no_persp_eval_img = correctPerspective(eval_img_RGB);

  // Preprocess reference and evaluate image
  cv::Mat final_ref_img = preprocessImg(ref_img_RGB);
  cv::Mat final_eval_img = preprocessImg(eval_img_RGB); 
  
  // Reference image - show original and preprocessed reference image 
  display_imgs(ref_img_RGB, final_ref_img, true);

  // Evaluate image - show original and preprocessed reference image 
  display_imgs(eval_img_RGB, final_eval_img, true);

  // XOR evaluate and reference image
  cv::Mat XOR_result;
  cv::bitwise_xor(final_ref_img, final_eval_img, XOR_result);
  
  // Clear XOR result from noise - closure
  noise_removal(XOR_result);

  // Show XOR result
  cv::Mat display_XOR_result;
  cv::resize(XOR_result, display_XOR_result, cv::Size(2560, 1600), 0, 0, cv::INTER_LINEAR);
  cv::imshow("XOR result", display_XOR_result);
  cv::waitKey(0);

  cv::destroyAllWindows();

  return EXIT_SUCCESS;		
}
