#include "stepper.h"

static GPIO_Type* IdentifyGPIO(PORT_Type *);
static uint32_t ToBinaryGPIO(uint32_t);

Stepper CreateStepper(uint32_t stp_pin, PORT_Type* stp_port, uint32_t dir_pin,
					 PORT_Type* dir_port, uint32_t enable_pin, PORT_Type *enable_port){
	Stepper stepper;
	stepper.stp_pin = stp_pin;
	stepper.stp_port = stp_port;
	stepper.dir_pin = dir_pin;
	stepper.dir_port = dir_port;
	stepper.enable_pin = enable_pin;
	stepper.enable_port = enable_port;
	stepper.run = 1;
	return stepper;
}

void SetStepperSettings(Stepper* stepper, uint8_t dir,
						uint32_t step_delay, uint16_t steps_rev){
	stepper->direction = dir;
	stepper->step_delay = step_delay;
	stepper->steps_rev = steps_rev;
}

void SetStepperDirection(Stepper* stepper, uint8_t dir){
	stepper->direction = dir;
	GPIO_Type *dir_gpio = IdentifyGPIO(stepper->dir_port);
	if(dir == CLOCKWISE)
		SetPinHigh(dir_gpio, ToBinaryGPIO(stepper->dir_pin));
	else
		SetPinLow(dir_gpio, ToBinaryGPIO(stepper->dir_pin));
}

void SetStepperDelay(Stepper *stepper, uint32_t delay_micros){
	stepper->step_delay = delay_micros;
}

void StepperInit(Stepper *step_struct){
	// Configure GPIOs
	GPIO_Configure(step_struct->stp_port, step_struct->stp_pin);
	GPIO_Configure(step_struct->dir_port, step_struct->dir_pin);
	GPIO_Configure(step_struct->enable_port, step_struct->enable_pin);
	// Set default between steps delay (in microseconds)
	SetStepperDelay(step_struct, 4000);
	// Set default direction clockwise
	SetStepperDirection(step_struct, CLOCKWISE);
}

// Put this function inside a LOOP to move constantly (note that systicks should be less than millis)
void RunStepper(Stepper* stepper){

	GPIO_Type* stp_gpio = IdentifyGPIO(stepper->stp_port);

	if(stepper->run == 1){
		SetPinHigh(stp_gpio, ToBinaryGPIO(stepper->stp_pin));
		for(int i=0; i < 5000; i++);
		SetPinLow(stp_gpio, ToBinaryGPIO(stepper->stp_pin));
		for(int i=0; i < 5000; i++);
	}else{
		SetPinLow(stp_gpio, ToBinaryGPIO(stepper->stp_pin));
	}
}

/**
 * @brief This function sets the velocity of the motor given a resolution
 * Half of the resolution is set for counterclockwise and the rest for clockwise
 * */
void SetStepperVelocity(Stepper* stepper, uint8_t velocity){
	if(velocity >= (SPEED_RESOLUTION >> 1))
		SetStepperDirection(stepper, CLOCKWISE);
	else
		SetStepperDirection(stepper, COUNTERCLOCKWISE);
	velocity = (velocity >= (SPEED_RESOLUTION >> 1)? velocity - (SPEED_RESOLUTION >> 1) : velocity << 1);
	uint16_t delay_us = (MIN_US_DELAY * SPEED_RESOLUTION) / velocity;
	if(delay_us < MAX_US_DELAY){
		stepper->run = 1;
		SetStepperDelay(stepper, delay_us);
	}
	else{
		stepper->run = 0;
	}
}

/**
 * @brief A function to move certain number of millimeters in the current direction
 * */
void MoveMillimeters(Stepper *stepper, uint16_t millimeters){
	uint16_t counter = 0;  uint16_t desired_count = (millimeters / STEP_SIZE); // Check if it can handle float div
	stepper->run = 1;
	while(counter < desired_count){
		for(uint8_t i=0; i < stepper->steps_rev; i++){   // One spin
			RunStepper(stepper);
		}
	}
	stepper->run = 0;
	RunStepper(stepper);   // Turn stepper off
}

void StepperOff(Stepper* stepper){
	GPIO_Type* stp_gpio = IdentifyGPIO(stepper->stp_port);
	SetPinLow(stp_gpio, ToBinaryGPIO(stepper->stp_pin));
}

void SetPinHigh(GPIO_Type *port, uint32_t pin){
	port->PDOR |= pin;
}

void SetPinLow(GPIO_Type *port, uint32_t pin){
	port->PDOR &= ~pin;
}

static GPIO_Type* IdentifyGPIO(PORT_Type *port){
	if(port == PORTA){
		return GPIOA;
	}else if(port == PORTB){
		return GPIOB;
	}else if(port == PORTC){
		return GPIOC;
	}else if(port == PORTD){
		return GPIOD;
	}else{
		return GPIOE;
	}
}

void GPIO_Configure(PORT_Type *port, uint32_t pin){
	PortConfigure(port);
	port->PCR[pin] = 0x100;

	if(port == PORTA){
		GPIOA->PDDR |= ToBinaryGPIO(pin);    // Set as output
	}else if (port == PORTB){
		GPIOB->PDDR |= ToBinaryGPIO(pin);
	}else if (port == PORTC){
		GPIOC->PDDR |= ToBinaryGPIO(pin);
	}else if (port == PORTD){
		GPIOD->PDDR |= ToBinaryGPIO(pin);
	}else{
		GPIOE->PDDR |= ToBinaryGPIO(pin);
	}
}

void PortConfigure(PORT_Type *port){
	if(port == PORTA){
		SIM->SCGC5 |= 0x200;   // Enable clock port A
	}else if (port == PORTB){
		SIM->SCGC5 |= 0x400;   // Enable clock port B
	}else if (port == PORTC){
		SIM->SCGC5 |= 0x800;   // Enable clock port C
	}else if (port == PORTD){
		SIM->SCGC5 |= 0x1000;  // Enable clock port D
	}else{
		SIM->SCGC5 |= 0x2000;  // Enable clock port E
	}
}

static uint32_t ToBinaryGPIO(uint32_t num){
	return (1 << num);
}
