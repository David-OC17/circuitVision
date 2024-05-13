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
 * This header file specifies all the functions related to the communication
 * between the Raspberry Pi and KL25 board via UART, which also includes minor
 * orchestration within the Raspberry Pi modules.
 */

#include <exception>
#include <string>
#include <utility>

/**
 * @brief Opens the port for serial communication.
 *
 * @return Returns a pair of {EXIT_STATUS, serialPort}
 */
std::pair<int, int> serialComsInit(int baudRate, std::string serialDevice);

/************************************************
 *              Exception handling
 ***********************************************/

/**
 * @brief Class to process generic errors that stop the execution of the
 * evaluation process
 */
class NotifyError : public std::exception {
public:
  NotifyError(const std::string &message) : message(message) {}

  const char *what() const noexcept override { return message.c_str(); }

private:
  std::string message;
};
