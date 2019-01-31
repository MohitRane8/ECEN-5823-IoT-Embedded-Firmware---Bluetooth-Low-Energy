/************************************************************************
 *	Assignment 2 - Managing Energy Modes
 *	Author: Mohit Rane
 *	Submission Date: January 30th, 2019
 *
 *	LED blinking using LETIMER
 *	
 *	To change LED on time or period:
 *	Go to letimer.h and change "PERIOD_MS" and "ONTIME_MS" as required.
 *
 *	To change the sleep state of CPU:
 *	Go to configSLEEP.h and change "EnergyMode" as required.
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
#include "letimer.h"
#include "configSLEEP.h"

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

int main(void)
{
	// Initialize device
	initMcu();
	// Initialize board
	initBoard();
	// Initialize application
	initApp();

	// Initialize stack
	gecko_init(&config);

	// Configure sleep
	configSLEEP();

	// Initialize GPIO
	gpioInit();

	// Initialize LETIMER
	initLETIMER();

	/* Infinite loop */
	while (1) {
		#if (EnergyMode == 0)
		#endif

		#if (EnergyMode == 1)
			SLEEP_SleepBlockBegin(sleepEM2);
			SLEEP_Sleep();
		#endif

		#if (EnergyMode == 2)
			SLEEP_SleepBlockBegin(sleepEM3);
			SLEEP_Sleep();
		#endif

		#if (EnergyMode == 3)
			EMU_EnterEM3(true);
		#endif
 	}
}
