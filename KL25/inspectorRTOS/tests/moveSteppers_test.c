#include "mainSystem.h"

// TEST DESIGN INCOMPLETE: must test paths or manual operation first

void moveSteppers_test(void) {
  //////////  Hardware modules init  //////////
	stepperMotors = Steppers_Config();

	xTaskCreate(moveXStepperMotor, "moveXStepperMotorTask", complexTasks_stackDepth,
	              NULL, moveStepperMotor_priority, &moveXStepperMotor_handle);

	 xTaskCreate(moveYStepperMotor, "moveYStepperMotorTask", complexTasks_stackDepth,
	              NULL, moveStepperMotor_priority, &moveYStepperMotor_handle);


  //////////      Start scheduler      //////////
  vTaskStartScheduler();

  for (;;);
}
