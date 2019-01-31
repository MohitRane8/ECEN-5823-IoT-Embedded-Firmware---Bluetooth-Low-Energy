/*
 * clock.c
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#include "clock.h"
#include "configSLEEP.h"

void initCLOCK(void)
{
#if ((EnergyMode == 0) | (EnergyMode == 1) | (EnergyMode == 2))
	/* Enable required oscillator */
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);

	/* Enable necessary clocks */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	/* Prescalar of 4 for maximum period of 8 seconds in LETIMER0 */
	CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);
#endif

#if (EnergyMode == 3)
	/* Enable required oscillator */
	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);

	/* Enable necessary clocks */
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
#endif

//	/* Enable required oscillator */
//	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
////	CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);		//for ULFRCO
//
//	/* Enable necessary clocks */
//	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
////	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);		//for ULFRCO
//
//	// Prescalar of 4
//	CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);


	CMU_ClockEnable(cmuClock_LETIMER0, true);
}

