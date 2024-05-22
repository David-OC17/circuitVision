#ifndef KEYPAD_H
#define KEYPAD_H

#include "common.h"

/**
 * @brief Configure clock and pins to use 4x4 matrix keypad.
 */
void keypad_init(void);

/**
 * @brief Query the keypad to sense which key is getting pressed by the user.
 *
 * @return Returns a character corresponding to the letter/number that was
 * pressed in the keypad.
 */
char keypad_getkey(void);

#endif // KEYPAD_H
