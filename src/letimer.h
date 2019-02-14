/*
 * letimer.h
 * Header file required for letimer.c
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#ifndef SRC_LETIMER_H_
#define SRC_LETIMER_H_

#include "em_cmu.h"
#include "em_letimer.h"
#include "gpio.h"
#include "clock.h"
#include "event.h"

/* Setting time period for temperature measurement */
#define PERIOD_MS 3000

void initLETIMER(void);
void LETIMER0_IRQHandler(void);
void timerSetEventInUs(uint32_t us_wait);

#endif /* SRC_LETIMER_H_ */
