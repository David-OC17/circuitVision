/**
 * Annotations:
 *
 */

#include "mainSystem.h"

void LCDbufferWritter(void *pvParameters) {
	for(;;){

		// Acquire the mutex for the LCDbuffer
		// Modify the buffer with the desired value
		// Give the mutex back
		// Send a notifications to LCDprint (to start transmittion and trigger change in LCD)

		GreenOn();

		if (xSemaphoreTake(LCDBuffer_mux, pdMS_TO_TICKS(1000)) == pdTRUE){
			GreenOn();
			strcpy(LCDBuffer, "Inspection modes: O(one PCB), R(row of PCBs)");
			xSemaphoreGive(LCDBuffer_mux);
		}

		RedOn();
		vTaskResume(UARTsend_handle);
		RGB_On();
		xTaskNotify(ulUARTnotificationVal, 1, eSetBits);

		vTaskSuspend(NULL);
	}
}

TaskHandle_t LCDbufferWritter_handle = NULL;


void LCDprintOnce(void *pvParameters) {
	for(;;){
		char compLCDBuffer[50];
		strcpy(compLCDBuffer, LCDBuffer);

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
		vTaskSuspend(NULL);
	}
}

TaskHandle_t LCDprintOnce_handle = NULL;


void printToLCD_test(void) {
  //////////  Hardware modules init  //////////
	RGB_Config();
	LCD8_Config();
	YellowOn();

//  xTaskCreate(LCDprint, "LCDprintTask", simpleTasks_stackDepth, NULL,
//				LCDprint_priority, &LCDprint_handle);
//  vTaskSuspendAll(LCDprint_handle);

  xTaskCreate(LCDprintOnce, "printOneLCDtask", simpleTasks_stackDepth, NULL,
  				LCDprint_priority, &LCDprintOnce_handle);
    vTaskSuspend(LCDprintOnce_handle);

    ////////////////////

  xTaskCreate(LCDbufferWritter, "LCDprintTask", simpleTasks_stackDepth, NULL,
  				10, &LCDbufferWritter_handle);

  LCDBuffer_mux = xSemaphoreCreateBinary();
  GreenOn();

  //////////      Start scheduler      //////////
  vTaskStartScheduler();

  for (;;);
}
