/************************************************
 *            Default template imports
 ***********************************************/
#include "FreeRTOS.h"
#include "MKL25Z4.h"
#include "board.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "portmacro.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "list.h"
#include <stdio.h>

/************************************************
 *                Extra imports
 ***********************************************/
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
const int keyboardInputPin = 10;

/************************************************
 *           Mutexes and semaphores
 ***********************************************/
static SemaphoreHandle_t evalResults_mux;

static SemaphoreHandle_t keypadInput_sem;

static SemaphoreHandle_t uartOutBuffer_mux;
static SemaphoreHandle_t uartInBuffer_mux;
static SemaphoreHandle_t LCDBuffer_mux;

static SemaphoreHandle_t builtinLED_mux;

/************************************************
 *         Global variables and queues
 ***********************************************/
//////////   Steppers    /////////
static Steppers stepperMotors;

const int stepperQueue_maxLen = 6;
static QueueHandle_t stepperXQueue;
static QueueHandle_t stepperYQueue;

static TickType_t stepperAux_x = 0;
static TickType_t stepperAux_y = 0;

//////////     ADC       /////////
static uint16_t ADCres1 = 0, ADCres2 = 0;
static TickType_t ADCaux = 0;

//////////    UART     /////////
static uint32_t ulUARTnotificationVal;
static char uartOutCharBuffer;
int inputReceived = 0; // 0:not received, 1:received

static char uartInBuffer[50] = {0}; // Results info from RaspPi

//////////   Keypad    /////////
static char operationMode;
static char keypadInput;
static int keypadInputAvailable = 0; // Flag for when a change happens in

//////////    LCD     /////////
static uint64_t printTime_aux = 0;
static uint32_t ulLCDprintNotificationVal;
static char LCDBuffer[50];

//////////   Other    /////////
// Values for stepper motor ranges
static const uint16_t stpMovePos = 3500;
static const uint16_t stpMoveNeg = 500;
//static const uint16_t stpMoveNone = 2048;

static const int timeTraverseRow = 0; // CHANGE
static const int timeWaitAtWaypoint = 3000; // ms

static uint32_t ulErrorNotificationVal;

static uint32_t ulUserSelectNotificationVal;

/************************************************
 *         		 Task functions
 ***********************************************/

//////////   Handlers   /////////
static TaskHandle_t userSelectMode_handle = NULL;
static TaskHandle_t UARTsend_handle = NULL;
static TaskHandle_t LCDprint_handle = NULL;
static TaskHandle_t moveXStepperMotor_handle = NULL;
static TaskHandle_t moveYStepperMotor_handle = NULL;
static TaskHandle_t calcStepperIns_handle = NULL;
static TaskHandle_t displayResults_handle = NULL;
static TaskHandle_t errorTaskStop_handle = NULL;

static TaskHandle_t returnToOrigin_handle = NULL;
static TaskHandle_t loadRowIns_handle = NULL;
static TaskHandle_t loadAllIns_handle = NULL;

static TaskHandle_t blinkRedLED_handle = NULL;
static TaskHandle_t blinkYellowLED_handle = NULL;
static TaskHandle_t blinkBlueLED_handle = NULL;
static TaskHandle_t blinkGreenLED_handle = NULL;

//////////   Stack size (in words)    /////////
static const int blinkLEDtask_stackDepth = 128;
static const int displayResults_stackDepth = 128;
static const int simpleTasks_stackDepth = 128;
static const int complexTasks_stackDepth = 1024;

//////////   Priority (1 to 5)   /////////
static const int errorTaskStop_priority = 5;
static const int userSelectMode_priority = 4;
static const int UARTsend_priority = 3;
static const int LCDprint_priority = 3;
static const int moveStepperMotor_priority = 4;
static const int calcStepperIns_priority = 4;
static const int displayResults_priority = 2;
static const int blinkLED_priority = 1;

/**
 * @brief Display menu message to LCD, wait for input,
 * send operation mode to RaspPi via UART.
 */
void userSelectMode(void *pvParameters) {
  for (;;) {
    // Write menu to LCD print variable
    xSemaphoreTake(evalResults_mux, portMAX_DELAY);
    strcpy(LCDBuffer, "Inspection modes: O(one PCB), R(row of PCBs)");
    xSemaphoreGive(evalResults_mux);
    vTaskDelay(500 / portTICK_RATE_MS); // Let the LCD task run

    // Wait for the user to type the mode
    if (xSemaphoreTake(keypadInput_sem, portMAX_DELAY) == pdTRUE) {
      switch (operationMode) { // CHECK MAPPING TO THE MATRIX KEYPAD
      case 'A':
        uartOutCharBuffer = 'O';
        vTaskResume(calcStepperIns_handle);
        break;

      case 'B':
        uartOutCharBuffer = 'R';
        vTaskResume(calcStepperIns_handle);
        break;

      case 'M':
    	  uartOutCharBuffer = 'M'; // Missing as RaspPi option
    	  vTaskResume(calcStepperIns_handle);
    	  break;
      }

      vTaskResume(UARTsend_handle);
      xTaskNotify(ulUARTnotificationVal, 1, eSetBits);

      if (xTaskNotifyWait(0, 0, &ulUserSelectNotificationVal, portMAX_DELAY) == pdTRUE)
        vTaskSuspend(UARTsend_handle);
    }
  }
}

/**
 * @brief Perform thread safe UART send operation.
 * Acquire uartOutBuffer_mux, pass message char by char,
 * release the mutex.
 */
void UARTsend(void *pvParameters) {
  for (;;) {
    if (xTaskNotifyWait(0, 0, &ulUARTnotificationVal, portMAX_DELAY) ==
        pdTRUE) {
      xSemaphoreTake(uartOutBuffer_mux, portMAX_DELAY);

      printTime_aux = xTaskGetTickCount();
      UART0->D = uartOutCharBuffer;
      while (!(UART0->S1 & 0x40)) { // Wait for the TX buffer to be empty
        if (xTaskGetTickCount() - printTime_aux >= UART_TIMEOUT_TICKS)
          break;
      }

      xSemaphoreGive(uartOutBuffer_mux);

      xTaskNotifyGive(userSelectMode_handle);
    }
  }
}

/**
 * @brief
 */
void moveXStepperMotor(void *pvParameters) {
  for (;;) {
    uint16_t ADCval = 0;

    // Send available instruction to motors
    if (xQueueReceive(stepperXQueue, &ADCval, 0)) {
      if (ADCval > 3070)
        SetStepperDirection(&stepperMotors.xStepper, CLOCKWISE);
      else if (ADCval < 1023)
        SetStepperDirection(&stepperMotors.xStepper, COUNTERCLOCKWISE);
      else
        continue; // skip this iteration, no movement to happen

      if (xTaskGetTickCount() - stepperAux_x >= TICKS_TO_RUN_STEPPER) {
        for (uint8_t i = 0; i < 200; i++) {
          RunStepper(&stepperMotors.xStepper);
        }
        stepperAux_x = xTaskGetTickCount();
      }
      vTaskDelay(pdMS_TO_TICKS(timeWaitAtWaypoint)); // Delay 3 seconds to wait to take image
    }
  }
}

/**
 * @brief
 */
void moveYStepperMotor(void *pvParameters) {
  for (;;) {
    uint16_t ADCval = 0;

    // Send available instruction to motors
    if (xQueueReceive(stepperYQueue, &ADCval, 0)) {
      if (ADCval > 3070)
        SetStepperDirection(&stepperMotors.yStepper, CLOCKWISE);
      else if (ADCval < 1023)
        SetStepperDirection(&stepperMotors.yStepper, COUNTERCLOCKWISE);
      else
        continue; // skip this iteration, no movement to happen

      if (xTaskGetTickCount() - stepperAux_y >= TICKS_TO_RUN_STEPPER) {
        for (uint8_t i = 0; i < 200; i++) {
          RunStepper(&stepperMotors.yStepper);
        }
        stepperAux_y = xTaskGetTickCount();
      }
      vTaskDelay(pdMS_TO_TICKS(timeWaitAtWaypoint)); // Delay 3 seconds to wait to take image
    }
  }
}

/**
 * @brief Move to -X and -Y until we get to the limits. Stops when
 * an interruption is triggered by the limit buttons.
 */
void returnToOrigin(void *pvParameters){
	// MISSING
}

/**
 * @brief Calculate instructions to move in x and y directions, in mm,
 * from the input of the joysticks.
 */
void calcStepperIns(void *pvParameters) {
  for (;;) {
    // Blink yellow LED
    vTaskResume(blinkYellowLED_handle);

    // Read ADC and clear flag --> CHECK LOGIC
    if (xTaskGetTickCount() - ADCaux >= TICKS_TO_READ_ADC) {
      ADCaux = xTaskGetTickCount();

      while (!(ADC0->SC1[0] & 0x80)) { // Wait for end of conversion flag
        if (xTaskGetTickCount() - ADCaux >= ADC_TIMEOUT_TICKS)
          break;
      }
      // Assign ADC result
      ADCres1 = (ADC0->R[0] >> 4) - 10;
      ADCaux = xTaskGetTickCount();

      // Start ADC conversion in channel 4
      ADC0->SC1[0] = 0x04;

      while (!(ADC0->SC1[0] & 0x80)) { // Wait for end of conversion flag
        if (xTaskGetTickCount() - ADCaux >= ADC_TIMEOUT_TICKS)
          break;
      }
      // Store result
      ADCres2 = (ADC0->R[0] >> 4) - 10;
      ADCaux = xTaskGetTickCount();
      // Start ADC conversion in channel 0
      ADC0->SC1[0] = 0;
    }

    xQueueSend(stepperXQueue, (void *)&ADCres1, 0);
    xQueueSend(stepperYQueue, (void *)&ADCres2, 0);

    // Stop yellow LED blink
    vTaskSuspend(blinkYellowLED_handle);
  }
}

/**
 * @brief Stop motors from moving, notify error via LCD. Wait for reset
 * button input to reset all and continue (physical kl25 reset).
 */
void errorTaskStop(void *pvParameters){
	for(;;){
		if (xTaskNotifyWait(0, 0, &ulErrorNotificationVal, portMAX_DELAY) ==
		        pdTRUE){
			vTaskSuspend(moveXStepperMotor_handle);
			vTaskSuspend(moveYStepperMotor_handle);
			vTaskSuspend(calcStepperIns_handle);
			vTaskSuspend(userSelectMode_handle);
			vTaskSuspend(UARTsend_handle);
			vTaskSuspend(displayResults_handle);

			vTaskResume(blinkRedLED_handle);

			// LCDBuffer = 0;

			// Print to LCD
			xTaskNotify(ulLCDprintNotificationVal, 1, eSetBits);

			while(1); // Stop all until micro-controller restart
		}
	}
}

/**
 * @brief Check LCD buffer and print contents to LCD.
 * Only display new and clear if there have been changes.
 */
void LCDprinter(void *pvParameters){
	char compLCDBuffer[50];
	strcpy(compLCDBuffer, LCDBuffer);

	for(;;){
		if (xTaskNotifyWait(0, 0, &ulLCDprintNotificationVal, portMAX_DELAY) ==
				        pdTRUE){
			// Check for changes in LCD buffer
			if (strcmp(compLCDBuffer, LCDBuffer) != 0){
				strcpy(compLCDBuffer, LCDBuffer);

				LCD8_Clear();
				int i = 0;
				while (LCDBuffer[i] != '\0') {
					LCD8_Write(&LCDBuffer[i]);
					i++;
				}
			}
		}
	}
}

/**
 * @brief
 */
void loadRowIns(void *pvParameters){
	for(;;){
		xQueueSend(stepperXQueue, (void *)&stpMovePos, 0);
		xQueueSend(stepperXQueue, (void *)&stpMovePos, 0);
		xQueueSend(stepperXQueue, (void *)&stpMovePos, 0);

		vTaskDelay(timeTraverseRow);

		vTaskSuspend(NULL);
	}
}

/**
 * @brief
 */
void loadAllIns(void *pvParameters) {
	for(;;){
	xQueueSend(stepperXQueue, (void *)&stpMovePos, 0);
	xQueueSend(stepperXQueue, (void *)&stpMovePos, 0);
	xQueueSend(stepperXQueue, (void *)&stpMovePos, 0);

	vTaskDelay(timeTraverseRow);

	xQueueSend(stepperYQueue, (void *)&stpMovePos, 0);

	xQueueSend(stepperYQueue, (void *)&stpMoveNeg, 0);
	xQueueSend(stepperYQueue, (void *)&stpMoveNeg, 0);
	xQueueSend(stepperYQueue, (void *)&stpMoveNeg, 0);

	vTaskDelay(timeTraverseRow);

	xQueueSend(stepperYQueue, (void *)&stpMovePos, 0);

	xQueueSend(stepperYQueue, (void *)&stpMoveNeg, 0);
	xQueueSend(stepperYQueue, (void *)&stpMoveNeg, 0);
	xQueueSend(stepperYQueue, (void *)&stpMoveNeg, 0);

	vTaskSuspend(NULL);
	}
}

/**
 * @brief Display the contents of uartInBuffer to the LCD.
 * Acquire uartOutBuffer_mux, send data to LCD,
 * release the mutex.
 *
 */
void displayResults(void *pvParameters) {
  xSemaphoreTake(uartInBuffer_mux, portMAX_DELAY);

  // Write UART input to LCDbuffer
  vTaskDelay(timeWaitAtWaypoint); // REMOVE

  xSemaphoreGive(uartInBuffer_mux);

  // Print to LCD
  xTaskNotify(ulLCDprintNotificationVal, 1, eSetBits);

  // Return to origin
}

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
void blinkRedLED(void *pvParameters) {
  xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
  RedToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  RedToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  xSemaphoreGive(builtinLED_mux);
}

/**
 * @brief Blink the builtinLED yellow color one time by toggling its state. Use
 * of mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkYellowLED(void *pvParameters) {
  xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
  YellowToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  YellowToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  xSemaphoreGive(builtinLED_mux);
}

/**
 * @brief Blink the builtinLED blue color one time by toggling its state. Use of
 * mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkBlueLED(void *pvParameters) {
  xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
  BlueToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  BlueToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  xSemaphoreGive(builtinLED_mux);
}

/**
 * @brief Blink the builtinLED green color one time by toggling its state. Use
 * of mutex included to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be
 * preempted, causing the mutex to be held for longer, and the toggle to be done
 * later than expected.
 */
void blinkGreenLED(void *pvParameters) {
  xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
  GreenToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  GreenToggle();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  xSemaphoreGive(builtinLED_mux);
}

/************************************************
 *          Interruptions and handlers
 ***********************************************/
// Receive inspection error

// Receive inspection error handler

// Receive evaluation results

// Receive evaluation results handler

// CNC collision
void configPORTA_IRQ(void){
	/* configure PTA1 for interrupt */
	PORTA->PCR[1] |= 0x00100; /* make it GPIO */
	PORTA->PCR[1] |= 0x00003; /* enable pull-up */
	PTA->PDDR &= ~0x0002; /* make pin input */
	PORTA->PCR[1] &= ~0xF0000; /* clear interrupt selection */
	PORTA->PCR[1] |= 0xA0000; /* enable falling edge INT */

	/*configure PTA2 for interrupt*/
	PORTA->PCR[2] |= 0x00100; /* make it GPIO */
	PORTA->PCR[2] |= 0x00003; /* enable pull-up */
	PTA->PDDR &= ~0x0004; /* make pin input */
	PORTA->PCR[2] &= ~0xF0000; /* clear interrupt selection */
	PORTA->PCR[2] |= 0xA0000; /* enable falling edge INT */

	/*configure PTA3 for interrupt*/
	PORTA->PCR[3] |= 0x00100; /* make it GPIO */
	PORTA->PCR[3] |= 0x00003; /* enable pull-up */
	PTA->PDDR &= ~0x0006; /* make pin input */
	PORTA->PCR[3] &= ~0xF0000; /* clear interrupt selection */
	PORTA->PCR[3] |= 0xA0000; /* enable falling edge INT */

	NVIC->ISER[0] |= 0x40000000; /* enable INT30 (bit 30 of ISER[0]) */

	__enable_irq(); /* global enable IRQs */
}

void PORTA_IRQHandler(void){
	while (PORTA->ISFR & 0x00000007) {
		// Stop from RaspPi
		if (PORTA->ISFR & 0x00000001) {
			// Stop motors, show error in LCD
			//vTaskNotifyGiveFromISR();
		}
		// Stop button X axis
		else if (PORTA->ISFR & 0x00000002) {


		}
		// Stop button Y axis
		else if (PORTA->ISFR & 0x00000004){

		}
	}

	PORTA->ISFR = 0x00000004; /* clear interrupt flag */
}

// CNC collision handler --> stop motors (disable moveStepperMotor task),

/************************************************
 *         		      Main
 ***********************************************/
int main(void) {
  //////////  Hardware modules init  //////////
  stepperMotors = InitAll();
  // configure LED missing --> configure for external LEDs

  //////////  Configuration of tasks //////////
  xTaskCreate(userSelectMode, "userSelectModeTask", complexTasks_stackDepth, NULL,
              userSelectMode_priority, &userSelectMode_handle);

  xTaskCreate(UARTsend, "UARTsendTask", simpleTasks_stackDepth, NULL,
              UARTsend_priority, &UARTsend_handle);

  xTaskCreate(LCDprint_handle, "LCDprintTask", complexTasks_stackDepth, NULL,
                LCDprint_priority, &LCDprint_handle);

  xTaskCreate(calcStepperIns, "calcStepperInsTask", complexTasks_stackDepth, NULL,
              calcStepperIns_priority, &calcStepperIns_handle);

  xTaskCreate(calcStepperIns, "calcStepperInsTask", complexTasks_stackDepth, NULL,
              calcStepperIns_priority, &calcStepperIns_handle);

  xTaskCreate(displayResults, "displayResultsTask", displayResults_stackDepth, NULL,
              displayResults_priority, &displayResults_handle);

  //////////    Blink LED   //////////
  xTaskCreate(blinkRedLED, "blinkRedLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkRedLED_handle);
  vTaskSuspend(blinkRedLED_handle);

  xTaskCreate(blinkYellowLED, "blinkYellowLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkYellowLED_handle);
  vTaskSuspend(blinkYellowLED_handle);

  xTaskCreate(blinkBlueLED, "blinkBlueLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkBlueLED_handle);
  vTaskSuspend(blinkBlueLED_handle);

  xTaskCreate(blinkGreenLED, "blinkGreenLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkGreenLED_handle);
  vTaskSuspend(blinkGreenLED_handle);


  //////////    Stepper movement related   //////////
  xTaskCreate(moveXStepperMotor, "moveXStepperMotorTask", complexTasks_stackDepth,
              NULL, moveStepperMotor_priority, &moveXStepperMotor_handle);

  xTaskCreate(moveYStepperMotor, "moveYStepperMotorTask", complexTasks_stackDepth,
              NULL, moveStepperMotor_priority, &moveYStepperMotor_handle);

  xTaskCreate(returnToOrigin, "returnToOriginTask", simpleTasks_stackDepth, NULL,
                moveStepperMotor_priority, &returnToOrigin_handle);
  vTaskSuspend(returnToOrigin_handle);

  xTaskCreate(loadRowIns, "loadRowInsTask", simpleTasks_stackDepth, NULL,
			   moveStepperMotor_priority, &loadRowIns_handle);
  vTaskSuspend(returnToOrigin_handle);

  xTaskCreate(loadAllIns, "loadAllInsTask", simpleTasks_stackDepth, NULL,
			  errorTaskStop_priority, &errorTaskStop_handle);
  vTaskSuspend(returnToOrigin_handle);

  //////////    Error handling  //////////
  xTaskCreate(errorTaskStop, "errorTaskStopTask", simpleTasks_stackDepth, NULL,
  			  moveStepperMotor_priority, &loadAllIns_handle);
    vTaskSuspend(returnToOrigin_handle);


  //////////    Mutexes and queues   //////////
  vSemaphoreCreateBinary(evalResults_mux);
  vSemaphoreCreateBinary(uartOutBuffer_mux);
  vSemaphoreCreateBinary(uartInBuffer_mux);
  vSemaphoreCreateBinary(uartInBuffer_mux);
  vSemaphoreCreateBinary(LCDBuffer_mux);

  stepperXQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));
  stepperYQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));

  //////////      Interruptions      //////////

  //////////      Start scheduler      //////////
  vTaskStartScheduler();
  for (;;);

  return 0;
}
