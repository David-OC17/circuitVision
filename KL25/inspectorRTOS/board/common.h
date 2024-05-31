#ifndef INSPECTOR_COMMON_H
#define INSPECTOR_COMMON_H

#include "MKL25Z4.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#include "LCD_8BIT.h"
#include "RGB.h"
#include "common.h"
#include "i2cdisplay.h"
#include "keypad.h"
#include "stepper.h"

//#define TIME_TO_RUN_STEPPER 0
//#define TIME_TO_PRINT_MS 500
//#define TIME_TO_READ_ADC_MS 50
//#define UART_TIMEOUT_MS 1000
//#define ADC_TIMEOUT_MS 500

#define TICKS_TO_RUN_STEPPER 0
#define TICKS_TO_PRINT_MS 500
#define TICKS_TO_READ_ADC 50
#define UART_TIMEOUT_TICKS 1000
#define ADC_TIMEOUT_TICKS 500

/************************************************
 *                 Stepper aux 
 ***********************************************/

#define X_MOTOR_STP_PIN 0
#define X_MOTOR_DIR_PIN 7
#define X_MOTOR_ENABLE_PIN 3

#define Y_MOTOR_STP_PIN 5
#define Y_MOTOR_DIR_PIN 4
#define Y_MOTOR_ENABLE_PIN 6

#define Z_MOTOR_STP_PIN 12 
#define Z_MOTOR_DIR_PIN 4
#define Z_MOTOR_ENABLE_PIN 4

/************************************************
 *               Config functions
 ***********************************************/
Steppers InitAll(void);

void ADC_Config(void);
void UART_Config(void);
void I2C_Config(void);
Steppers Steppers_Config(void);
void Keypad_Config(void);

void RGB_Config(void);
void ErrorHandler(void);

void sendUART(char uart_tx[50]);

#endif // INSPECTOR_COMMON_H
