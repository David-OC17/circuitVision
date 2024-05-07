#include <iostream>
#include <string>

#include "../include/csv.hpp"
#include "../include/aux_functions.hpp"

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

////////////////////////////////////////////////////////////////////////////////
///                           Auxiliary functions                            ///
////////////////////////////////////////////////////////////////////////////////

void display_img(cv::Mat &original_img, bool resize, int width, int height){
  // Resize if needed
  if (resize){
    cv::Mat display_original_img;
    cv::resize(original_img, display_original_img, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);

    cv::imshow("Original image", display_original_img);
    cv::waitKey(0);

  } else{
    cv::imshow("Original image", original_img);
    cv::waitKey(0);
  }
}

void display_imgs(cv::Mat &original_img, cv::Mat &preprocessed_img, bool resize, int width, int height){
  // Resize if needed
  if (resize){
    cv::Mat display_original_img;
    cv::resize(original_img, display_original_img, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);

    cv::Mat display_preprocessed_img;
    cv::resize(preprocessed_img, display_preprocessed_img, cv::Size(width, height), 0, 0, cv::INTER_LINEAR);

    cv::imshow("Original image", display_original_img);
    cv::waitKey(0);
    cv::imshow("Preprocessed image", display_preprocessed_img);
    cv::waitKey(0);

  } else{
    cv::imshow("Original image", original_img);
    cv::waitKey(0);
    cv::imshow("Preprocessed image", preprocessed_img);
    cv::waitKey(0);
  }
}

void printComponentPlace(io::CSVReader <3> comp_placement){
	std::string comp_name; float x_pos; float y_pos;

	while(comp_placement.read_row(comp_name, x_pos, y_pos)){
		std::cout << "Component name: " << comp_name << "\n";
    std::cout << "X position: " << x_pos << "\n";
		std::cout << "Y position: " << y_pos << "\n\n";
	}
}

void floodMask(cv::Mat &inputImg, int width, int height){
    // Calcualte center
    cv::Point center(width/2, height/2);

    // Floodfill from point center of image
    cv::Mat im_floodfill = inputImg.clone();
    cv::floodFill(im_floodfill, center, cv::Scalar(255));
 
    // Invert floodfilled image
    cv::Mat im_floodfill_inv;
    cv::bitwise_not(im_floodfill, im_floodfill_inv);
}

void closeMask(cv::Mat &inputImg, int repetitions){
  int morph_size = 25;

  // Create structuring element 
  cv::Mat element = getStructuringElement( 
      cv::MORPH_RECT, 
      cv::Size(2 * morph_size + 1, 
           2 * morph_size + 1), 
      cv::Point(morph_size, morph_size)); 

  cv::Mat output; 
  
  // Apply closing operation multiple times 
  while(repetitions){
    cv::morphologyEx(inputImg, output, 
             cv::MORPH_CLOSE, element, 
             cv::Point(-1, -1), 2); 
  
    repetitions --;
    inputImg = output;
  }
}
