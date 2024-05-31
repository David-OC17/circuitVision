#include "mainSystem.h"

/************************************************
 *         General macros and constants
 ***********************************************/
const int keyboardInputPin = 10;

/************************************************
 *           Mutexes and semaphores
 ***********************************************/
SemaphoreHandle_t evalResults_mux;

SemaphoreHandle_t keypadInput_sem;

SemaphoreHandle_t uartOutBuffer_mux;
SemaphoreHandle_t uartInBuffer_mux;
SemaphoreHandle_t LCDBuffer_mux;

SemaphoreHandle_t builtinLED_mux;

/************************************************
 *         Global variables and queues
 ***********************************************/
//////////   Steppers    /////////
Steppers stepperMotors;

const int stepperQueue_maxLen = 10;
QueueHandle_t stepperXQueue;
QueueHandle_t stepperYQueue;

TickType_t stepperAux_x = 0;
TickType_t stepperAux_y = 0;

//////////     ADC       /////////
uint16_t ADCres1 = 0, ADCres2 = 0;
TickType_t ADCaux = 0;

//////////    UART     /////////
uint32_t ulUARTnotificationVal;
char uartOutCharBuffer;
int inputReceived = 0; // 0:not received, 1:received

char uartInBuffer[50] = {0}; // Results info from RaspPi

//////////   Keypad    /////////
char operationMode;
char keypadInput;
int keypadInputAvailable = 0; // Flag for when a change happens in

//////////    LCD     /////////
uint64_t printTime_aux = 0;
uint32_t ulLCDprintNotificationVal;
char LCDBuffer[50];

//////////   Other    /////////
// Values for stepper motor ranges
const uint16_t stpMovePos = 3500;
const uint16_t stpMoveNeg = 500;
const uint16_t stpMoveNone = 2048;

const int timeTraverseRow = 1000; // CHANGE
const int timeWaitAtWaypoint = 3000; // ms
const int timeWaitMotorIns = 1000;

uint32_t ulErrorNotificationVal;

uint32_t ulUserSelectNotificationVal;

/************************************************
 *         		 Task functions
 ***********************************************/

//////////   Handlers   /////////
TaskHandle_t userSelectMode_handle = NULL;
TaskHandle_t UARTsend_handle = NULL;
TaskHandle_t LCDprint_handle = NULL;
TaskHandle_t moveXStepperMotor_handle = NULL;
TaskHandle_t moveYStepperMotor_handle = NULL;
TaskHandle_t calcStepperIns_handle = NULL;
TaskHandle_t displayResults_handle = NULL;
TaskHandle_t errorTaskStop_handle = NULL;

TaskHandle_t returnToOrigin_handle = NULL;
TaskHandle_t loadRowIns_handle = NULL;
TaskHandle_t loadAllIns_handle = NULL;

TaskHandle_t blinkRedLED_handle = NULL;
TaskHandle_t blinkYellowLED_handle = NULL;
TaskHandle_t blinkBlueLED_handle = NULL;
TaskHandle_t blinkGreenLED_handle = NULL;

//////////   Stack size (in words)    /////////
const uint16_t blinkLEDtask_stackDepth = 16;
const uint16_t displayResults_stackDepth = 16;
const uint16_t simpleTasks_stackDepth = 16;
const uint16_t complexTasks_stackDepth = 32;

//////////   Priority (1 to 5)   /////////
const uint32_t errorTaskStop_priority = 5;
const uint32_t userSelectMode_priority = 4;
const uint32_t UARTsend_priority = 3;
const uint32_t LCDprint_priority = 3;
const uint32_t moveStepperMotor_priority = 4;
const uint32_t calcStepperIns_priority = 4;
const uint32_t displayResults_priority = 2;
const uint32_t blinkLED_priority = 1;


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

void moveXStepperMotor(void *pvParameters) {
  for (;;) {
    uint16_t ADCval = stpMoveNone;

    // Send available instruction to motors
    if (xQueueReceive(stepperXQueue, &ADCval, timeWaitMotorIns) == pdTRUE) {
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
      vTaskDelay(timeWaitAtWaypoint); // Delay 3 seconds to wait to take image
    }
  }
}

void moveYStepperMotor(void *pvParameters) {
  for (;;) {
    uint16_t ADCval = stpMoveNone;

    // Send available instruction to motors
    if (xQueueReceive(stepperYQueue, &ADCval, timeWaitMotorIns) == pdTRUE) {
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
      vTaskDelay(timeWaitAtWaypoint); // Delay 3 seconds to wait to take image
    }
  }
}

void returnToOrigin(void *pvParameters){
	// MISSING
}

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

void LCDprint(void *pvParameters){
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

void loadRowIns(void *pvParameters){
	for(;;){
		if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
		if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
		if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);

		vTaskDelay(timeTraverseRow);

		vTaskSuspend(NULL);
	}
}

void loadAllIns(void *pvParameters) {
	for(;;){
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);

	vTaskDelay(timeTraverseRow);

	if(xQueueSend(stepperYQueue, &stpMovePos, 0) == pdTRUE);

	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);

	vTaskDelay(timeTraverseRow);

	if(xQueueSend(stepperYQueue, &stpMovePos, 0) == pdTRUE);

	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);
	if(xQueueSend(stepperXQueue, &stpMovePos, portMAX_DELAY) == pdTRUE);

	vTaskSuspend(NULL);
	}
}

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

void blinkRedLED(void *pvParameters) {
	for(;;) {
	        if (xSemaphoreTake(builtinLED_mux, pdMS_TO_TICKS(1000)) == pdTRUE) {
	            RedOn();
	            vTaskDelay(pdMS_TO_TICKS(1000));
	            RedOff();
	            xSemaphoreGive(builtinLED_mux);
	        }
	        vTaskDelay(pdMS_TO_TICKS(1000));
	    }
}

void blinkYellowLED(void *pvParameters) {
	for(;;) {
	        if (xSemaphoreTake(builtinLED_mux, pdMS_TO_TICKS(1000)) == pdTRUE) {
	            YellowOn();
	            vTaskDelay(pdMS_TO_TICKS(1000));
	            YellowOff();
	            xSemaphoreGive(builtinLED_mux);
	        }
	        vTaskDelay(pdMS_TO_TICKS(1000));
	    }
}

void blinkBlueLED(void *pvParameters) {
for(;;) {
		if (xSemaphoreTake(builtinLED_mux, pdMS_TO_TICKS(1000)) == pdTRUE) {
			BlueOn();
			vTaskDelay(pdMS_TO_TICKS(1000));
			BlueOff();
			xSemaphoreGive(builtinLED_mux);
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void blinkGreenLED(void *pvParameters) {
for(;;) {
		if (xSemaphoreTake(builtinLED_mux, pdMS_TO_TICKS(1000)) == pdTRUE) {
			GreenOn();
			vTaskDelay(pdMS_TO_TICKS(1000));
			BlueOff();
			xSemaphoreGive(builtinLED_mux);
		}
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
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
