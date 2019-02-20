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
#include "scheduler.h"
#include "gecko_ble_errors.h"

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


extern bool gecko_update(struct gecko_cmd_packer* evt);

//extern bool NoEvent = true;

extern struct gecko_cmd_packet* evt;

//void gecko_ecen5823_update(struct gecko_cmd_packer* evt)
//{
//
//}


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
//		if(ENERGYMODE > sleepEM0 && TEMP_EVENT.NoEvent == true)
//		{
//			evt = gecko_wait_event();
//		}

		if(ENERGYMODE > sleepEM0)
		{
			evt = gecko_wait_event();
		}

		gecko_update(evt);

		switch BGLIB_MSG_ID(evt->header) {
			case gecko_evt_system_boot_id:

				/* Set advertising parameters. 100ms advertisement interval.
				 * The first parameter is advertising set handle
				 * The next two parameters are minimum and maximum advertising interval, both in
				 * units of (milliseconds * 1.6).
				 * The last two parameters are duration and maxevents left as default. */
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_advertise_timing(0, 160, 160, 0, 0))

				/* Start general advertising and enable connections. */
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable));
				break;


			case gecko_evt_le_connection_opened_id:
				gecko_cmd_le_connection_set_parameters();
				break;


			case gecko_evt_le_connection_closed_id:
	//			gecko_cmd_gatt_server_write_attribute_value();
	//			gecko_cmd_gatt_server_send_characteristic_notification();
				break;

			case gecko_evt_system_external_signal_id:
				if ((evt->data.evt_system_external_signal.extsignals) & UF_FLAG) {
					/* Run application specific task */
					TEMP_EVENT.UF_flag = true;
					scheduler();
				}
				break;

				if ((evt->data.evt_system_external_signal.extsignals) & COMP1_FLAG) {
					/* Run application specific task */
					TEMP_EVENT.COMP1_flag = true;
					scheduler();
				}
				break;

				if ((evt->data.evt_system_external_signal.extsignals) & I2C_TRANSACTION_DONE) {
					/* Run application specific task */
					TEMP_EVENT.I2CTransactionDone = true;
					scheduler();
				}
				break;

				if ((evt->data.evt_system_external_signal.extsignals) & I2C_TRANSACTION_ERROR) {
					/* Run application specific task */
					TEMP_EVENT.I2CTransactionError = true;
					scheduler();
				}
				break;
		}
 	}
}
