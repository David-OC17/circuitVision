#include "Headers/LCD_8BIT.h"

// Static functions
static GPIO_Type* IdentifyGPIO(PORT_Type *);
static uint32_t ToBinaryGPIO(uint32_t);
static void ConfigurePort(PORT_Type *);
static void ConfigureGPIO(PORT_Type *, uint32_t);
static void SetGPIOs(void);

// Variables
GPIO_Type *d0_gpio, *d1_gpio, *d2_gpio, *d3_gpio,
		   *d4_gpio, *d5_gpio, *d6_gpio, *d7_gpio;

GPIO_Type *enable_gpio, *rw_gpio, *rs_gpio;

// Default config struct
LCD8_Struct LCD8_defaultConfig = {
	// Ports
	.d0_port = PORTD, .d1_port = PORTD, .d2_port = PORTD, .d3_port = PORTD,
	.d4_port = PORTD, .d5_port = PORTD, .d6_port = PORTD, .d7_port = PORTD,
	.enable_port = PORTA, .rs_port = PORTA, .rw_port = PORTA,
	// Pins
	.d0_pin = 0, .d1_pin = 1, .d2_pin = 2, .d3_pin = 3,
	.d4_pin = 4, .d5_pin = 5, .d6_pin = 6, .d7_pin = 7,
	.enable_pin = 5, .rs_pin = 2, .rw_pin = 4
};

// LCD8 initialization
void LCD8_Init(void){
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

void LCD8_Command(unsigned char command){
	LCD8_Ready();
	rs_gpio->PCOR |= RS;
	rw_gpio->PCOR |= RW;
	d0_gpio->PDOR = command;
	enable_gpio->PSOR = EN;
	delayMs(0);
	enable_gpio->PCOR = EN;
}

void LCD8_CommandNoWait(unsigned char command){
	rs_gpio->PCOR |= RS;
	rw_gpio->PCOR |= RW;
	d0_gpio->PDOR = command;
	enable_gpio->PSOR = EN;
	delayMs(0);
	enable_gpio->PCOR = EN;
}

void LCD8_Data(unsigned char data){
	LCD8_Ready();
	rs_gpio->PCOR |= RS;
	rw_gpio->PCOR |= RW;
	d0_gpio->PDOR = data;
	enable_gpio->PSOR = EN;
	delayMs(0);
	enable_gpio->PCOR |= EN;
}

void LCD8_Ready(void){
	char status;
	d0_gpio->PDDR = 0;
	rs_gpio->PCOR = RS;
	rw_gpio->PSOR = RW;
	do{
		enable_gpio->PSOR = EN; // Raise En
		delayMs(0);
		status = d0_gpio->PDIR;
		enable_gpio->PCOR = EN;
		delayMs(0);
	}while(status & 0x80);   // Keep blocking while LCD8 is busy

	rw_gpio->PCOR = RW;
	d0_gpio->PDDR = 0xFF;
}

///// HIGHER LEVEL COMMANDS /////

// Clear the LCD and set the cursor to home
void LCD8_Clear(void){
	LCD8_Command(LCD8_CLEAR_COMMAND);
}

// Set LCD cursor to No blink
void LCD8_NoBlink(void){
	LCD8_Command(LCD8_NO_BLINK);
}

// Set cursor to blink
void LCD8_Blink(void){
	LCD8_Command(LCD8_BLINK_HOME);
}

void LCD8_Write(char *data)
{
	while(*data != '\0') // Loop through all the string, until null-terminator is found
	{
		LCD8_Ready();      // wait until LCD is ready
		rs_gpio->PSOR = RS;   // RS = 1, R/W = 0
		rw_gpio->PCOR = RW;   // readwrite
		d0_gpio->PDOR = *data;
		enable_gpio->PSOR = EN;   // pulse E
		delayMs(0);
		enable_gpio->PCOR = EN;
		data++;			 // Increment data pointer
	}
}

/////////////////////////////////


static void SetGPIOs(void){
	// Data pins
	d0_gpio = IdentifyGPIO(LCD8_defaultConfig.d0_port);
	d1_gpio = IdentifyGPIO(LCD8_defaultConfig.d1_port);
	d2_gpio = IdentifyGPIO(LCD8_defaultConfig.d2_port);
	d3_gpio = IdentifyGPIO(LCD8_defaultConfig.d3_port);
	d4_gpio = IdentifyGPIO(LCD8_defaultConfig.d4_port);
	d5_gpio = IdentifyGPIO(LCD8_defaultConfig.d5_port);
	d6_gpio = IdentifyGPIO(LCD8_defaultConfig.d6_port);
	d7_gpio = IdentifyGPIO(LCD8_defaultConfig.d7_port);
	// Additional pins
	enable_gpio = IdentifyGPIO(LCD8_defaultConfig.enable_port);
	rw_gpio = IdentifyGPIO(LCD8_defaultConfig.rw_port);
	rs_gpio = IdentifyGPIO(LCD8_defaultConfig.rs_port);
}

static void ConfigureGPIO(PORT_Type *port, uint32_t pin){
	ConfigurePort(port);
	port->PCR[pin] = 0x100;   // Set as GPIO

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

static void ConfigurePort(PORT_Type *port){
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

static uint32_t ToBinaryGPIO(uint32_t num){
	return (1 << num);
}
