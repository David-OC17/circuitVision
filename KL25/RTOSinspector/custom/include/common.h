#ifndef INSPECTOR_COMMON_H
#define INSPECTOR_COMMON_H

#include "../../CMSIS/MKL25Z4.h"
#include <stdint.h>
#include "../include/stepper.h"

#define TIME_TO_RUN_STEPPER 0
#define TIME_TO_PRINT_MS 500
#define TIME_TO_READ_ADC_MS 50
#define UART_TIMEOUT_MS 1000
#define ADC_TIMEOUT_MS 500

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

typedef struct {
  Stepper xStepper;
  Stepper yStepper;
  Stepper zStepper;
} Steppers;

/************************************************
 *               Config functions
 ***********************************************/
static Steppers InitAll(void);

static void ADC_Config(void);
static void UART_Config(void);
static void I2C_Config(void);
static void Steppers_Config(void);
static void Keypad_Config(void);
static void LCD8_Config(void);
static void RGB_Config(void);
static void ErrorHandler(void);

/************************************************
 *                Delay functions
 ***********************************************/

void delayMs(uint32_t);
void delayUs(uint32_t);

#endif // INSPECTOR_COMMON_H
