#include "mainSystem.h"
#include "pathSteppers_test.c"

int main_(void) {
  //////////  Hardware modules init  //////////
  stepperMotors = InitAll();
  // configure LED missing --> configure for external LEDs

  //////////  Configuration of tasks //////////
  xTaskCreate(userSelectMode, "userSelectModeTask", complexTasks_stackDepth, NULL,
              userSelectMode_priority, &userSelectMode_handle);

  xTaskCreate(UARTsend, "UARTsendTask", simpleTasks_stackDepth, NULL,
              UARTsend_priority, &UARTsend_handle);

  xTaskCreate(LCDprint, "LCDprintTask", complexTasks_stackDepth, NULL,
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
  vTaskSuspend(loadRowIns_handle);

  xTaskCreate(loadAllIns, "loadAllInsTask", simpleTasks_stackDepth, NULL,
			  errorTaskStop_priority, &errorTaskStop_handle);
  vTaskSuspend(loadAllIns_handle);

  //////////    Error handling  //////////
  xTaskCreate(errorTaskStop, "errorTaskStopTask", simpleTasks_stackDepth, NULL,
  			  moveStepperMotor_priority, &loadAllIns_handle);
  vTaskSuspend(returnToOrigin_handle);


  //////////    Mutexes and queues   //////////
  builtinLED_mux = xSemaphoreCreateMutex();
  evalResults_mux = xSemaphoreCreateBinary();
  uartOutBuffer_mux = xSemaphoreCreateBinary();
  uartInBuffer_mux = xSemaphoreCreateBinary();
  LCDBuffer_mux = xSemaphoreCreateBinary();

  stepperXQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));
  stepperYQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));

  //////////      Interruptions      //////////

  //////////      Start scheduler      //////////
  vTaskStartScheduler();
  for (;;);

  return 0;
}

/************************************************
 *         		      Main
 ***********************************************/
int main(void){
	moveSteppers_test();

	return 0;
}
