/*
 * letimer.c
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#include "letimer.h"
#include "configSLEEP.h"

//double rollover;

/* Initial event */
//enum g_event event;

/* Initialization of structure for temperature events */
struct tempEvents TEMP_EVENT;

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


/****************************************************
 *	Delay Generation Function in microseconds
 ****************************************************/
void timerSetEventInUs(uint32_t us_wait)
{
	// Calculating the number of ticks required
	uint32_t ticks = us_wait/61.03515;

	// Getting the current timer CNT value
	uint32_t cntValue = LETIMER_CounterGet(LETIMER0);

	cntValue -= ticks;
	if(cntValue<0)
		cntValue = 49152 + cntValue;

	// Waiting till the timer gets to the required value
//	while(cntValue != LETIMER_CounterGet(LETIMER0));

	LETIMER_CompareSet(LETIMER0, 1, cntValue);
	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
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
	{
		TEMP_EVENT.UF_flag = true;
		rollover++;
	}

	else if(reason & LETIMER_IF_COMP1)
	{
		TEMP_EVENT.COMP1_flag = true;
	}

	/* Clearing pending interrupts */
	LETIMER_IntClear(LETIMER0, reason);

	__enable_irq();
}

