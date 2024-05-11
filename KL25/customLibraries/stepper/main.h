#ifndef MAIN_H
#define MAIN_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "stdio.h"
#include "string.h"
#include "i2cdisplay.h"
#include "Headers/Rgb.h"
#include "Headers/Clocks.h"
#include "Headers/Stepper.h"

//////////// MACROS ////////////
#define TIME_TO_RUN_STEPPER 0
#define TIME_TO_PRINT_MS    1000
#define TIME_TO_READ_ADC_MS 1000
#define UART_TIMEOUT_MS  1000
#define ADC_TIMEOUT_MS  500

////////// Functions  //////////
//void ClockConfig(void);
void UART_Config(void);
void ADC_Config(void);
void I2C_Config(void);

#endif /*MAIN_H*/
