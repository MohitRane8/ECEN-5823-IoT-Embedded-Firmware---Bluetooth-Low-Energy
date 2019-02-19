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
#include "log.h"
//#include "event.h"
#include "scheduler.h"

uint32_t timestamp;

/* Setting time period for temperature measurement */
#define PERIOD_MS 3000

/* Ticks that will be loaded in COMP0 as per time period */
uint32_t periodCount;

/* Variable which increments every time a time period is completed */
uint8_t rollover;

void initLETIMER(void);
void LETIMER0_IRQHandler(void);
void timerSetEventInUs(uint32_t us_wait);

#endif /* SRC_LETIMER_H_ */
