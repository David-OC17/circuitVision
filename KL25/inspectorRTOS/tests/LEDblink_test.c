#include "mainSystem.h"

void blinkLED_test(void *pvParameters) {
	for(;;){
  vTaskDelay(1000);
  RGB_Off();
  vTaskDelay(1000);
  RGB_On();
	}
}

TaskHandle_t blinkLED_test_handle;

void LEDblink_test(void) {
  //////////  Hardware modules init  //////////
  RGB_Config();

  /////    Blink LED   //////////
  xTaskCreate(blinkRedLED, "blinkRedLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkRedLED_handle);

  xTaskCreate(blinkYellowLED, "blinkYellowLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkYellowLED_handle);

  xTaskCreate(blinkBlueLED, "blinkBlueLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkBlueLED_handle);

  xTaskCreate(blinkGreenLED, "blinkGreenLEDTask", blinkLEDtask_stackDepth, NULL,
              blinkLED_priority, &blinkGreenLED_handle);

//  xTaskCreate(blinkLED_test, "blinkGreenLEDTask", blinkLEDtask_stackDepth, NULL,
//                 blinkLED_priority, NULL);

  builtinLED_mux = xSemaphoreCreateMutex();


  //////////      Start scheduler      //////////
  vTaskStartScheduler();

  for (;;);
}
