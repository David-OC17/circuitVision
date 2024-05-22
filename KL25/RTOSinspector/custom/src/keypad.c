#include "../include/keypad.h"

char keypad_getkey(void) {
  int row, col;
  const char row_select[] = {0x01, 0x02, 0x04, 0x08};
  /* one row is active */
  /* check to see any key pressed */
  PTC->PDDR |= 0x0F; /* enable all rows */
  PTC->PCOR = 0x0F;
  delayUs(2);             /* wait for signal return */
  col = PTC->PDIR & 0xF0; /* read all columns */
  PTC->PDDR = 0;          /* disable all rows */
  if (col == 0xF0)
    return 0; /* no key pressed */

  /* If a key is pressed, we need find out which key.*/
  for (row = 0; row < 4; row++) {
    PTC->PDDR = 0;                /* disable all rows */
    PTC->PDDR |= row_select[row]; /* enable one row */
    PTC->PCOR = row_select[row];  /* drive active row low*/
    delayUs(2);                   /* wait for signal to settle */
    col = PTC->PDIR & 0xF0;       /* read all columns */
    if (col != 0xF0)
      break;
    /* if one of the input is low, some key is pressed. */
  }

  PTC->PDDR = 0; /* disable all rows */
  if (row == 4)
    return 0; /* if we get here, no key is pressed */
  /* gets here when one of the rows has key pressed*/
  /*check which column it is*/
  if (col == 0xE0)
    return row * 4 + 1; /* key in column 0 */
  if (col == 0xD0)
    return row * 4 + 2; /* key in column 1 */
  if (col == 0xB0)
    return row * 4 + 3; /* key in column 2 */
  if (col == 0x70)
    return row * 4 + 4; /* key in column 3 */
  return 0;             /* just to be safe */
}
