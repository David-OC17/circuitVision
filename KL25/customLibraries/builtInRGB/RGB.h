#ifndef RGB_H
#define RGB_H

/////////// Includes ///////////
#include "main.h"

//////////// MACROS ////////////
#define TIME_TO_BLINK_MS 1000    // Adjust as needed
#define GREEN_LED_PORT   GPIOB
#define BLUE_LED_PORT    GPIOD
#define RED_LED_PORT     GPIOB

/////////// Variables //////////
enum RGB_LED {
	RED = 0x40000,
	GREEN = 0x80000,
	BLUE = 0x02,
	PURPLE = 0x40000 | 0x02,
	WHITE = 0x40000 | 0x80000 | 0x02,
	YELLOW = 0x40000 | 0x80000
};

////////// Functions  //////////
void RGB_Init(void);

// For all LEDs in RGB LED
void RGB_On(void);
void RGB_Off(void);

// To specify LEDs
void LED_On(GPIO_Type *, uint32_t);
void LED_Off(GPIO_Type *, uint32_t);
void LED_Toggle(GPIO_Type *, uint32_t);

// For individual LEDs
void GreenOn(void);
void BlueOn(void);
void RedOn(void);

void GreenOff(void);
void BlueOff(void);
void RedOff(void);

void GreenToggle(void);
void BlueToggle(void);
void RedToggle(void);

// Basic Color Combinations
void WhiteOn(void);
void YellowOn(void);
void PurpleOn(void);

void WhiteOff(void);
void YellowOff(void);
void PurpleOff(void);

void WhiteToggle(void);
void YellowToggle(void);
void PurpleToggle(void);

#endif /*RGB_H*/
