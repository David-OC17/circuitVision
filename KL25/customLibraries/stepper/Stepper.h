#ifndef STEPPER_H
#define STEPPER_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"
#include "Headers/Clocks.h"
//////////// MACROS ////////////
#define CLOCKWISE 1
#define COUNTERCLOCKWISE 0

/////////// Variables //////////
typedef struct{
	uint8_t stp_pin;
	uint8_t dir_pin;
	uint8_t enable_pin;
	PORT_Type *stp_port;
	PORT_Type *dir_port;
	PORT_Type *enable_port;

	uint8_t direction;
	uint32_t step_delay;		// In microseconds
	uint16_t steps_rev;   		// 200 steps per revolution when using full step
} Stepper;

// This resolution is given by the values of MS1, MS2 and MS3 respectively
enum StepResolution{
	FULL_STEP = 0x00,
	HALF_STEP = 0x04,
	QUARTER_STEP = 0x02,
	EIGHTH_STEP  = 0x06,
	SIXTEENTH_STEP = 0x07
};

////////// Functions  //////////
Stepper CreateStepper(uint32_t, PORT_Type*, uint32_t, PORT_Type*, uint32_t, PORT_Type*);
void SetStepperSettings(Stepper*, uint8_t, uint32_t, uint16_t);
void SetStepperDirection(Stepper*, uint8_t);
void SetStepperDelay(Stepper*, uint32_t);
void StepperInit(Stepper*);
void GPIO_Configure(PORT_Type*, uint32_t);
void PortConfigure(PORT_Type*);

void RunStepper(Stepper*);
void SetPinHigh(GPIO_Type*, uint32_t);
void SetPinLow(GPIO_Type*, uint32_t);

#endif /*STEPPER_H*/
