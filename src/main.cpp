#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/csv.hpp"
#include "../include/PCB_inspection.hpp"


int main(){
  // Read CSV file with componet positions
	io::CSVReader<3> comp_placement("../board/ELYOS_component_placement.csv");

  // Print components with their placement (in mm)
  // printComponentPlace(comp_placement);

  // Import image and create gray copy
  cv::Mat ref_img_RGB = cv::imread("../imgs/reference/ELYOS_PCB_0.jpg");
  cv::Mat eval_img_RGB = cv::imread("../imgs/evaluate/ELYOS_PCB_16.jpg");

  // Correct perspective differnce on both images
  cv::Mat no_persp_ref_img = correctPerspective(ref_img_RGB);
  cv::Mat no_persp_eval_img = correctPerspective(eval_img_RGB);

  // Preprocess reference and evaluate image
  cv::Mat final_ref_img = preprocessImg(ref_img_RGB);
  cv::Mat final_eval_img = preprocessImg(eval_img_RGB); 
  
  // Show original and preprocessed reference image 
  cv::imshow("Original reference image", ref_img_RGB);
  cv::waitKey(0);

  cv::imshow("Preprocessed reference image", final_ref_img);
  cv::waitKey(0);

  // cv::imshow("Perspecgive corrected reference image", no_persp_ref_img);
  // cv::waitKey(0);

  cv::destroyAllWindows();

  return EXIT_SUCCESS;		
}
