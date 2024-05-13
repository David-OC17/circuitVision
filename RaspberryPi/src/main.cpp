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
#include <wiringPi.h>
#include <wiringSerial.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

#include "../include/PCB_inspection.hpp"
#include "../include/coms.hpp"

#define BOARD_TYPE "ELYOS"
#define SERIAL_DEVICE "/dev/ttyS0"
#define BAUD_RATE 9600

/**
 * Test missing for the following functions/portions of the code:
 * - serialComsInit() and general communication via UART (all wiringPi related)
 * - takeRowPictures() and takePicture() actually taking and saving pictures
 *
 * Implementation is missing for:
 * - backupPictures()
 *
 * Other needed changes:
 * - For the actual use of this code, change the SERIAL_DEVICE and verify BAUD_RATE
 */

int main() {
  // Initialize communication port
  std::pair<int, int> comsInfo = serialComsInit(BAUD_RATE, SERIAL_DEVICE);
  if (comsInfo.first) {
    throw NotifyError(
        "Error occurred during serial communication initialization.");
  }
  char dataUART;
  int serialPort = comsInfo.second;

  while (1) {
    if (serialDataAvail(serialPort)) {
      dataUART = serialGetchar(serialPort); // receive character serially
      std::string completionMsg;

      switch (dataUART) {
      case 'R':
        completionMsg = rowMode(BOARD_TYPE);
        break;

      case 'O':
        completionMsg = oneMode(BOARD_TYPE);
        break;
      }

      // Divide completion message into chars and send
      const char *completionMsg_chars = completionMsg.c_str();
      for (int i = 0; i < strlen(completionMsg_chars); i++) {
        serialPutchar(serialPort, completionMsg_chars[i]);
      }
    }
  }

  return EXIT_SUCCESS;
}
