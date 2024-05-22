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
#include <stdbool.h>
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
 *         Global variables and queues
 ***********************************************/
static char operationMode; // Default to OnePCB
static char keyboardInput;

struct moveInstruction{
	bool direction; // true:x, false:y
	bool velocity; // true:fast, false:slow
	unsigned int distance_mm;
};

static char *evalResult; // Format: XX_XXX_XXX_XXX...
static int inputReceived = 0; // 0:not received, 1:received

// Other useful constants and variables
const TickType_t delay500ms = 500 / portTICK_PERIOD_MS; // Delay of 500ms

// Stepper motor instruction queue
const int stepInsQueue_maxLen = 6;
static QueueHandle_t stepperInsQueue;

/************************************************
 *         		 Task functions
 ***********************************************/
void userSelectMode(void *evalResults_mux){
	// Write menu to LCD print variable
	xSemaphoreTake(evalResults_mux, portMAX_DELAY);
	evalResults = "Inspection modes: O(one PCB), R(row of PCBs)";
	xSemaphoreGive(evalResults_mux);
	vTaskDelay( 500 / portTICK_RATE_MS ); // Let the LCD task run

	// Wait for the user to type the mode

	// Sends mode char via UART to Raspberry Pi
}

void monitorKeyboard(void){

}

// Stop handler as a task (used by all stopping interruptions)
void stopRestartAll(void){
	// Suspend motor movement, calculation of instructions

	// Report stop to LCD
	xSemaphoreTake(evalResults_mux, portMAX_DELAY);
	evalResults = "System suddenly stopped... restarting.";
	xSemaphoreGive(evalResults_mux);

	// Allow user to select mode again
}

void moveStepperMotor(void){
	moveInstruction currentIns;

	// Send available instruction to motors
	if(xQueueReceive(stepperInsQueue, (void *) &currentIns, 0)){
		// MISSING LOGIC
	}
}

// Calculate distance to move in x and y directions, in mm
void calcStepperIns(void){
	moveInstruction nextIns;

	// Calculate the next instruction

	// Writes to instruction queue
	xQueueSend(stepperInsQueue, (void *) &nextInst, 0);
}

void displayResults(void){

}

void finishRestart(void){

}

/************************************************
 *          Interruptions and handlers
 ***********************************************/
// Keyboard input interrupt

// Keyboard input interrupt handler -->


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
//    /* Init board hardware. */
//    BOARD_InitBootPins();
//    BOARD_InitBootClocks();
//    BOARD_InitBootPeripherals();
//#ifndef BOARD_INIT_DEBUG_CONSOLE_PERIPHERAL
//    /* Init FSL debug console. */
//    BOARD_InitDebugConsole();
//#endif

    //////////  Configuration of tasks //////////


    //////////    Mutexes and queues   //////////
    SemaphoreHandle_t evalResults_mux = SemaphoreCreateMutex();

    stepperInsQueue = xQueueCreate(stepInsQueue_maxLen, sizeof(moveInstruction));


    //////////      Interruptions      //////////


    return 0;
}
