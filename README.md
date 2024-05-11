# circuitVision

**Disclosure**
_This project is still in development. Changes are being made almost everyday._

This repository contains a project for automatic optical inspection of PCB boards done using a RaspberryPi 4 and KL25 microcontroller. The system is able to automatically take pictures of PCB boards, via the use of a CNC machine, and evaluate if the components of the PCB are correctly placed and present.

## General orchestration / user interaction
### Modes of operation
The system will provide two modes of operation that the user may choose from by making the selection from the keypad, and seeing its menu on the LCD display.

The modes of operation are:
1. Manual configuration and picture taking
2. Automatic inspection of preconfigured PCB board

The _manual configuration_ option allows the user to move the camera, which is mounted on the final actuator of the CNC, via the use of the joystick. The user may also instruct the system to take a picture of what the camera is seeing at that moment, which automatically triggers the system to evaluate for the selected PCB board design.

The _automatic inspection_ mode is recommended. This allows the user to inspect up to as many board are know to fit in the test bed. The CNC goes over each of the PCBs, taking a picture of each of them, saving the results, and displaying them to the monitor.

## KL25
The code that runs on the KL25 micontroller corresponds to the interaction or control of the following hardware modules:
- LCD display
- Joytick
- 3 x 3 keypad
- Stepper motor driver control
- Communication with Raspberry Pi 4 via UART

In this manner, the KL25 is in charge of the general orchestration of the system, only communicating to the Raspberry Pi 4 when the actual PCB evaluation is needed.

## Raspberry Pi 4
The modules that run on the Raspberry Pi 4 correspond to the image processing and evaluation portion of the system. OpenCV in C++ is used for this purpose. These modules provide a way for the user to take pictures of the PCB board being evaluated at the CNC bed, and get back the results to the KL25, which displays them to the LCD. The system may also be configured to show the results to a monitor, using the standard HDMI connection to the Raspberry Pi board.

### How the pipeline of the image automatic optical evaluation works
The design of this system is based on the work presented on the following papers/surveys:
- _PCB Defect Detection USING OPENCV with Image Subtraction Method_, by Fa Iq Raihan and Win Ce
- _Automatic PCB Inspection Algorithms: A Survey_, by Madhav Mogonti and Fikret Ercal

The general operations being done for the evaluation are the following:
1. Take and save the picture
2. Create a mask to filter the white background
3. Find the largest contour, which corresponds to the PCB itself
4. Correct the perspective and enlarge to fit the complete window
5. Apply a preprocessing step where the image is blurred, edges are found, and it converted to one channel
6. Apply a XOR operation between the reference and evaluation image
7. Remove noise from the resulting XOR
8. Read from a CSV the coordinates corresponding to bounding boxes of the components
9. Generate a box image for each component
10. Calculate percentage of lit pixels in the image and compare to allowed maximum
11. Display and save results

## Some preview images
See the following temporary pictures as reference:

<div style="display:flex">
  <img src="./imgs/ref_PCB_board.png" alt="Image 1" style="width:50%">
  <img src="./imgs/eval_results.png" alt="Image 2" style="width:50%">
</div>

<div style="display:flex">
  <img src="./imgs/stepper_motors.png" alt="Image 1" style="width:50%">
  <img src="./imgs/CNC_machine.png" alt="Image 2" style="width:50%">
</div>