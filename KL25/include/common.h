#ifndef INSPECTOR_COMMON_H
#define INSPECTOR_COMMON_H

/////////// Includes ///////////
#include "../RTOSinspector/CMSIS/MKL25Z4.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>

//////////// MACROS ////////////
#define TIME_TO_RUN_STEPPER 0
#define TIME_TO_PRINT_MS 500
#define TIME_TO_READ_ADC_MS 50
#define UART_TIMEOUT_MS 1000
#define ADC_TIMEOUT_MS 500

////////// Functions  //////////
void ClockConfig(void);
void UART_Config(void);
void ADC_Config(void);

#endif // INSPECTOR_COMMON_H
