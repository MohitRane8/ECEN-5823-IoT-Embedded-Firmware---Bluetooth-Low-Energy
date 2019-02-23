/*
 * gpio.h
 *
 *  Created on: Dec 12, 2018
 *      Author: Dan Walkes
 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>

/* LCD header file */
#include "displayls013b7dh03config.h"

/* Header files required for display */
#define GPIO_SET_DISPLAY_EXT_COMIN_IMPLEMENTED 	1
#define GPIO_DISPLAY_SUPPORT_IMPLEMENTED		1

void gpioInit();
void gpioLed0SetOn();
void gpioLed0SetOff();
void gpioLed1SetOn();
void gpioLed1SetOff();

/* Functions required for display */
void gpioEnableDisplay();
void gpioSetDisplayExtcomin(bool high);

#endif /* SRC_GPIO_H_ */
