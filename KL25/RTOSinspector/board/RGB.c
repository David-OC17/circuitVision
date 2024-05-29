#include "RGB.h"

// Turn on LED
void LED_On(GPIO_Type *port, uint32_t pin) { port->PDOR &= ~pin; }

// Turn off LED
void LED_Off(GPIO_Type *port, uint32_t pin) { port->PDOR |= pin; }

// Toggle the LED
void LED_Toggle(GPIO_Type *port, uint32_t pin) { port->PTOR |= pin; }

// Set all LEDs On
void RGB_On(void) {
  LED_On(GREEN_LED_PORT, GREEN);
  LED_On(BLUE_LED_PORT, BLUE);
  LED_On(RED_LED_PORT, RED);
}

// Set all LEDs Off
void RGB_Off(void) {
  LED_Off(GREEN_LED_PORT, GREEN);
  LED_Off(BLUE_LED_PORT, BLUE);
  LED_Off(RED_LED_PORT, RED);
}

/************************************************
 *                    Colors 
 ***********************************************/
void GreenOn(void) { LED_On(GREEN_LED_PORT, GREEN); }

void BlueOn(void) { LED_On(BLUE_LED_PORT, BLUE); }

void RedOn(void) { LED_On(RED_LED_PORT, RED); }

void WhiteOn(void) { RGB_On(); }

void YellowOn(void) {
  GreenOn();
  RedOn();
}

void PurpleOn(void) {
  RedOn();
  BlueOn();
}

/************************************************
 *                 Colors Off 
 ***********************************************/
void GreenOff(void) { LED_Off(GREEN_LED_PORT, GREEN); }

void BlueOff(void) { LED_Off(BLUE_LED_PORT, BLUE); }

void RedOff(void) { LED_Off(RED_LED_PORT, RED); }

void WhiteOff(void) { RGB_Off(); }

void YellowOff(void) {
  GreenOff();
  RedOff();
}

void PurpleOff(void) {
  RedOff();
  BlueOff();
}

/************************************************
 *               Colors Toggle
 ***********************************************/
void GreenToggle(void) { LED_Toggle(GREEN_LED_PORT, GREEN); }

void BlueToggle(void) { LED_Toggle(BLUE_LED_PORT, BLUE); }

void RedToggle(void) { LED_Toggle(RED_LED_PORT, RED); }

void WhiteToggle(void) {
  GreenToggle();
  BlueToggle();
  RedToggle();
}

void YellowToggle(void) {
  GreenToggle();
  RedToggle();
}
void PurpleToggle(void) {
  BlueToggle();
  RedToggle();
}
