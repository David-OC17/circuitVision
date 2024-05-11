#ifndef I2CDisplay
#define I2CDisplay

/////////// Includes ///////////
#include "MKL25Z4.h"
#include "main.h"

//////////// MACROS ////////////
// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

#define RETURN_TYPE  uint8_t
#define NO_ERROR     0
#define ERROR        1

// I2C MACROS
#define I2C_TIMEOUT_MS   100

/////////// Variables //////////
// Config struct
typedef struct{
	uint16_t address;
	uint8_t cols;
	uint8_t rows;
	uint8_t datasize;
	uint8_t backlight;
	uint8_t displayFunction;
	uint8_t displayControl;
	uint8_t displayMode;
}LCD_Config_Struct;

// Store possible i2c addresses for display
enum LCD_ADDRESS{
	addr1 = 0x3F,
	addr2 = 0x27
};

// I2C handler
extern I2C_Type *hi2c;
extern uint64_t *millisecs;

////////// Functions  //////////
RETURN_TYPE LCD_DefaultInit(I2C_Type *, uint64_t *);
RETURN_TYPE LCD_Init(LCD_Config_Struct *);
RETURN_TYPE Write4Bits(uint8_t, uint16_t);
RETURN_TYPE Send(uint8_t, uint8_t, uint16_t);
RETURN_TYPE Command(uint8_t, uint16_t);
RETURN_TYPE PulseEnable(uint8_t, uint16_t);

// Display configuring functions
RETURN_TYPE Display(void);
RETURN_TYPE NoDisplay(void);
RETURN_TYPE Cursor(void);
RETURN_TYPE NoCursor(void);
RETURN_TYPE Blink(void);
RETURN_TYPE NoBlink(void);

// Other user commands
RETURN_TYPE Home(void);
RETURN_TYPE Clear(void);
RETURN_TYPE SetCursor(uint8_t, uint8_t);

// Timing functions
void delayMillis(uint16_t);
void delayMicros(uint64_t);

// I2C functions
RETURN_TYPE I2C_WriteByte(uint8_t, uint16_t);

#endif // I2CDisplay
