/**
 * @file
 * @author David Ortiz
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * @section DESCRIPTION
 *
 * This header file provides a different definitions of functions
 * provided in the regular aux_functions.hpp header. All this
 * functions are meant to be used only for testing and should not
 * be kept for the final build of the project.
 */

#ifndef AUX_FUNCTIONS_TEST_H
#define AUX_FUNCTIONS_TEST_H

#include <string>
#include <vector>

/************************************************
 *                Picture taking
 ***********************************************/

/**
 * @brief Meant to replace takeRowPictures for testing purposes. The functions
 * takes pictures by accessing the available camera with OpenCV, instead of
 * using the libcamera-still command, which may only be used in the Raspberry
 * Pi.
 */
std::vector<std::string> takeRowPictures_test(std::string boardType);

/**
 * @brief Meant to replace takeRowPicture for testing purposes. The functions
 * takes a picture by accessing the available camera with OpenCV, instead of
 * using the libcamera-still command, which may only be used in the Raspberry
 * Pi.
 */
std::string takePicture_test(std::string boardType);

#endif // AUX_FUNCTIONS_TEST_H
