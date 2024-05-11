#include "Headers/Stepper.h"

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
	SetStepperDelay(step_struct, 1200);
	// Set default direction clockwise
	SetStepperDirection(step_struct, CLOCKWISE);
}

// Put this function inside a LOOP to move constantly (note that systicks should be less than millis)
void RunStepper(Stepper* stepper){
	GPIO_Type* stp_gpio = IdentifyGPIO(stepper->stp_port);

	SetPinHigh(stp_gpio, ToBinaryGPIO(stepper->stp_pin));
	delayUs(stepper->step_delay);
	SetPinLow(stp_gpio, ToBinaryGPIO(stepper->stp_pin));
	delayUs(stepper->step_delay);
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
