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
 *
 *	References:
 *	1. soc-thermometer Software Example by Silicon Labs
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
#include "main.h"

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

struct gecko_cmd_packet* evt;

extern bool gecko_update(struct gecko_cmd_packet* evt);

int8_t rssi;

extern bool ble_connection_flag;

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
		if(ENERGYMODE > sleepEM0 && TEMP_EVENT.NoEvent == true)
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
		    	  BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_advertise_timing(0, 400, 400, 0, 0));

		        /* Start general advertising and enable connections. */
		    	  BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable));
		        break;

			/* Setting connection parameters for connection */
			case gecko_evt_le_connection_opened_id:
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_set_parameters(evt->data.evt_le_connection_opened.connection, MIN_INTERVAL, MAX_INTERVAL, SLAVE_LATENCY, TIMEOUT));
				ble_connection_flag = true;
				break;

			/* Getting rssi */
			case gecko_evt_gatt_server_characteristic_status_id:
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_get_rssi(evt-> data.evt_gatt_server_characteristic_status.connection));
				break;

			/* Setting transmit power */
			case gecko_evt_le_connection_rssi_id:
				rssi = evt->data.evt_le_connection_rssi.rssi;
				struct gecko_msg_system_set_tx_power_rsp_t *tx_power;

				/* Adjusting transmit power based on proximity of master/client */
				if(rssi > -35)
					tx_power = (TX_MIN);
				else if(( rssi <= -35 ) && (rssi > -45))
					tx_power = -200;
				else if((rssi <= -45 ) && (rssi > -55))
					tx_power = -150;
				else if((rssi <= -55) && (rssi > -65))
					tx_power = -50;
				else if((rssi <= -65) && (rssi > -75))
					tx_power = 0;
				else if((rssi <= -75) && (rssi > -85))
					tx_power = 50;
				else
					tx_power = TX_MAX;

				/* Modifying Tx power in safe way */
				gecko_cmd_system_halt(1);
				gecko_cmd_system_set_tx_power(tx_power);
				gecko_cmd_system_halt(0);
				break;

			/* Handling all external events */
			case gecko_evt_system_external_signal_id:
				if(ble_connection_flag == true){
					if (((evt->data.evt_system_external_signal.extsignals) & UF_FLAG) != 0) {
						TEMP_EVENT.UF_flag = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}

					if (((evt->data.evt_system_external_signal.extsignals) & COMP1_FLAG) != 0) {
						TEMP_EVENT.COMP1_flag = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}

					if (((evt->data.evt_system_external_signal.extsignals) & I2C_TRANSACTION_DONE) != 0) {
						TEMP_EVENT.I2CTransactionDone = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}

					if (((evt->data.evt_system_external_signal.extsignals) & I2C_TRANSACTION_ERROR) != 0) {
						TEMP_EVENT.I2CTransactionError = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}
				}
		}
 	}
}
