/*
 * letimer.c
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#include "letimer.h"
#include "configSLEEP.h"

/**************************************
 *	LETIMER0 Initialization
 **************************************/
void initLETIMER(void)
{
	/* Initializing clock with proper settings */
	initCLOCK();

	/* Initialize LETIMER */
	LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;
	letimerInit.comp0Top = true;
	LETIMER_Init(LETIMER0, &letimerInit);

#if ((EnergyMode == 0) | (EnergyMode == 1) | (EnergyMode == 2))
	/* Setting value of COMP0 - counting will start from this value */
	uint32_t periodCount = PERIOD_MS * 8.192;
	/* Setting value of COMP1 - where interrupt will occur and LED will turn ON */
	uint32_t ontimeCount = ONTIME_MS * 8.192;
#endif

#if (EnergyMode == 3)
	/* Setting value of COMP0 - counting will start from this value */
	uint32_t periodCount = PERIOD_MS;
	/* Setting value of COMP1 - where interrupt will occur and LED will turn ON */
	uint32_t ontimeCount = ONTIME_MS;
#endif

	/* Setting value of COMP0 - counting will start from this value */
	LETIMER_CompareSet(LETIMER0, 0, periodCount);

	/* Setting value of COMP1 - where interrupt will occur and LED will turn ON */
	LETIMER_CompareSet(LETIMER0, 1, ontimeCount);

	/* Enabling the interrupt flags */
	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

	/* Enabling LETIMER0 in NVIC */
	NVIC_EnableIRQ(LETIMER0_IRQn);

	/* Starting LETIMER0 */
	LETIMER_Enable(LETIMER0, true);
}

/**************************************
 *	LETIMER0 Interrupt Handling
 **************************************/
void LETIMER0_IRQHandler(void)
{
	__disable_irq();

	uint32_t reason = LETIMER_IntGet(LETIMER0);

	if((reason & LETIMER_IF_COMP1) == LETIMER_IF_COMP1)
		gpioLed0SetOn();

	else if((reason & LETIMER_IF_UF) == LETIMER_IF_UF)
		gpioLed0SetOff();

	LETIMER_IntClear(LETIMER0, reason);

	__enable_irq();
}

