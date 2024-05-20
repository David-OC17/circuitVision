#ifndef LCD8_H
#define LCD8_H

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"

//////////// MACROS ////////////
// This should vary depending on the LCD8 connections
#define RS 0x04 /* PTA2 mask */
#define RW 0x10 /* PTA4 mask */
#define EN 0x20 /* PTA5 mask */

/* LCD COMMANDS
	- 0x38 -> set 8-bit data, 2-line, 5x7 font
	- 0x06 -> move cursor right
	- 0x01 -> clear screen, move cursor to 0,0
	- 0x0F -> turn on display, cursor blinking
*/

#define LCD8_CONFIG_COMMAND   	    0x38
#define LCD8_RIGHT_CURSOR_COMMAND   0x06
#define LCD8_CLEAR_COMMAND 		    0x01
#define LCD8_BLINK_HOME				0x0F
#define LCD8_NO_BLINK				(0x08) & ~ 1

/////////// Variables //////////
typedef struct{
	// LCD8 data pins	// LCD8 PORTS
	uint8_t d0_pin;		PORT_Type *d0_port;
	uint8_t d1_pin;		PORT_Type *d1_port;
	uint8_t d2_pin;     PORT_Type *d2_port;
	uint8_t d3_pin;		PORT_Type *d3_port;
	uint8_t d4_pin;		PORT_Type *d4_port;
	uint8_t d5_pin;		PORT_Type *d5_port;
	uint8_t d6_pin;		PORT_Type *d6_port;
	uint8_t d7_pin;		PORT_Type *d7_port;
	// Mode pins
	uint8_t enable_pin;	PORT_Type *enable_port;
	uint8_t rs_pin;		PORT_Type *rs_port;
	uint8_t rw_pin;		PORT_Type *rw_port;
} LCD8_Struct;

// GPIOs for data pins
extern GPIO_Type *d0_gpio, *d1_gpio, *d2_gpio, *d3_gpio,
				  *d4_gpio, *d5_gpio, *d6_gpio, *d7_gpio;

// GPIOs for additional pins
extern GPIO_Type *enable_gpio, *rw_gpio, *rs_gpio;

////////// Functions  //////////
void LCD8_Init(void);
void LCD8_Command(unsigned char);
void LCD8_CommandNoWait(unsigned char);
void LCD8_Data(unsigned char);
void LCD8_Ready(void);

// Higher level commands
void LCD8_Write(char *);
void LCD8_Clear(void);
void LCD8_Blink(void);
void LCD8_NoBlink(void);

#endif /*LCD8_H*/
