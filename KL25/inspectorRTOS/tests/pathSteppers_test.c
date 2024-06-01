/**
 * Annotations:
 *
 * loadRowIns does load the instructions to the appropriate queue, exits correctly.
 * loadAllIns does load the instructions correctly.
 * Both moveXStepperMotor and moveYStepperMotor consume the elements from the queue correctly, exits with success.
 * moveXStepperMotor working correctly.
 *
 * PENDING:
 * Both moveStepper tasks working, but Y stepper ends sooner than expected (is not waiting the desired amount of time to take the next instruction)
 * Sometimes, the instructions that move the motors seem to overlap, making instructions "disappear"
 * Test change in direction of motors.
 */

#include "mainSystem.h"

const int fiveSeconds = 5000;

void motorTestSupervisor(void *pvParameters) {
	for(;;){
//		// Start loading instructions into the queue
//		vTaskResume(loadRowIns_handle);
		vTaskResume(loadAllIns_handle);

////		// Allow the motors to move
		vTaskResume(moveXStepperMotor_handle);
		vTaskResume(moveYStepperMotor_handle);

//		vTaskSuspend(moveXStepperMotor_handle);
//		vTaskSuspend(moveYStepperMotor_handle);

		vTaskSuspend(NULL);
	}
}

TaskHandle_t motorTestSupervisor_handle = NULL;

//void moveOneMotor(void *pvParameters){
//	RedOn();
//	for(;;)
//	RunStepper(&stepperMotors.xStepper);
//}
//
//TaskHandle_t handleThisNuts = NULL;
//
//xTaskCreate(moveOneMotor, "supervisor", complexTasks_stackDepth,
//				  NULL, 10, &handleThisNuts);


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

	xTaskCreate(loadAllIns, "loadAllInsTask", complexTasks_stackDepth, NULL,
			  moveStepperMotor_priority, &loadAllIns_handle);
	vTaskSuspend(loadAllIns_handle);
//
//	//////////////////
//
	xTaskCreate(motorTestSupervisor, "supervisor", complexTasks_stackDepth,
					  NULL, 10, &motorTestSupervisor_handle);

	stepperXQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));
	stepperYQueue = xQueueCreate(stepperQueue_maxLen, sizeof(uint16_t));

  //////////      Start scheduler      //////////
  vTaskStartScheduler();

  for (;;);
}
