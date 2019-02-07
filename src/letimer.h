/*
 * letimer.h
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

// define period and LED on time in milli-seconds
#define PERIOD_MS 3000
//#define ONTIME_MS 175

void initLETIMER(void);
void LETIMER0_IRQHandler(void);

#endif /* SRC_LETIMER_H_ */
