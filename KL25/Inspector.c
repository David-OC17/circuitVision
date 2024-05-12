#include "main.h"

static void ADC_Config(void);
static void UART_Config(void);
static void I2C_Config(void);
static void SteppersInit(void);
static void InitAll(void);
static void ErrorHandler(void);

// Time aux
uint64_t blink_aux = 0, stepper_aux = 0, adc_aux = 0, print_aux = 0;
// Store ADC results
uint16_t adc_res = 0, adc_res2 = 0;
// Systick pointer
const uint64_t *systicks;
// UART buffer
char uart_c[50];
// Stepper instances
Stepper stpX, stpY, stpZ;

int main(void){
	// Init peripherals
	InitAll();
	// Set systick pointer
	systicks = GetSysTicks();
	// Main loop
	while(1){
		// Run stepper
		if(*systicks - stepper_aux >= TIME_TO_RUN_STEPPER * conv_factor){
			RunStepper(&stpX);
			stepper_aux = *systicks;
		}

		// Blink led
		if(*systicks - blink_aux >= TIME_TO_BLINK_MS * conv_factor){
			YellowToggle();
			blink_aux = *systicks;
		}

		// Read ADC and clear flag
		if(*systicks - adc_aux >= TIME_TO_READ_ADC_MS){
			adc_aux = *systicks;

			while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
				if(*systicks - adc_aux >= ADC_TIMEOUT_MS * conv_factor){
					break;
				}
			}
			// Assign ADC result
			adc_res = (ADC0->R[0] >> 4) - 10;
			adc_aux = *systicks;

			// Start ADC conversion in channel 4
			ADC0->SC1[0] = 0x04;

			while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
				if(*systicks - adc_aux >= ADC_TIMEOUT_MS * conv_factor){
					break;
				}
			}
			// Store result
			adc_res2 = (ADC0->R[0] >> 4) - 10;
			adc_aux = *systicks;
			// Start ADC conversion in channel 0
			ADC0->SC1[0] = 0;
		}
		SetStepperVelocity(&stpX, adc_res);
		SetStepperVelocity(&stpY, adc_res2);

		// UART Print
		if(*systicks - print_aux >= TIME_TO_PRINT_MS * conv_factor){
			// Assign values to the buffer
			snprintf(uart_c, sizeof(uart_c), "X: %d, Y: %d\n", SystemCoreClock, adc_res2);

			for(uint8_t i=0; i < strlen(uart_c); i++){
				print_aux = *systicks;
				UART0->D = uart_c[i];
				while(!(UART0->S1 & 0x40)){  // Wait for the TX buffer to be empty
					if(*systicks - print_aux >= UART_TIMEOUT_MS * conv_factor)
						break;
				}
			}
			print_aux = *systicks;
		}
	}
	return 0;
}

void InitAll(void){
	MCG_Config();
	ClkInitIt(CLK_TENS_MICROS);

	UART_Config();	  // UART Config
	I2C_Config();	  // I2C Config
	ADC_Config();	  // ADC Config

	RGB_Init();	      // Init RGB led
	SteppersInit();   // Init steppers
	if (LCD_DefaultInit(I2C0, systicks) == ERROR)
		ErrorHandler();
}

void ADC_Config(void){
	SIM->SCGC5 |= 0x2000;       // Enable PORTE clock
	SIM->SCGC6 |= 0x8000000;    // ADC0 clock
	PORTE->PCR[20] = 0x000;     // Configure pin PORTE 20 as analog
	PORTE->PCR[21] = 0x000;     // Configure pin PORTE 21 as analog

	ADC0->SC2 &= ~0x40;      // Software trigger
	ADC0->CFG1 = 0x40 | 0x04 | 0x00 | 0x10;

	// Start ADC conversion in channel 0
	ADC0->SC1[0] = 0;
}

void UART_Config(void){
	// Clocks
	SIM->SOPT2 |= (1 << 26);   // Specify clock  --> Uses the same as clock core (48 Mhz currently)
	SIM->SCGC4 |= 0x400;	   // Enable clock for UART0
	// Baud Rate
	UART0->BDH = 0x01;
	UART0->BDL = 0x38;         // Baud Rate = 9600
	// Configurations
	UART0->C1 |= 0x80;  	   // UART RX and TX in different pins (normal operation)
	UART0->C2 |= 0x08;		   // Enable UART Transmitter
	UART0->C4 |= 0x0F;		   // Enable oversampling to 16
	// Ports
	SIM->SCGC5 |= 0x200;       // Enable clock for PORTA
	PORTA->PCR[2] = 0x0200;    // Make PTA2 UART0 Pin
}

// Configure I2C
void I2C_Config(void){
	SIM->SCGC4 |= 0x40;   // Enable I2C0 clock
	SIM->SCGC5 |= 0x800;  // Enable clock port C
	I2C0->C1 = 0x00;      // Disable I2C0 peripheral to configure

	// Need to reach a frequency close to 100 khz
	I2C0->F |= (I2C_F_MULT(0x00) | I2C_F_ICR(0x14));

	I2C0->C1 |= 0X80;                // Enable I2C module
	PORTC->PCR[8] = 0x0600 | 0x03;   // Enable SDA for this pin and enable pullup
	PORTC->PCR[9] = 0x0600 | 0x03;   // Enable SCL for this pin and enable pullup
}

void SteppersInit(void){
	// Configure motor X pins
	uint32_t stp_pin = 0;
	PORT_Type* stp_port = PORTC;
	uint32_t dir_pin = 7;
	PORT_Type* dir_port = PORTC;
	uint32_t enable_pin = 3;
	PORT_Type* enable_port = PORTC;
	// Create motor instance (step pin = C7, dir pin = C0)
	stpX = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
								 enable_pin, enable_port);
	// Configure motor Y pins
	stp_pin = 5;
	stp_port = PORTC;
	dir_pin = 4;
	dir_port = PORTC;
	enable_pin = 6;
	enable_port = PORTC;
	// Create instance
	stpY = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
									 enable_pin, enable_port);
	// Configure motor Z pins
	stp_pin = 12;
	stp_port = PORTA;
	dir_pin = 4;
	dir_port = PORTD;
	enable_pin = 4;
	enable_port = PORTA;
	// Create instance
	stpZ = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
									 enable_pin, enable_port);
	// Init steppers
	StepperInit(&stpX);
	StepperInit(&stpY);
	StepperInit(&stpZ);
}

void ErrorHandler(void){
	RGB_Off();  // Turn off RGB LED
	RedOn();    // Turn on red light
	while(1){}  // Infinite loop

}
