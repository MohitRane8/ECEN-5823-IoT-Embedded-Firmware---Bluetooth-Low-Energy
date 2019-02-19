/************************************************************************
 *	Assignment 4 - I2C Load Power Management Part 2
 *	Author: Mohit Rane
 *	Submission Date: February 13th, 2019
 *
 *	In this assignment, we measure the temperature every 3 seconds
 *	using the inbuilt temperature sensor over I2C. The period is
 *	controlled by the LETIMER.
 *
 *	ENERGYMODE parameter defined in configSLEEP.h allows the system to
 *	sleep in that particular sleep mode.
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
//#include "event.h"
#include "scheduler.h"

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


///* Initialization of structure for temperature events */
//extern struct tempEvents TEMP_EVENT = {0};
//
///* Defining states for state machine */
//enum temp_sensor_state {
//	TEMP_SENSOR_POWER_OFF,
//	TEMP_SENSOR_WAIT_FOR_POWER_UP,
//	TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE,
//	TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE,
//	TEMP_SENSOR_I2C_ERROR = -1
//};
//
///* Defining the initial state */
//enum temp_sensor_state current_state = TEMP_SENSOR_POWER_OFF;
//enum temp_sensor_state next_state = TEMP_SENSOR_WAIT_FOR_POWER_UP;

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
	initI2C();

	// Initialize LETIMER
	initLETIMER();

	/* Initialize all events to 0 */
//	TEMP_EVENT = {0};

	TEMP_EVENT.NoEvent = true;

#if ((ENERGYMODE == 0) | (ENERGYMODE == 1) | (ENERGYMODE == 2))
	SLEEP_SleepBlockBegin(ENERGYMODE+1);
#endif

	/* Infinite loop */
	while(1)
	{
		/* System sleeps when there is no event */
		if(ENERGYMODE > sleepEM0 && TEMP_EVENT.NoEvent == true)
			SLEEP_Sleep();

		scheduler();
 	}
}
