#include "common.h"

char uart_tx[50];
const TickType_t UART_timeout_ticks = 500;
TickType_t UARTtick_aux;

/************************************************
 *               Config functions
 ***********************************************/

Steppers InitAll(void) {
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
    SIM->SCGC4 |= SIM_SCGC4_UART0_MASK; // Enable clock for UART0
    SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1); // Specify clock

    // Baud Rate
	UART0->BDH = 0x00;
	UART0->BDL = 0x0C;         // Baud Rate = 115200

    // Configurations
    UART0->C1 = 0x00; // 8-bit data, no parity
    UART0->C2 |= UART_C2_TE_MASK | UART_C2_RE_MASK; // Enable transmitter and receiver
    UART0->C4 |= 0x0F;	 // Enable oversampling to 16

    // Ports
    SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK; // Enable clock for PORTA
    PORTA->PCR[1] = PORT_PCR_MUX(2); // Set PTA1 to UART0_RX
    PORTA->PCR[2] = PORT_PCR_MUX(2); // Set PTA2 to UART0_TX
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
  Steppers tempSteppers;

  // Configure motor X pins
  uint32_t stp_pin = X_MOTOR_STP_PIN;
  PORT_Type *stp_port = PORTC;
  uint32_t dir_pin = X_MOTOR_DIR_PIN;
  PORT_Type *dir_port = PORTC;
  uint32_t enable_pin = X_MOTOR_ENABLE_PIN;
  PORT_Type *enable_port = PORTC;

  tempSteppers.xStepper = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
                                        enable_pin, enable_port);

  // Configure motor Y pins
  stp_pin = Y_MOTOR_STP_PIN;
  stp_port = PORTC;
  dir_pin = Y_MOTOR_DIR_PIN;
  dir_port = PORTC;
  enable_pin = Y_MOTOR_ENABLE_PIN;
  enable_port = PORTC;

  tempSteppers.yStepper = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
                                        enable_pin, enable_port);

  // Configure motor Z pins
  stp_pin = Z_MOTOR_STP_PIN;
  stp_port = PORTA;
  dir_pin = Z_MOTOR_DIR_PIN;
  dir_port = PORTD;
  enable_pin = Z_MOTOR_ENABLE_PIN;
  enable_port = PORTA;

  tempSteppers.zStepper = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
                                        enable_pin, enable_port);

  StepperInit(&tempSteppers.xStepper);
  StepperInit(&tempSteppers.yStepper);
  StepperInit(&tempSteppers.zStepper);

  return tempSteppers;
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

void sendUART(char uart_tx_send[50]){
	strcpy(uart_tx, uart_tx_send);

	for(uint8_t i=0; i < strlen(uart_tx); i++){
		UART0->D = uart_tx[i];
		while(!(UART0->S1 & 0x40)){  // Wait for the TX buffer to be empty
			if(xTaskGetTickCount() - UARTtick_aux >= UART_timeout_ticks){
				break;
			}
		}
		UARTtick_aux = xTaskGetTickCount();
		memset(&uart_tx[0], 0, sizeof(uart_tx)); // Clear
	}
}
