/************************************************************************
 *	Assignment 7 - Bluetooth BLE Client
 *	Author: Mohit Rane
 *	Submission Date: February 27th, 2019
 *
 *	In this assignment, we measure the temperature every 3 seconds
 *	using the inbuilt temperature sensor over I2C. The period is
 *	controlled by the LETIMER.
 *
 *	The temperature data is sent to the client bluetooth device when
 *	a connection is established. The system only takes temperature
 *	when a connection is made.
 *
 *	The temperature data is also displayed on the in-built LCD on server
 *	side.
 *
 *	ENERGYMODE parameter defined in configSLEEP.h allows the system to
 *	sleep in that particular sleep mode.
 *
 *	References:
 *	1. soc-thermometer Software Example by Silicon Labs
 *	2. https://www.silabs.com/community/wireless/bluetooth/knowledge-
 *	   base.entry.html/2016/10/04/scheduling_applicati-ERXS
 *  3. https://www.silabs.com/community/wireless/bluetooth/forum.topic
 *     .html/bgm121_i_cannot_exp-B8Ae
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

/* Flag which is set upon successful connection
 * Indicates the system to do the temperature reading process */
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

	// Initialize GPIO
	gpioInit();

#if DEVICE_IS_BLE_SERVER
	// Configure Sleep
	configSLEEP();

	// Initialize I2C
	initI2C();

	// Initialize LETIMER
	initLETIMER();

	// Setting initial scheduler event as no event
	TEMP_EVENT.NoEvent = true;

	// Blocking sleep according to the mode defined
#if ((ENERGYMODE == 0) | (ENERGYMODE == 1) | (ENERGYMODE == 2))
	SLEEP_SleepBlockBegin(ENERGYMODE+1);
#endif
#endif

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
	// Initializes LCD display
	displayInit();
	displayPrintf(DISPLAY_ROW_NAME, BLE_DEVICE_TYPE_STRING);
#endif

#if !DEVICE_IS_BLE_SERVER
handle.connection = 0;
handle.service = 0;
handle.characteristic = 0;
GATT_state = GATT_WAITING_FOR_SERVICE_DISCOVERY;
#endif


	/* Infinite loop */
	while(1)
	{
#if DEVICE_IS_BLE_SERVER
		/* DEVICE IS SERVER */

		/* Allowing the system to sleep in defined state if there is no event */
		if(ENERGYMODE >= sleepEM0 && TEMP_EVENT.NoEvent == true){
			/* Blocking until new event arrives */
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
		    	  BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_advertise_timing(0, ADV_MIN_INTERVAL, ADV_MIN_INTERVAL, 0, 0));

		        /* Start general advertising and enable connections. */
		    	  BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable));

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
		    	  displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
#endif
		        break;

			case gecko_evt_le_connection_opened_id:
				/* Setting connection parameters for connection */
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_set_parameters(evt->data.evt_le_connection_opened.connection, MIN_INTERVAL, MAX_INTERVAL, SLAVE_LATENCY, TIMEOUT));

				/* Setting connection flag to start state machine based on external events */
				ble_connection_flag = true;

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				struct gecko_msg_system_get_bt_address_rsp_t * rsp;
				bd_addr addr;

				rsp = gecko_cmd_system_get_bt_address();
				addr = rsp->address;
				displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x", addr.addr[0],addr.addr[1],addr.addr[2],addr.addr[3],addr.addr[4],addr.addr[5]);
				displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
#endif
				break;

			case gecko_evt_gatt_server_characteristic_status_id:
				/* Getting rssi */
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_get_rssi(evt-> data.evt_gatt_server_characteristic_status.connection));
				break;

			case gecko_evt_le_connection_rssi_id:
				rssi = evt->data.evt_le_connection_rssi.rssi;
				int16 tx_power;

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
				gecko_cmd_system_set_tx_power(tx_power);	// Setting transmit power
				gecko_cmd_system_halt(0);
				break;

			/* Case handling all external events */
			case gecko_evt_system_external_signal_id:
				if(ble_connection_flag == true){
					// 3 second underflow flag check
					if (((evt->data.evt_system_external_signal.extsignals) & UF_FLAG) != 0) {
						TEMP_EVENT.UF_flag = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}

					// 80 ms comp1 flag check
					if (((evt->data.evt_system_external_signal.extsignals) & COMP1_FLAG) != 0) {
						TEMP_EVENT.COMP1_flag = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}

					// Write and Read transaction completion check
					if (((evt->data.evt_system_external_signal.extsignals) & I2C_TRANSACTION_DONE) != 0) {
						TEMP_EVENT.I2CTransactionDone = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}

					// Write and Read transaction error check
					if (((evt->data.evt_system_external_signal.extsignals) & I2C_TRANSACTION_ERROR) != 0) {
						TEMP_EVENT.I2CTransactionError = true;
						TEMP_EVENT.NoEvent = false;
						scheduler();
					}
				}
				break;

			case gecko_evt_le_connection_closed_id:
				gecko_cmd_system_set_tx_power(0);
				/* Restart advertising after client has disconnected */
				gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);

				/* Turning the connection flag to stop the system from taking temperature */
				ble_connection_flag = false;

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				displayPrintf(DISPLAY_ROW_BTADDR, "");
				displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
				displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
#endif
				break;
		}

#else
		/* DEVICE IS CLIENT */


		evt = gecko_wait_event();

		gecko_update(evt);

		switch BGLIB_MSG_ID(evt->header) {
			case gecko_evt_system_boot_id:
				/*
				 * This case is triggered when the client boots
				 * 1st argument: LE 1M PHY
				 */
				gecko_cmd_le_gap_start_discovery(1, le_gap_general_discoverable);

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
		    	displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
#endif
				break;

			case gecko_evt_le_gap_scan_response_id:
				/*
				 * This event reports any advertising or scan response packet that is received
				 * by the device's radio while in scanning mode.
				 * 3rd argument: LE 1M PHY
				 */
				;
				bd_addr serverBtAddr;

				serverBtAddr.addr[0] = 0x3d;
				serverBtAddr.addr[1] = 0x62;
				serverBtAddr.addr[2] = 0x08;
				serverBtAddr.addr[3] = 0x6f;
				serverBtAddr.addr[4] = 0x0d;
				serverBtAddr.addr[5] = 0x00;

				bd_addr recv_addr;
				uint8 recv_addr_type;
				recv_addr = evt->data.evt_le_gap_scan_response.address;
				recv_addr_type = evt->data.evt_le_gap_scan_response.address_type;

				LOG_INFO("Received Address: %x:%x:%x:%x:%x:%x\n", recv_addr.addr[0], recv_addr.addr[1], recv_addr.addr[2], recv_addr.addr[3], recv_addr.addr[4], recv_addr.addr[5]);

				if((recv_addr.addr[0] == serverBtAddr.addr[0]) &
					(recv_addr.addr[1] == serverBtAddr.addr[1]) &
					(recv_addr.addr[2] == serverBtAddr.addr[2]) &
					(recv_addr.addr[3] == serverBtAddr.addr[3]) &
					(recv_addr.addr[4] == serverBtAddr.addr[4]) &
					(recv_addr.addr[5] == serverBtAddr.addr[5])){
					gecko_cmd_le_gap_end_procedure();
//					gecko_cmd_le_gap_connect(recv_addr, recv_addr_type, le_gap_phy_1m);
//					displayPrintf(DISPLAY_ROW_BTADDR2, "Received address");
					gecko_cmd_le_gap_connect(evt->data.evt_le_gap_scan_response.address, evt->data.evt_le_gap_scan_response.address_type, le_gap_phy_1m);
				}

//				gecko_cmd_le_gap_connect(evt->data.evt_le_gap_scan_response.address, evt->data.evt_le_gap_scan_response.address_type, le_gap_phy_1m);

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				struct gecko_msg_system_get_bt_address_rsp_t * rsp;
				bd_addr addr;

				rsp = gecko_cmd_system_get_bt_address();
				addr = rsp->address;
				displayUpdate();
				displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);
#endif
				break;

			case gecko_evt_le_connection_opened_id:
				/* Setting connection parameters for connection */
				gecko_cmd_le_connection_set_parameters(evt->data.evt_le_connection_opened.connection, MIN_INTERVAL, MAX_INTERVAL, SLAVE_LATENCY, TIMEOUT);

				handle.connection = evt->data.evt_le_connection_opened.connection;

				/* Health Thermometer service UUID: 1809 */
				HTM_service.data[0] = 0x09;
				HTM_service.data[1] = 0x18;
				HTM_service.size = 2;

				gecko_cmd_gatt_discover_primary_services_by_uuid(handle.connection, HTM_service.size, HTM_service.data);

				GATT_state = GATT_WAITING_FOR_SERVICE_DISCOVERY;

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				displayPrintf(DISPLAY_ROW_CONNECTION, "YYY Connected YYY");
#endif
				break;

			case gecko_evt_gatt_service_id:
				handle.service = evt->data.evt_gatt_service.service;
				break;

			case gecko_evt_gatt_characteristic_id:
				handle.characteristic = evt->data.evt_gatt_characteristic.characteristic;
				uint8array uuid = evt->data.evt_gatt_characteristic.uuid;
				LOG_INFO("Characteristic ID: %d\n", handle.characteristic);
				break;

			case gecko_evt_gatt_characteristic_value_id:
				handle.connection = evt->data.evt_gatt_characteristic.connection;

				// do we need to create an event gatt_characteristic_value for this?
				if(evt->data.evt_gatt_characteristic_value.att_opcode == gatt_handle_value_indication)
					gecko_cmd_gatt_send_characteristic_confirmation(handle.connection);
//				evt->data.evt_gatt_characteristic_value.value.data
				LOG_INFO("char value recvd");
				break;

			case gecko_evt_gatt_procedure_completed_id:
				LOG_INFO("GATT state: %d\n\n", GATT_state);
				if(GATT_state == GATT_WAITING_FOR_SERVICE_DISCOVERY){

//					handle.connection = evt->data.evt_gatt_procedure_completed.connection;

					/* Temperature Measurement characteristic UUID: 2A1C */
					HTM_characteristic.data[0] = 0x1C;
					HTM_characteristic.data[1] = 0x2A;
					HTM_characteristic.size = 2;

//					displayUpdate();
					displayPrintf(DISPLAY_ROW_BTADDR2, "%d", handle.service);

					struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t* response;
					LOG_INFO("Service response result: %d", response->result);
					response = gecko_cmd_gatt_discover_characteristics_by_uuid(handle.connection, handle.service, HTM_characteristic.size, HTM_characteristic.data);

					LOG_INFO("service handle: %x\n", handle.service);
					GATT_state = GATT_WAITING_FOR_CHARACTERISTICS_DISCOVERY;
				}

				else if(GATT_state == GATT_WAITING_FOR_CHARACTERISTICS_DISCOVERY){

//					displayUpdate();
//					displayPrintf(DISPLAY_ROW_BTADDR2, "Dis characte");
					displayPrintf(DISPLAY_ROW_CLIENTADDR, "%x", handle.characteristic);

//					handle.connection = evt->data.evt_gatt_procedure_completed.connection;
//					gecko_cmd_gatt_read_characteristic_value(handle.connection, handle.characteristic);
					gecko_cmd_gatt_set_characteristic_notification(handle.connection, handle.characteristic, gatt_indication);
					GATT_state = GATT_WAITING_FOR_CHARACTERISTIC_VALUE;
				}

				else if(GATT_state == GATT_WAITING_FOR_CHARACTERISTIC_VALUE){
					displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
					handle.connection = evt->data.evt_gatt_procedure_completed.connection;
					GATT_state = GATT_WAITING_FOR_CHARACTERISTIC_INDICATION;
				}

//				else if(GATT_state == GATT_WAITING_FOR_CHARACTERISTIC_INDICATION){
//#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
//					displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
//					displayUpdate();
//#endif
//					GATT_state = GATT_NO_ACTION;
////					break;
//				}
				break;

			case gecko_evt_le_connection_closed_id:
				gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_general_discoverable);

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
		    	displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
#endif
				break;
		}

#endif
 	}
}
