#include "Headers/Clocks.h"

// Variables
static uint32_t freq;
static uint64_t systicks;
static uint32_t time_delay;
uint16_t conv_factor;

// Systick init
void ClkInit(uint32_t clock_freq){
	freq = clock_freq;
	ClkFreqConfig(clock_freq);
	SetConvFactor(clock_freq);
}

// Systick init with interrupt
void ClkInitIt(uint32_t clock_freq){
	freq = clock_freq;
	ClkFreqConfig(clock_freq);
	SetConvFactor(clock_freq);
	SysTick->CTRL |= 0x02;       // Enable Systick interrupt
}

// Conversion factors to use milliseconds as reference for timing
void SetConvFactor(uint32_t freq){
	if(freq == CLK_MICROS)
		conv_factor = MILLIS2MICROS;
	else if(freq == CLK_HUNDRED_MICROS)
		conv_factor = MILLIS2HUNDRED_MICROS;
	else if (freq == CLK_TENS_MICROS)
		conv_factor = MILLIS2TENS_MICROS;
	else
		conv_factor = 1;
}

// Systick callback
void SysTick_Handler(void){
	time_delay = (time_delay > 0)? time_delay-1 : 0;
	systicks++;
}

// Configure the frequency of the systick clock
void ClkFreqConfig(uint32_t clock_freq){
	SysTick->LOAD = clock_freq;  // Remember default systick freq for KL25Z is 21.9 Mhz
	SysTick->CTRL |= 0x05;     	 // Enable timer and set sysclk as source clock
}

// Get current systicks
uint64_t *GetSysTicks(void){
	return &systicks;
}

// Set delay ticks in current timing unit
void SetDelayTicks(uint32_t delay_ticks){
	time_delay = delay_ticks;
}

///// DELAY FUNCTIONS /////
void delayMs(uint32_t delay){
	// Manage different conversions according to time unit
	if(freq == CLK_MILLIS)
		time_delay = delay;
	else if (freq == CLK_HUNDRED_MICROS)
		time_delay = delay * 10;
	else if (freq == CLK_TENS_MICROS)
		time_delay = delay * 100;
	else
		time_delay = delay << 10;   // Multiplication cannot catch up at high speeds

	// Do nothing until time_delay goes back to 0
	while(time_delay != 0){
	}
}

void delayUs(uint32_t delay){
	if (freq == CLK_MILLIS)
		time_delay = delay >> 10;
	else if(freq == CLK_HUNDRED_MICROS)
		time_delay = delay / 100;
	else if(freq == CLK_TENS_MICROS)
		time_delay = delay / 10;
	else
		time_delay = delay;

	// Do nothing until time_delay goes back to 0
	while(time_delay != 0){
	}
}
