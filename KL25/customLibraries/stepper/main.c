#include "main.h"

// Time aux
uint64_t blink_aux = 0, stepper_aux = 0;
// Systick pointer
const uint64_t *systicks;

int main(void){
	// Config functions
	ClkInitIt(CLK_TENS_MICROS);
	// Init RGB led
	RGB_Init();
	// Set systick pointer
	systicks = GetSysTicks();

	// Configure motor pins
	uint32_t stp_pin = 0;
	PORT_Type* stp_port = PORTC;
	uint32_t dir_pin = 7;
	PORT_Type* dir_port = PORTC;
	uint32_t enable_pin = 3;
	PORT_Type* enable_port = PORTC;

	// Run motor (step pin = C7, dir pin = C0)
	Stepper stp1 = CreateStepper(stp_pin, stp_port, dir_pin, dir_port,
								 enable_pin, enable_port);
	StepperInit(&stp1);

	// Main loop
	while(1){
		if(*systicks - stepper_aux >= TIME_TO_RUN_STEPPER * conv_factor){
			for(uint8_t i=0; i<200; i++){
				RunStepper(&stp1);
			}
			stepper_aux = *systicks;
		}

		// Blink led
		if(*systicks - blink_aux >= TIME_TO_BLINK_MS * conv_factor){
			YellowToggle();
			blink_aux = *systicks;
		}

	}
	return 0;
}
