#include <iostream>
#include <string>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/csv.hpp"
#include "../include/PCB_inspection.hpp"

// g++ -I/usr/include/opencv4 main.cpp -o main -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_highgui

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

  // Deskew image
  cv::Mat ref_img_rotated = deskew(ref_img_RGB);
  
  // Show output images
  cv::imshow("Original image", ref_img_RGB);
  cv::waitKey(0);
  cv::imshow("Rotated image", ref_img_rotated);
  cv::waitKey(0);
  cv::destroyAllWindows();
  

return EXIT_SUCCESS;		
}
