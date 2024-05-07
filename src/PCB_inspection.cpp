#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "../include/PCB_inspection.hpp"

////////////////////////////////////////////////////////////////////////////////
///                             Preprocessing                                ///
////////////////////////////////////////////////////////////////////////////////

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

cv::Mat correctPerspective(cv::Mat inputImg){
  // MISSING: Find contours and provide desired countours.
  
  // Get the transformation to adjust image to almost fullscreen and without perspective
  // cv::getPerspectiveTransform(); 

  cv::Mat finalImg;
  cv::Size img_size = inputImg.size();
  // cv::warpPerspective(inputImg, finalImg, perspectiveTransform, img_size);

  return finalImg;
}

////////////////////////////////////////////////////////////////////////////////
///                             Fault finding                                ///
////////////////////////////////////////////////////////////////////////////////
 
void noise_removal(cv::Mat &XOR_img, int closure_iterations, int ind_operation_iterations){
  cv::Mat closure_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
  cv::Mat XOR_eroded_img;
  cv::Mat XOR_dialated_img;

  // Apply closure via erossion and dilation
  cv::erode(XOR_img, XOR_eroded_img, closure_kernel, cv::Point(-1, -1), ind_operation_iterations);
  cv::dilate(XOR_eroded_img, XOR_dialated_img, closure_kernel, cv::Point(-1, -1), ind_operation_iterations);

  XOR_img = XOR_dialated_img;
}

cv::Mat colorFilterHSV(cv::Mat inputImg, std::vector<int> lowerLims, std::vector<int> upperLims){
  // Convert to HSV
  cv::Mat hsvImg;
  cv::cvtColor(inputImg, hsvImg, cv::COLOR_BGR2HSV);

  // Define the bound of the color to filter
  cv::Scalar lower_green(lowerLims[0], lowerLims[1], lowerLims[2]);
  cv::Scalar upper_green(upperLims[0], upperLims[1], upperLims[2]);

  // Create mask
  cv::Mat mask;
  cv::inRange(inputImg, lower_green, upper_green, mask);
  display_img(mask, true);

  // Repair mask (remove noise and holes)
  repairMask(mask);

  // Aplly AND with the mask
  cv::Mat filteredImg;
  cv::bitwise_and(inputImg, inputImg, filteredImg, mask);

  // Convert back to BGR
  cv::Mat result;
  cv::cvtColor(filteredImg, result, cv::COLOR_HSV2BGR);

  return result;
}
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

/**
 * @brief Fill holes of PCB outline, creating a solid rectangle.
 */
void repairMask(cv::Mat &inputImg){
    // Threshold.
    // Set values equal to or above 220 to 0.
    // Set values below 220 to 255.
    cv::Mat im_th;
    threshold(inputImg, im_th, 220, 255, cv::THRESH_BINARY_INV);
 
    // Floodfill from point (0, 0)
    cv::Mat im_floodfill = im_th.clone();
    cv::floodFill(im_floodfill, cv::Point(0,0), cv::Scalar(255));
 
    // Invert floodfilled image
    cv::Mat im_floodfill_inv;
    cv::bitwise_not(im_floodfill, im_floodfill_inv);
 
    // Combine the two images to get the foreground.
    // Change inputImg to resulting img
    inputImg = (im_th | im_floodfill_inv);
}

void closeMask(cv::Mat &inputImg){
  int morph_size = 3;

  // Create structuring element 
  cv::Mat element = getStructuringElement( 
      cv::MORPH_RECT, 
      cv::Size(2 * morph_size + 1, 
           2 * morph_size + 1), 
      cv::Point(morph_size, morph_size)); 
  cv::Mat output; 
  
  // Closing 
  cv::morphologyEx(inputImg, output, 
             cv::MORPH_CLOSE, element, 
             cv::Point(-1, -1), 2); 

  inputImg = output;
}
