/*
 * clock.c
 *	Clock configuration and initialization
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#include "clock.h"
#include "configSLEEP.h"

void initCLOCK(void)
{
	/* Clock configurations (LFXO - 32768 Hz) for EM0, EM1 and EM2 */
	#if ((ENERGYMODE == 0) | (ENERGYMODE == 1) | (ENERGYMODE == 2))
		/* Enable required oscillator */
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

		/* Enable necessary clocks */
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

		/* **********************************************************
		 * Prescalar of 2 for maximum period of 4 seconds in LETIMER0
		 * 32768/2 = 16384 Hz and FFFF = 65536
		 * (1/16384)*65536 = 4 seconds
		 * **********************************************************/
		CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_2);
	#endif

	/* Clock configurations (ULFRCO - 1000 Hz) for EM3 */
	#if (ENERGYMODE == 3)
		/* Enable required oscillator */
		CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

		/* Enable necessary clocks */
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
	#endif
	
	/* Enabling clock of LETIMER0 */
	CMU_ClockEnable(cmuClock_LETIMER0, true);
}

