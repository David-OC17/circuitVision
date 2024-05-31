/**
 * Annotations:
 *
 * loaadRowIns does seem to load the instructions to the appropriate queue, exits correctly.
 * moveXStepperMotor does seem to consume the elements from the queue correctly, exits with success.
 *
 * TESTING WITH ACTUAL MOTORS NEEDED
 * stack depth had to be adjusted, memory seemed to be running out (fixed freeze in code)
 */

#include "mainSystem.h"

const int fiveSeconds = 5000;

void motorTestSupervisor(void *pvParameters) {
	for(;;){
		GreenOn();
//		// Start loading instructions into the queue
//		vTaskResume(loadRowIns_handle);
//		vTaskResume(loadAllIns_handle);
		vTaskDelay(fiveSeconds);
		GreenOff();

////		// Allow the motors to move
		uint16_t ADCval = stpMoveNone;
		// Send available instruction to motors
		if (xQueueReceive(stepperXQueue, &ADCval, 1) == pdTRUE) RedOn();

//		vTaskResume(moveXStepperMotor_handle);
//		vTaskResume(moveYStepperMotor_handle);
		vTaskDelay(fiveSeconds);
		RedOff();

//		vTaskSuspend(moveXStepperMotor_handle);
	}
}

TaskHandle_t motorTestSupervisor_handle = NULL;

void moveSteppers_test(void) {
  //////////  Hardware modules init  //////////
	RGB_Config();
	stepperMotors = Steppers_Config();

	xTaskCreate(moveXStepperMotor, "moveXStepperMotorTask", complexTasks_stackDepth,
				  NULL, moveStepperMotor_priority, &moveXStepperMotor_handle);
	vTaskSuspend(moveXStepperMotor_handle); // ONLY USED IN TEST

	xTaskCreate(moveYStepperMotor, "moveYStepperMotorTask", complexTasks_stackDepth,
				  NULL, moveStepperMotor_priority, &moveYStepperMotor_handle);
	vTaskSuspend(moveYStepperMotor_handle); // ONLY USED IN TEST

	 ///////////////////////

//	xTaskCreate(loadRowIns, "loadRowInsTask", simpleTasks_stackDepth, NULL,
//				   moveStepperMotor_priority, &loadRowIns_handle);
//	vTaskSuspend(loadRowIns_handle);

	xTaskCreate(loadAllIns, "loadAllInsTask", simpleTasks_stackDepth, NULL,
			  errorTaskStop_priority, &errorTaskStop_handle);
	vTaskSuspend(loadAllIns_handle);


	//////////////////

	xTaskCreate(motorTestSupervisor, "supervisor", complexTasks_stackDepth,
					  NULL, 10, &motorTestSupervisor_handle);

	stepperXQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));
	stepperYQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));


  //////////      Start scheduler      //////////
  vTaskStartScheduler();

  for (;;);
}
