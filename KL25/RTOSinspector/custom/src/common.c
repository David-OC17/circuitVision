#include "../include/common.h"

/************************************************
 *               Config functions
 ***********************************************/

Steppers InitAll(void) {
  MCG_Config();
  ClkInitIt(CLK_TENS_MICROS);

  UART_Config();
  I2C_Config();
  ADC_Config();
  Keypad_Config();
  RGB_Config();
  LCD8_Config();

  return Steppers_Config(); // Object to access the stepper motors
}

void ADC_Config(void) {
  SIM->SCGC5 |= 0x2000;    // Enable PORTE clock
  SIM->SCGC6 |= 0x8000000; // ADC0 clock
  PORTE->PCR[20] = 0x000;  // Configure pin PORTE 20 as analog
  PORTE->PCR[21] = 0x000;  // Configure pin PORTE 21 as analog

  ADC0->SC2 &= ~0x40; // Software trigger
  ADC0->CFG1 = 0x40 | 0x04 | 0x00 | 0x10;

  // Start ADC conversion in channel 0
  ADC0->SC1[0] = 0;
}

void UART_Config(void) {
  // Clocks
  SIM->SOPT2 |= (1 << 26); // Specify clock  --> Uses the same as clock core (48
                           // Mhz currently)
  SIM->SCGC4 |= 0x400;     // Enable clock for UART0
  // Baud Rate
  UART0->BDH = 0x01;
  UART0->BDL = 0x38; // Baud Rate = 9600
  // Configurations
  UART0->C1 |= 0x80; // UART RX and TX in different pins (normal operation)
  UART0->C2 |= 0x08; // Enable UART Transmitter
  UART0->C4 |= 0x0F; // Enable oversampling to 16
  // Ports
  SIM->SCGC5 |= 0x200;    // Enable clock for PORTA
  PORTA->PCR[2] = 0x0200; // Make PTA2 UART0 Pin
}

void I2C_Config(void) {
  SIM->SCGC4 |= 0x40;  // Enable I2C0 clock
  SIM->SCGC5 |= 0x800; // Enable clock port C
  I2C0->C1 = 0x00;     // Disable I2C0 peripheral to configure

  // Need to reach a frequency close to 100 khz
  I2C0->F |= (I2C_F_MULT(0x00) | I2C_F_ICR(0x14));

  I2C0->C1 |= 0X80;              // Enable I2C module
  PORTC->PCR[8] = 0x0600 | 0x03; // Enable SDA for this pin and enable pullup
  PORTC->PCR[9] = 0x0600 | 0x03; // Enable SCL for this pin and enable pullup
}

Steppers Steppers_Config(void) {
  // Configure motor X pins
  uint32_t stp_pin = X_MOTOR_STP_PIN;
  PORT_Type *stp_port = PORTC;
  uint32_t dir_pin = X_MOTOR_DIR_PIN;
  PORT_Type *dir_port = PORTC;
  uint32_t enable_pin = X_MOTOR_ENABLE_PIN;
  PORT_Type *enable_port = PORTC;

  stpX = CreateStepper(stp_pin, stp_port, dir_pin, dir_port, enable_pin,
                       enable_port);

  // Configure motor Y pins
  stp_pin = Y_MOTOR_STP_PIN;
  stp_port = PORTC;
  dir_pin = Y_MOTOR_DIR_PIN;
  dir_port = PORTC;
  enable_pin = Y_MOTOR_ENABLE_PIN;
  enable_port = PORTC;

  stpY = CreateStepper(stp_pin, stp_port, dir_pin, dir_port, enable_pin,
                       enable_port);

  // Configure motor Z pins
  stp_pin = Z_MOTOR_STP_PIN ;
  stp_port = PORTA;
  dir_pin = Z_MOTOR_DIR_PIN;
  dir_port = PORTD;
  enable_pin = Z_MOTOR_ENABLE_PIN;
  enable_port = PORTA;

  stpZ = CreateStepper(stp_pin, stp_port, dir_pin, dir_port, enable_pin,
                       enable_port);

  StepperInit(&stpX);
  StepperInit(&stpY);
  StepperInit(&stpZ);

  Steppers stepperMotors = {stpX, stpY, stpZ}; 

  return stepperMotors;
}

void Keypad_Config(void) {
  SIM->SCGC5 |= 0x0800;  /* enable clock to Port C */
  PORTC->PCR[0] = 0x103; /* PTD0, GPIO, enable pullup*/
  PORTC->PCR[1] = 0x103; /* PTD1, GPIO, enable pullup*/
  PORTC->PCR[2] = 0x103; /* PTD2, GPIO, enable pullup*/
  PORTC->PCR[3] = 0x103; /* PTD3, GPIO, enable pullup*/
  PORTC->PCR[4] = 0x103; /* PTD4, GPIO, enable pullup*/
  PORTC->PCR[5] = 0x103; /* PTD5, GPIO, enable pullup*/
  PORTC->PCR[6] = 0x103; /* PTD6, GPIO, enable pullup*/
  PORTC->PCR[7] = 0x103; /* PTD7, GPIO, enable pullup*/
  PTC->PDDR = 0x0F;      /* make PTD7-0 as input pins */
}

void LCD8_Config(void) {
  // Configure GPIOs
  ConfigureGPIO(LCD8_defaultConfig.enable_port, LCD8_defaultConfig.enable_pin);
  ConfigureGPIO(LCD8_defaultConfig.rs_port, LCD8_defaultConfig.rs_pin);
  ConfigureGPIO(LCD8_defaultConfig.rw_port, LCD8_defaultConfig.rw_pin);
  // Data pins
  ConfigureGPIO(LCD8_defaultConfig.d0_port, LCD8_defaultConfig.d0_pin);
  ConfigureGPIO(LCD8_defaultConfig.d1_port, LCD8_defaultConfig.d1_pin);
  ConfigureGPIO(LCD8_defaultConfig.d2_port, LCD8_defaultConfig.d2_pin);
  ConfigureGPIO(LCD8_defaultConfig.d3_port, LCD8_defaultConfig.d3_pin);
  ConfigureGPIO(LCD8_defaultConfig.d4_port, LCD8_defaultConfig.d4_pin);
  ConfigureGPIO(LCD8_defaultConfig.d5_port, LCD8_defaultConfig.d5_pin);
  ConfigureGPIO(LCD8_defaultConfig.d6_port, LCD8_defaultConfig.d6_pin);
  ConfigureGPIO(LCD8_defaultConfig.d7_port, LCD8_defaultConfig.d7_pin);
  // Get the GPIOS with the port
  SetGPIOs();

  delayMs(20);
  LCD8_CommandNoWait(0x30);
  delayMs(5);
  LCD8_CommandNoWait(0x30);
  delayMs(1);
  LCD8_CommandNoWait(0x30);
  /* set 8-bit data, 2-line, 5x7 font */
  LCD8_Command(0x38);
  /* move cursor right */
  LCD8_Command(0x06);
  /* clear screen, move cursor to home */
  LCD8_Command(0x01);
  /* turn on display, cursor blinking */
  LCD8_Command(0x0F);
}

void RGB_Config(void) {
  // LED config
  SIM->SCGC5 |= 0x400;  // Enable clock port B
  SIM->SCGC5 |= 0x1000; // Enable clock port D

  // Set pins as GPIOS
  PORTB->PCR[19] = 0x100;
  PORTB->PCR[18] = 0x100;
  PORTD->PCR[1] = 0x100;

  GPIOB->PDDR |= GREEN; // Enable (bit) pin 19 from GPIOB as output (Green)
  GPIOD->PDDR |= BLUE;  // Enable (bit) pin 1 from GPIOD as output (Blue)
  GPIOB->PDDR |= RED;   // Enable (bit) pin 18 from GPIOB as output (Red)

  RGB_Off(); // Set RGB Off after init
}

void ErrorHandler(void) {
  RGB_Off(); // Turn off RGB LED
  RedOn();   // Turn on red light
  while (1) {
  } // Infinite loop
}

/************************************************
 *                Delay functions
 ***********************************************/

void delayMs(uint32_t delay) {
  // Manage different conversions according to time unit
  if (freq == CLK_MILLIS)
    time_delay = delay;
  else if (freq == CLK_HUNDRED_MICROS)
    time_delay = delay * 10;
  else if (freq == CLK_TENS_MICROS)
    time_delay = delay * 100;
  else
    time_delay = delay << 10; // Multiplication cannot catch up at high speeds

  // Do nothing until time_delay goes back to 0
  while (time_delay != 0) {
  }
}

void delayUs(uint32_t delay) {
  if (freq == CLK_HUNDRED_MICROS)
    time_delay = delay / 100;
  else if (freq == CLK_TENS_MICROS)
    time_delay = delay / 10;
  else
    time_delay = delay;

  // Do nothing until time_delay goes back to 0
  while (time_delay != 0) {
  }
}
