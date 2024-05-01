#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/csv.hpp"
#include "../include/PCB_inspection.hpp"


int main(){
// Read CSV file with componet positions
	io::CSVReader<3> comp_placement("../board/ELYOS_component_placement.csv");
	std::string comp_name; float x_pos; float y_pos;

	while(comp_placement.read_row(comp_name, x_pos, y_pos)){
			std::cout << "Component name: " << comp_name << "\n";
			std::cout << "X position: " << x_pos << "\n";
			std::cout << "Y position: " << y_pos << "\n\n";
	}

  // Import image and create gray copy
  cv::Mat ref_img_RGB = cv::imread("../imgs/reference/ELYOS_PCB_0.jpg");
  cv::Mat eval_img_RGB = cv::imread("../imgs/evaluate/ELYOS_PCB_16.jpg");

  // Preprocess reference and evaluate image
  cv::Mat p_ref_img = preprocessImg(ref_img_RGB);
  cv::Mat p_eval_img = preprocessImg(eval_img_RGB); 
  
  // Show original and preprocessed reference image 
  cv::imshow("Original reference image", ref_img_RGB);
  cv::waitKey(0);
  cv::imshow("Preprocessed reference image", p_ref_img);
  cv::waitKey(0);
  cv::destroyAllWindows();

  return EXIT_SUCCESS;		
}
