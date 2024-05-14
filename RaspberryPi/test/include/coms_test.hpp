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
 * This file contains the implementation of functions for testing
 * of the general system, integrating control via serial communication.
 *
 * The provided functions are meant to replace some of the wiringPi
 * functions that enable serial communication.
 */

#ifndef COMS_TEST_H
#define COMS_TEST_H

#include <string>
#include <utility>

#include <wiringPi.h>
#include <wiringSerial.h>

/************************************************
 *             Other aux functions
 ***********************************************/

std::pair<int, int> serialComsInit_test(int baudRate, std::string serialDevice);

int serialPutchar_test(int serialPort, char transmit);

/************************************************
 *        WiringPi replacement functions
 ***********************************************/

bool serialDataAvail_test(const int fd);

char serialGetchar_test(int fd);

#endif // COMS_TEST_H
