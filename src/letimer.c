/*
 * letimer.c
 *
 *  Created on: Jan 29, 2019
 *      Author: Mohit
 */

#include "letimer.h"
#include "configSLEEP.h"


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
	letimerInit.enable = false;
	letimerInit.comp0Top = true;		//To let LETIMER start counting from COMP0 value
	LETIMER_Init(LETIMER0, &letimerInit);

	/* Calculating value to be loaded in COMP0 - counting will start from this value */
	periodCount = (PERIOD_MS/1000) * (CMU_ClockFreqGet(cmuClock_LETIMER0));

	/* Setting value of COMP0 - counting will start from this value */
	LETIMER_CompareSet(LETIMER0, 0, periodCount);

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
	uint32_t ticks;
	uint32_t cntValue;

	/* Calculating the number of ticks required to set delay */
	ticks = (us_wait/1000) * (CMU_ClockFreqGet(cmuClock_LETIMER0)/1000);

	/* Getting the current timer CNT value */
	cntValue = LETIMER_CounterGet(LETIMER0);
	uint32_t endTicks = 0;

	if(cntValue > ticks){
		//normal condition
		endTicks = cntValue - ticks;
	}
	else{
		endTicks = LETIMER_CompareGet(LETIMER0, 0) - (ticks-cntValue);
	}
	/* Enabling COMP1 interrupt */
	LETIMER_IntClear(LETIMER0, LETIMER_IFC_COMP1);
	/* Setting the value in COMP1 */
	LETIMER_CompareSet(LETIMER0, 1, endTicks);
	LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);
}


/**************************************
 *	LETIMER0 Interrupt Handling
 **************************************/
void LETIMER0_IRQHandler(void)
{
	uint32_t reason = LETIMER_IntGet(LETIMER0);

	/* Clearing pending interrupts */
	LETIMER_IntClear(LETIMER0, reason);

	/* Setting the event flag */
	if(reason & LETIMER_IF_UF)
	{
		ext_evt_status = UF_FLAG;
		gecko_external_signal(ext_evt_status);
//		TEMP_EVENT.UF_flag = true;
//		TEMP_EVENT.NoEvent = false;
		rollover++;
	}

	if(reason & LETIMER_IF_COMP1)
	{
		ext_evt_status = COMP1_FLAG;
		gecko_external_signal(ext_evt_status);
//		TEMP_EVENT.COMP1_flag = true;
//		TEMP_EVENT.NoEvent = false;
		LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);
	}
}

