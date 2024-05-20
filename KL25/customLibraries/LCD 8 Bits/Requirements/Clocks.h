#ifndef CLOCKS_H
#define CLOCKS_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"

//////////// MACROS ////////////

// Conversions
#define MILLIS2HUNDRED_MICROS  10
#define MILLIS2TENS_MICROS     100
#define MILLIS2MICROS          1000

/////////// Variables //////////
extern uint16_t conv_factor;

enum TIME_UNIT{
	CLK_MILLIS          = 0,
	CLK_HUNDRED_MICROS  = 1,
	CLK_TENS_MICROS     = 2,
	CLK_MICROS          = 3
};

/////////// Functions //////////
void MCG_Config(void);
void ClkInit(uint8_t);
void ClkInitIt(uint8_t);
void ClkFreqConfig(uint32_t);
uint32_t MapFrequency(uint8_t);

// Getters & setters
uint64_t* GetSysTicks(void);
void SetDelayTicks(uint32_t);
void SetConvFactor(uint8_t);

// Delay Functions
void delayMs(uint32_t);
void delayUs(uint32_t);

#endif /*CLOCKS_H*/
