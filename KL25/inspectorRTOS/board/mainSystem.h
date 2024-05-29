#ifndef MAIN_SYSTEM_H
#define MAIN_SYSTEM_H

#include "FreeRTOS.h"
#include "MKL25Z4.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "list.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "portmacro.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <stdio.h>

#include "LCD_8BIT.h"
#include "RGB.h"
#include "common.h"
#include "i2cdisplay.h"
#include "keypad.h"
#include "stdint.h"
#include "stepper.h"

/************************************************
 *         General macros and constants
 ***********************************************/
extern const int keyboardInputPin;

/************************************************
 *         Global variables and queues
 ***********************************************/
//////////   Steppers    /////////
extern Steppers stepperMotors;

extern const int stepperQueue_maxLen;
extern QueueHandle_t stepperXQueue;
extern QueueHandle_t stepperYQueue;

extern const int stepperQueue_maxLen;

extern TickType_t stepperAux_x;
extern TickType_t stepperAux_y;

//////////     ADC       /////////
extern uint16_t ADCres1, ADCres2;
extern TickType_t ADCaux;

//////////    UART     /////////
extern uint32_t ulUARTnotificationVal;
extern char uartOutCharBuffer;
extern int inputReceived; // 0:not received, 1:received

extern char uartInBuffer[50]; // Results info from RaspPi

//////////   Keypad    /////////
extern char operationMode;
extern char keypadInput;
extern int keypadInputAvailable; // Flag for when a change happens in

//////////    LCD     /////////
extern uint64_t printTime_aux;
extern uint32_t ulLCDprintNotificationVal;
extern char LCDBuffer[50];

//////////   Other    /////////
// Values for stepper motor ranges
extern const uint16_t stpMovePos;
extern const uint16_t stpMoveNeg;
// extern const uint16_t stpMoveNone = 2048;

extern const int timeTraverseRow;       // CHANGE
extern const int timeWaitAtWaypoint; // ms

extern uint32_t ulErrorNotificationVal;

extern uint32_t ulUserSelectNotificationVal;

/************************************************
 *           Mutexes and semaphores
 ***********************************************/
extern SemaphoreHandle_t evalResults_mux;

extern SemaphoreHandle_t keypadInput_sem;

extern SemaphoreHandle_t uartOutBuffer_mux;
extern SemaphoreHandle_t uartInBuffer_mux;
extern SemaphoreHandle_t LCDBuffer_mux;

extern SemaphoreHandle_t builtinLED_mux;

/************************************************
 *         		 Task functions
 ***********************************************/

//////////   Handlers   /////////
extern TaskHandle_t userSelectMode_handle;
extern TaskHandle_t UARTsend_handle;
extern TaskHandle_t LCDprint_handle;
extern TaskHandle_t moveXStepperMotor_handle;
extern TaskHandle_t moveYStepperMotor_handle;
extern TaskHandle_t calcStepperIns_handle;
extern TaskHandle_t displayResults_handle;
extern TaskHandle_t errorTaskStop_handle;

extern TaskHandle_t returnToOrigin_handle;
extern TaskHandle_t loadRowIns_handle;
extern TaskHandle_t loadAllIns_handle;

extern TaskHandle_t blinkRedLED_handle;
extern TaskHandle_t blinkYellowLED_handle;
extern TaskHandle_t blinkBlueLED_handle;
extern TaskHandle_t blinkGreenLED_handle;

//////////   Stack size (in words)    /////////
extern const int blinkLEDtask_stackDepth;
extern const int displayResults_stackDepth;
extern const int simpleTasks_stackDepth;
extern const int complexTasks_stackDepth;

//////////   Priority (1 to 5)   /////////
extern const int errorTaskStop_priority;
extern const int userSelectMode_priority;
extern const int UARTsend_priority;
extern const int LCDprint_priority;
extern const int moveStepperMotor_priority;
extern const int calcStepperIns_priority;
extern const int displayResults_priority;
extern const int blinkLED_priority;

/**
 * @brief Display menu message to LCD, wait for input,
 * send operation mode to RaspPi via UART.
 */
void userSelectMode(void *pvParameters);

/**
 * @brief Perform thread safe UART send operation.
 * Acquire uartOutBuffer_mux, pass message char by char,
 * release the mutex.
 */
void UARTsend(void *pvParameters);

/**
 * @brief
 */
void moveXStepperMotor(void *pvParameters);

/**
 * @brief
 */
void moveYStepperMotor(void *pvParameters);

/**
 * @brief Move to -X and -Y until we get to the limits. Stops when
 * an interruption is triggered by the limit buttons.
 */
void returnToOrigin(void *pvParameters);

/**
 * @brief Calculate instructions to move in x and y directions, in mm,
 * from the input of the joysticks.
 */
void calcStepperIns(void *pvParameters);

/**
 * @brief Stop motors from moving, notify error via LCD. Wait for reset
 * button input to reset all and continue (physical kl25 reset).
 */
void errorTaskStop(void *pvParameters);

/**
 * @brief Check LCD buffer and print contents to LCD.
 * Only display new and clear if there have been changes.
 */
void LCDprinter(void *pvParameters);

/**
 * @brief
 */
void loadRowIns(void *pvParameters);
/**
 * @brief
 */
void loadAllIns(void *pvParameters);

/**
 * @brief Display the contents of uartInBuffer to the LCD.
 * Acquire uartOutBuffer_mux, send data to LCD,
 * release the mutex.
 *
 */
void displayResults(void *pvParameters);

/************************************************
 *                 LED control
 ***********************************************/

/**
 * LED color meanings
 *
 * - Yellow blinking: receiving joystick input and adding instructions for
 * steppers, or loading instructions to queue.
 * - Blue blinking: displaying results to LCD.
 * - Green blinking: user selecting mode.
 * -
 * - Red constant: Error, terminating and restarting.
 */

/**
 * @brief Blink the builtinLED red color one time by toggling its state. Use of
 * mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkRedLED(void *pvParameters);

/**
 * @brief Blink the builtinLED yellow color one time by toggling its state. Use
 * of mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkYellowLED(void *pvParameters);

/**
 * @brief Blink the builtinLED blue color one time by toggling its state. Use of
 * mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkBlueLED(void *pvParameters);

/**
 * @brief Blink the builtinLED green color one time by toggling its state. Use
 * of mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkGreenLED(void *pvParameters);

/************************************************
 *          Interruptions and handlers
 ***********************************************/
// Receive inspection error

// Receive inspection error handler

// Receive evaluation results

// Receive evaluation results handler

// CNC collision
void configPORTA_IRQ(void);

void PORTA_IRQHandler(void);

// CNC collision handler --> stop motors (disable moveStepperMotor task),

#endif // MAIN_SYSTEM_H
