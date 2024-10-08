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
 * This file contains the implementation of functions that support the
 * main communication method, which is to be configured for communication
 * with the KL25 microcontroller.
 */

#include <cerrno>
#include <cstring>
#include <iostream>
#include <utility>

#include <wiringPi.h>
#include <wiringSerial.h>

std::pair<int, int> serialComsInit(int baudRate, std::string serialDevice) {
  const char *serialDevice_char = serialDevice.c_str();
  int serialPort;
  int errnum = errno;
  if ((serialPort = serialOpen(serialDevice_char, baudRate)) <
      0)
  {
    std::cerr << "Unable to open serial device: " << std::strerror(errnum)
              << std::endl;
    return std::make_pair(1, 0);
  }

  if (wiringPiSetup() == -1)
  {
    std::cerr << "Unable to start wiringPi: " << std::strerror(errnum)
              << std::endl;
    return std::make_pair(1, 0);
  }

  return std::make_pair(0, serialPort);
}
