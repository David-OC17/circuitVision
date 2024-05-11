#include "main.h"

// Time aux
uint64_t blink_aux = 0, print_aux = 0, adc_aux = 0;
// Systick pointer
const uint64_t *systicks;

// UART buffer
char uart_c[50];
// ADC readings
uint16_t adc_res = 0, adc_res2 = 0;

int main(void){
	// Config functions
	ClkInitIt(CLK_MILLIS);
	UART_Config();
	ADC_Config();
	I2C_Config();
	// Init RGB led
	RGB_Init();
	// Set systick pointer
	systicks = GetSysTicks();

	GreenOn();
	delayMs(2000);
	GreenOff();

	// Init LCD
	LCD_DefaultInit(I2C0, systicks);

	// Start ADC conversion in channel 0
	ADC0->SC1[0] = 0;

	// Main loop
	while(1){
		// Blink led
		if(*systicks - blink_aux >= TIME_TO_BLINK_MS * conv_factor){
			PurpleToggle();
			blink_aux = *systicks;
		}

		// Read ADC and clear flag
		if(*systicks - adc_aux >= TIME_TO_READ_ADC_MS * conv_factor){
			adc_aux = *systicks;

			while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
				if(*systicks - adc_aux >= ADC_TIMEOUT_MS * conv_factor){
					break;
				}
			}
			// Assign ADC result
			adc_res = ADC0->R[0];
			adc_aux = *systicks;

			// Start ADC conversion in channel 4
			ADC0->SC1[0] = 0x04;

			while(!(ADC0->SC1[0] & 0x80)) {   // Wait for end of conversion flag
				if(*systicks - adc_aux >= ADC_TIMEOUT_MS * conv_factor){
					break;
				}
			}
			// Store result
			adc_res2 = ADC0->R[0];
			adc_aux = *systicks;
			// Start ADC conversion in channel 0
			ADC0->SC1[0] = 0;
		}

		// Print in UART0
		if(*systicks - print_aux >= TIME_TO_PRINT_MS * conv_factor){
			// Assign values to the buffer
			snprintf(uart_c, sizeof(uart_c), "X: %d, Y: %d\n", adc_res, adc_res2);
			print_aux = *systicks;

			for(uint8_t i=0; i < strlen(uart_c); i++){
				UART0->D = uart_c[i];
				while(!(UART0->S1 & 0x40)){  // Wait for the TX buffer to be empty
					if(*systicks - print_aux >= UART_TIMEOUT_MS * conv_factor){
						break;
					}
				}
			}
			print_aux = *systicks;
		}
	}
	return 0;
}

// Configure ADC
void ADC_Config(void){
	SIM->SCGC5 |= 0x2000;       // Enable PORTE clock
	SIM->SCGC6 |= 0x8000000;    // ADC0 clock
	PORTE->PCR[20] = 0x000;     // Configure pin PORTE 20 as analog
	PORTE->PCR[21] = 0x000;     // Configure pin PORTE 21 as analog

	ADC0->SC2 &= ~0x40;      // Software trigger
	ADC0->CFG1 = 0x40 | 0x04 | 0x00 | 0x10;
}

void UART_Config(void){
	// Clocks
	SIM->SCGC4 |= 0x400;	   // Enable clock for UART0
	SIM->SOPT2 |= 0x4000000;   // Specify clock
	// Baud Rate
	UART0->BDH = 0x00;
	UART0->BDL = 0x0C;         // Baud Rate = 115200
	// Configurations
	UART0->C1 |= 0x80;  	   // UART RX and TX in different pins (normal operation)
	UART0->C2 |= 0x08;		   // Enable UART Transmitter
	UART0->C4 |= 0x0F;		   // Enable oversampling to 16
	// Ports
	SIM->SCGC5 |= 0x200;       // Enable clock for PORTA
	PORTA->PCR[1] = 0x0200;    // Make PTA1 UART0 Pin
	PORTA->PCR[2] = 0x0200;    // Make PTA2 UART0 Pin
}

// Configure I2C
void I2C_Config(void){
	SIM->SCGC4 |= 0x40;   // Enable I2C0 clock
	SIM->SCGC5 |= 0x800;  // Enable clock port C
	I2C0->C1 = 0x00;      // Disable I2C0 peripheral to configure

	// Need to reach a frequency close to 100 khz
	// My bus clock value: 5,492,500 Hz
	I2C0->F = 0x18;

	I2C0->C1 |= 0X80;         // Enable I2C module
	PORTC->PCR[8] = 0x0600 | 0x03;   // Enable SDA for this pin and enable pullup
	PORTC->PCR[9] = 0x0600 | 0x03;   // Enable SCL for this pin and enable pullup
}
