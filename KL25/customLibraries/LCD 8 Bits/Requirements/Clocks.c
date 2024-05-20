#include "Headers/Clocks.h"

// Variables
static uint8_t freq_index;
static uint32_t freq;
static uint64_t systicks;
static uint32_t time_delay;
uint16_t conv_factor;

// Configure the Multipurpose Clock Generator (MCG) - currently to 48 Mhz, bus clock to 8 Mhz
void MCG_Config(void){
	// MCG_C1: CLKS (bit 7-6) = 00
	MCG->C1 |= MCG_C1_CLKS(0);  // Select PLL/FLL as clock source
	// MCG_C1 : IREFS = 1
	MCG->C1 |= MCG_C1_IREFS(1);     // Select internal clock as FLL source
	//MCG->C1 |= MCG_C1_IRCLKEN(1);
	MCG->C2 |= MCG_C2_IRCS(0); 		// Enable slow internal clock (32 khz)

	MCG->C4 = MCG_C4_DMX32(1) | MCG_C4_DRST_DRS(1);  // Set to 48 Mhz FLL
	SIM->CLKDIV1 |= SIM_CLKDIV1_OUTDIV4(1);     // 48 Mhz/6 = 8Mhz bus clock

	SystemCoreClockUpdate(); // Update system core clock value
}

// Systick init
void ClkInit(uint8_t clock_freq){
	SystemCoreClockUpdate();
	freq_index = clock_freq;
	freq = MapFrequency(clock_freq);
	SetConvFactor(clock_freq);
	ClkFreqConfig(freq);
}

// Systick init with interrupt
void ClkInitIt(uint8_t clock_freq){
	SystemCoreClockUpdate();
	freq_index = clock_freq;
	freq = MapFrequency(clock_freq);
	SetConvFactor(clock_freq);
	ClkFreqConfig(freq);
	SysTick->CTRL |= 0x02;       // Enable Systick interrupt
}

// Map system core clock ticks to desired frequency
uint32_t MapFrequency(uint8_t desired_freq){
	switch(desired_freq){
	case CLK_MILLIS:
		return (SystemCoreClock/1000) - 1;
	case CLK_HUNDRED_MICROS:
		return (SystemCoreClock/10000) - 1;
	case CLK_TENS_MICROS:
		return (SystemCoreClock/100000) - 1;
	case CLK_MICROS:
		return (SystemCoreClock/1000000) - 1;
	default:
		return (SystemCoreClock/100000) - 1;    // Tens micros as default case
	}
}

// Conversion factors to use milliseconds as reference for timing
void SetConvFactor(uint8_t freq){
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
	if(freq_index == CLK_MILLIS)
		time_delay = delay;
	else if (freq_index == CLK_HUNDRED_MICROS)
		time_delay = delay * 10;
	else if (freq_index == CLK_TENS_MICROS)
		time_delay = delay * 100;
	else
		time_delay = delay << 10;   // Multiplication cannot catch up at high speeds

	// Do nothing until time_delay goes back to 0
	while(time_delay != 0){
	}
}

void delayUs(uint32_t delay){
	if (freq_index == CLK_MILLIS)
		time_delay = delay >> 10;
	else if(freq_index == CLK_HUNDRED_MICROS)
		time_delay = delay / 100;
	else if(freq_index == CLK_TENS_MICROS)
		time_delay = delay / 10;
	else
		time_delay = delay;

	// Do nothing until time_delay goes back to 0
	while(time_delay != 0){
	}
}
