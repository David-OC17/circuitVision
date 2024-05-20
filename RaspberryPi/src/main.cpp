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
 * This document contains the main implementation for the...
 */

#include <string>
#include <utility>
// #include <wiringPi.h>
// #include <wiringSerial.h>

#include "../include/PCB_inspection.hpp"
#include "../include/coms.hpp"

#include "../test/include/coms_test.hpp"

#define BOARD_TYPE std::string("ELYOS")
#define SERIAL_DEVICE std::string("/dev/ttyS0")
#define BAUD_RATE 9600

/**
 * Test missing for the following functions/portions of the code:
 * - serialComsInit() and general communication via UART (all wiringPi related)
 * --> test with RedPitaya
 *
 * - takeRowPictures() and takePicture(): test on Raspberry Pi
 *
 * Other needed changes:
 * - For the actual use of this code, change the SERIAL_DEVICE and verify
 * BAUD_RATE
 */

int main() {
  std::string boardType = BOARD_TYPE;
  std::string serialDevice = SERIAL_DEVICE;
  int baudRate = BAUD_RATE;

  // Initialize communication port
  std::pair<int, int> comsInfo = serialComsInit_test(baudRate, serialDevice);
  if (comsInfo.first) {
    throw NotifyError(
        "Error occurred during serial communication initialization.");
  }
  char dataUART;
  const int serialPort = comsInfo.second;

  while (1) {
    if (serialDataAvail_test(serialPort)) {
      dataUART = serialGetchar_test(serialPort); // receive character serially
      std::string completionMsg;

      switch (dataUART) {
      case 'R':
        completionMsg = rowMode(boardType);
        break;

      case 'O':
        completionMsg = oneMode(boardType);
        break;

      default:
        throw NotifyError("The received instruction is not valid.");
      }

      // Divide completion message into chars and send
      const char *completionMsg_chars = completionMsg.c_str();
      for (int i = 0; i < strlen(completionMsg_chars); i++) {
        // serialPutchar(serialPort, completionMsg_chars[i]);
        serialPutchar_test(serialPort, completionMsg_chars[i]);
      }
    }
  }

  return EXIT_SUCCESS;
}
