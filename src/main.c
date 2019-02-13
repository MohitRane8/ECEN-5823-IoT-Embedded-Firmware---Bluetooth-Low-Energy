/************************************************************************
 *	Assignment 3 - I2C Load Power Management
 *	Author: Mohit Rane
 *	Submission Date: February 6th, 2019
 *
 *	In this assignment, we measure the temperature every 3 seconds
 *	using the inbuilt temperature sensor over I2C. The period is
 *	controlled by the LETIMER.
 ************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "em_device.h"
#include "em_chip.h"
#include "gpio.h"
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif

/* Libraries required by this assignment */
#include "log.h"
#include "letimer.h"
#include "configSLEEP.h"
#include "i2c.h"

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};

/* Declaring an event flag to run schedule for temperature measurement */
extern bool eventFlag;

/* Function declration */
void timerWaitUs(uint32_t us_wait);

int main(void)
{
	// Initialize device
	initMcu();
	// Initialize board
	initBoard();
	// Initialize application
	initApp();

	// Log initialization
	logInit();

	// Initialize stack
	gecko_init(&config);

	// Configure Sleep
	configSLEEP();

	// Initialize GPIO
	gpioInit();

	// Initialize I2C
	initI2CSPM();

	// Initialize LETIMER
	initLETIMER();

	/* Infinite loop */
	while (1)
	{
		if (eventFlag == 1)
		{
			/* Load Power Management */
			//Enable temperature sensor
			GPIO_PinOutSet(gpioPortD, 15);

			// Adding delay of 80 microseconds
			timerWaitUs(80000);
			timerWaitUs(80000);

			// Start of I2C transfer
			performI2CTransfer();

			// Disable temperature sensor
			GPIO_PinOutClear(gpioPortD, 15);

			// Reseting the scheduler flag
			eventFlag = 0;
		}
		else
		{
			/* Enter the deepest possible sleep mode */
			EMU_EnterEM3(true);
		}
 	}
}

// TO BE IMPLEMENTED WITH INTERRUPT
/****************************************************
 *	Delay Generation Function in microseconds
 ****************************************************/
/*
void timerWaitUs(uint32_t us_wait)
{
	// Calculating the number of ticks required
	uint32_t ticks = us_wait/61.03515;

	// Getting the current timer CNT value
	uint32_t cntValue = LETIMER_CounterGet(LETIMER0);

	cntValue -= ticks;
	if(cntValue<0)
		cntValue = 49152 + cntValue;

	// Waiting till the timer gets to the required value
	while(cntValue != LETIMER_CounterGet(LETIMER0));
}
*/
