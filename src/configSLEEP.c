/*
 * configSLEEP.c
 *
 *  Created on: Jan 30, 2019
 *      Author: Mohit
 */

#include "configSLEEP.h"

void configSLEEP(void)
{
	const SLEEP_Init_t init = {0};
	SLEEP_InitEx(&init);
}


