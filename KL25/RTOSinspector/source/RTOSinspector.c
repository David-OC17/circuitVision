/************************************************
 *            Default template imports
 ***********************************************/
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"

/************************************************
 *                Extra imports
 ***********************************************/
#include "../../include/common.h"
#include "../../include/clocks.h"
#include "../../include/i2cdisplay.h"
#include "../../include/LCD_8BIT.h"
#include "../../include/RGB.h"
#include "../../include/stepper.h"
#include "../../include/keypad.h"

/************************************************
 *         General macros and constants
 ***********************************************/
#define FALLING_EDGE_INTERRUPT 0b10

const int keyboardInputPin = 10;


/************************************************
 *           Mutexes and semaphores
 ***********************************************/
static SemaphoreHandle_t evalResults_mux;

static SemaphoreHandle_t uartOutBuffer_mux;
static SemaphoreHandle_t uartInBuffer_mux;

static SemaphoreHandle_t builtinLED_mux;

/************************************************
 *         Global variables and queues
 ***********************************************/
//////////   Steppers    /////////
static Steppers stepperMotors;

const int steppperQueue_maxLen = 6;
static QueueHandle_t stepperXQueue;
static QueueHandle_t stepperYQueue;

static uint64_t stepperAux_x = 0;
static uint64_t stepperAux_y = 0;


//////////     ADC       /////////
static uint16_t ADCres1 = 0, ADCres2 = 0;
static uint64_t ADCaux = 0;

//////////   Systick    /////////
static const uint64_t *systicks;

//////////    UART     /////////
static char uartOutBuffer[50];
static char uartInBuffer[50]; // Results info from RaspPi
static int inputReceived = 0; // 0:not received, 1:received

//////////   Keypad    /////////
static char operationMode;
static char keypadInput;
static int  keypadInputAvailable = 0; // Flag for when a change happens in

//////////    LCD     /////////
static const uint64_t printTime_aux = 0;


//////////   Other    /////////
static const uint64_t *systicks;


/************************************************
 *         		 Task functions
 ***********************************************/

/**
 * @brief Display menu message to LCD, wait for input,
 * send operation mode to RaspPi via UART.
 */
void userSelectMode(void *evalResults_mux){
	// Write menu to LCD print variable
	xSemaphoreTake(evalResults_mux, portMAX_DELAY);
	evalResults = "Inspection modes: O(one PCB), R(row of PCBs)";
	xSemaphoreGive(evalResults_mux);
	vTaskDelay( 500 / portTICK_RATE_MS ); // Let the LCD task run

	// Wait for the user to type the mode
	while(!keypadInputAvailable){
		vTaskDelay( 500 / portTICK_RATE_MS );
	}

	// Sends mode char via UART to Raspberry Pi

	// remove UARTsend from suspension
	// wait for the task to complete
	// put UARTsend back to suspension
}

/**
 * @brief Perform thread safe UART send operation.
 * Acquire uartOutBuffer_mux, pass message char by char,
 * release the mutex.
 */
void UARTsend(void){
	xSemaphoreTake(uartOutBuffer_mux, portMAX_DELAY);

	for(uint8_t i=0; i < strlen(uartOutBuffer); i++){
	  printTime_aux = *systicks;
	  UART0->D = uart_c[i];
	  while(!(UART0->S1 & 0x40)){  // Wait for the TX buffer to be empty
		  if(*systicks - print_aux >= UART_TIMEOUT_MS * conv_factor) break;
	  }
	}

	xSemaphoreGive(uartOutBuffer_mux);
}

/**
 * @brief Stop handler as a task. Used by all stopping interruptions.
 */
void stopRestartAll(void){
	// Suspend motor movement, calculation of instructions

	// Report stop to LCD
	xSemaphoreTake(evalResults_mux, portMAX_DELAY);
	evalResults = "System suddenly stopped... restarting.";
	xSemaphoreGive(evalResults_mux);

	// Allow user to select mode again
}

/**
 * @brief Removes the move instructions from the stepperXQueue and
 * send it to the stepper X motor.
 */
void moveXStepperMotor(void){
	Stepper stepperXins;

	// Send available instruction to motors
	if(xQueueReceive(stepperXQueue, &stepperXins, 0)){
		if(*systicks - stepperAux_x >= TIME_TO_RUN_STEPPER * conv_factor){
			RunStepper(&stepperXins);
			stepperAux_x = *systicks;
		}
	}
}

/**
 * @brief Removes the move instructions from the stepperYQueue and
 * send it to the stepper Y motor.
 */
void moveYStepperMotor(void){
	Stepper stepperYins;

	// Send available instruction to motors
	if(xQueueReceive(stepperYQueue, &stepperYins, 0)){
		if(*systicks - stepperAux_y >= TIME_TO_RUN_STEPPER * conv_factor){
			RunStepper(&stepperXins);
			stepperAux_y = *systicks;
		}
	}
}

/**
 * @brief Calculate distance to move in x and y directions, in mm,
 * from the input of the joysticks.
 */
void calcStepperIns(void){
	// Blink yellow LED
	// Unsuspend corresponding task

	// Read ADC and clear flag --> CHECK LOGIC
	if(*systicks - ADCaux >= TIME_TO_READ_ADC_MS){
		ADCaux = *systicks;

		while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
			if(*systicks - ADCaux >= ADC_TIMEOUT_MS * conv_factor){
				break;
			}
		}
		// Assign ADC result
		ADCres1 = (ADC0->R[0] >> 4) - 10;
		ADCaux = *systicks;

		// Start ADC conversion in channel 4
		ADC0->SC1[0] = 0x04;

		while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
			if(*systicks - ADCaux >= ADC_TIMEOUT_MS * conv_factor){
				break;
			}
		}
		// Store result
		ADCres2 = (ADC0->R[0] >> 4) - 10;
		ADCaux = *systicks;
		// Start ADC conversion in channel 0
		ADC0->SC1[0] = 0;
	}

	SetStepperVelocity(&stepperMotors.xStepper, ADCres1);
	SetStepperVelocity(&stepperMotors.yStepper, ADCres2);

	xQueueSend(stepperXQueue, (void *) &stepperMotors.xStepper, 0);
	xQueueSend(stepperYQueue, (void *) &stepperMotors.yStepper, 0);

	// Stop yellow LED blink
	// Suspend corresponding task
}

/**
 * @brief Display the contents of uartInBuffer to the LCD.
 * Acquire uartOutBuffer_mux, send data to LCD,
 * release the mutex.
 *
 */
void displayResults(void){
	xSemaphoreTake(uartInBuffer_mux, portMAX_DELAY);

	xSemaphoreGive(uartInBuffer_mux);
}

/**
 * @brief
 */
void finishRestart(void){

}

/************************************************
 *                 LED control
 ***********************************************/

/**
 * LED color meanings
 *
 * - Yellow blinking: receiving joystick input and adding instructions for steppers.
 * - Blue blinking: displaying results to LCD.
 * - Green blinking: user selecting mode.
 * -
 * - Red constant: Error, terminating and restarting.
 */

/**
 * @brief Blink the builtinLED red color one time by toggling its state. Use of mutex included
 * to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be preempted, causing
 * the mutex to be held for longer, and the toggle to be done later than expected.
 */
void blinkRedLED(void){
	xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
	RedToggle();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	RedToggle();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	xSemaphoreGive(builtinLED_mux);
}

/**
 * @brief Blink the builtinLED yellow color one time by toggling its state. Use of mutex included
 * to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be preempted, causing
 * the mutex to be held for longer, and the toggle to be done later than expected.
 */
void blinkYellowLED(void){
	xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
	YellowToggle();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	YellowToggle();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	xSemaphoreGive(builtinLED_mux);
}

/**
 * @brief Blink the builtinLED blue color one time by toggling its state. Use of mutex included
 * to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be preempted, causing
 * the mutex to be held for longer, and the toggle to be done later than expected.
 */
void blinkBlueLED(void){
	xSemaphoreTake(builtinLED_mux, 50 / portTICK_PERIOD_MS);
	BlueToggle();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	BlueToggle();
	vTaskDelay(100 / portTICK_PERIOD_MS);
	xSemaphoreGive(builtinLED_mux);
}

/**
 * @brief Blink the builtinLED green color one time by toggling its state. Use of mutex included
 * to avoid color mixing.
 *
 * Note that the blink might take longer than expected, as the task might be preempted, causing
 * the mutex to be held for longer, and the toggle to be done later than expected.
 */
void blinkGreenLED(void){
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
// Keypad input interrupt

// Keypad input interrupt handler -->


// Receive inspection error

// Receive inspection error handler


// Receive evaluation results

// Receive evaluation results handler --> display on LCD, restart to main menu, blink LED


// CNC collision

// CNC collision handler --> stop motors (disable moveStepperMotor task),


/************************************************
 *         		      Main
 ***********************************************/
int main(void) {
	//////////  Hardware modules init  //////////
	stepperMotors = InitAll();
	systicks = GetSysTicks();
	// configure LED missing

    //////////  Configuration of tasks //////////


    //////////    Mutexes and queues   //////////
    evalResults_mux = SemaphoreCreateMutex();
    uartOutBuffer_mux = SemaphoreCreateMutex();
    uartInBuffer_mux = SemaphoreCreateMutex();
    LED_mux = SemaphoreCreateMutex();

    stepperXQueue = xQueueCreate(stepperQueue_maxLen, sizeof(Stepper));
    stepperYQueue = xQueueCreate(stepperQueue_maxLen, sizeof(Stepper));


    //////////      Interruptions      //////////



    //////////      Start scheduler      //////////
    vTaskStartScheduler();
    for( ;; );

    return 0;
}
