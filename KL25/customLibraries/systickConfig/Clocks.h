#ifndef CLOCKS_H
#define CLOCKS_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"

//////////// MACROS ////////////
#define CLK_MILLIS          (SystemCoreClock/1000) - 1
#define CLK_HUNDRED_MICROS  (SystemCoreClock/10000) - 1
#define CLK_TENS_MICROS     (SystemCoreClock/100000) - 1
#define CLK_MICROS          (SystemCoreClock/1000000) - 1

/////////// Variables //////////

/////////// Functions //////////
void ClkInit(uint32_t);
void ClkInitIt(uint32_t);
void ClkFreqConfig(uint32_t);

// Getters & setters
uint64_t GetSysTicks(void);
void SetDelayTicks(uint32_t);

// Delay Functions
void delayMs(uint32_t);
void delayUs(uint32_t);

#endif /*CLOCKS_H*/
