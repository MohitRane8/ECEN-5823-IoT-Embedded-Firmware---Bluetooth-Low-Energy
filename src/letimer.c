/*
 * letimer.c
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#include "letimer.h"
#include "configSLEEP.h"

/* 
 * A flag to set an event in the schedule.
 * The schedule will activate the I2C and take temperature readings.
*/
bool eventFlag;

/**************************************
 *	LETIMER0 Initialization
 **************************************/
void initLETIMER(void)
{
	/* Initializing clock with proper settings */
	initCLOCK();

	/* Initialize LETIMER */
	LETIMER_Init_TypeDef letimerInit = LETIMER_INIT_DEFAULT;
	letimerInit.comp0Top = true;		//To let LETIMER start counting from COMP0 value
	LETIMER_Init(LETIMER0, &letimerInit);

	/* Setting value of COMP0 - counting will start from this value */
	LETIMER_CompareSet(LETIMER0, 0, 49152);

	/* Enabling the Underflow interrupt flag */
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

	/* Setting the event flag */
	if(reason & LETIMER_IF_UF)
		eventFlag = true;

	/* Clearing pending interrupts */
	LETIMER_IntClear(LETIMER0, reason);

	__enable_irq();
}

