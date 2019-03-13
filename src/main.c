/************************************************************************
 *	Assignment 7 - Bluetooth BLE Client
 *	Author: Mohit Rane
 *	Submission Date: February 27th, 2019
 *
 *	FOR TESTING:
 *	Change #define DEVICE_IS_BLE_SERVER in ble_device_type.h to upload
 *	code to appropriate device (server/client). Set the server address
 *	in ble_device_type.h that the client will connect to.
 *
 *	DESCRIPTION:
 *	In this assignment, we measure the temperature every 3 seconds
 *	using the inbuilt temperature sensor over I2C on server. The period
 *	is controlled by the LETIMER.
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
 *	#if DEVICE_IS_BLE_SERVER statements used in main.c, main.h, scheduler.c
 *	scheduler.h, i2c.c, i2c.h and letimer.c
 *
 *	REFERENCES:
 *	1. soc-thermometer Software Example by Silicon Labs
 *	2. https://www.silabs.com/community/wireless/bluetooth/knowledge-
 *	   base.entry.html/2016/10/04/scheduling_applicati-ERXS
 *  3. https://www.silabs.com/community/wireless/bluetooth/forum.topic
 *     .html/bgm121_i_cannot_exp-B8Ae
 *  4. https://www.silabs.com/community/wireless/bluetooth/forum.topic
 *     .html/how_to_enable_encryp-rDLW
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

#if DEVICE_IS_BLE_SERVER
int8_t rssi;

/* Flag which is set upon successful connection
 * Indicates the system to do the temperature reading process */
extern bool ble_connection_flag;
#endif

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

	// Initialize LETIMER
	initLETIMER();

#if DEVICE_IS_BLE_SERVER
	// Initialize I2C
	initI2C();

	// Configure Sleep
	configSLEEP();

	// Setting initial scheduler event as no event
	TEMP_EVENT.NoEvent = true;

	// Blocking sleep according to the mode defined
#if ((ENERGYMODE == 0) | (ENERGYMODE == 1) | (ENERGYMODE == 2))
	SLEEP_SleepBlockBegin(ENERGYMODE+1);
#endif
#endif /* DEVICE_IS_BLE_SERVER */

#if !DEVICE_IS_BLE_SERVER
handle.connection = 0;
handle.service = 0;
handle.characteristic = 0;
GATT_state = GATT_WAITING_FOR_SERVICE_DISCOVERY;
#endif

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
	// Initializes LCD display
	displayInit();
	displayPrintf(DISPLAY_ROW_NAME, BLE_DEVICE_TYPE_STRING);
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
				// Delete previous bondings
				gecko_cmd_sm_delete_bondings();

				// Configuring the security settings
				gecko_cmd_sm_configure(0x01, sm_io_capability_displayyesno);

				gecko_cmd_sm_set_bondable_mode(1);

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

			case gecko_evt_sm_confirm_passkey_id:
				passkey_handle = evt->data.evt_sm_confirm_bonding.connection;

				// Store the passkey for display
				sprintf(passkey, "%lu", evt->data.evt_sm_passkey_display.passkey);

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
		    	  displayPrintf(DISPLAY_ROW_PASSKEY, "Passkey: %s", passkey);
		    	  displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
#endif
		    	// User can confirm the passkey by pressing the PB0 button
		    	// Button press handled in interrupt
		    	break;

			case gecko_evt_le_connection_opened_id:
				/* Setting connection parameters for connection */
				BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_set_parameters(evt->data.evt_le_connection_opened.connection, MIN_INTERVAL, MAX_INTERVAL, SLAVE_LATENCY, TIMEOUT));

				/* Setting connection flag to start state machine based on external events */
				ble_connection_flag = true;

				/* Enhance the security of a connection to current security requirements */
				gecko_cmd_sm_increase_security(evt->data.evt_le_connection_opened.connection);

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				struct gecko_msg_system_get_bt_address_rsp_t * rsp;
				bd_addr addr;

				rsp = gecko_cmd_system_get_bt_address();
				addr = rsp->address;
				displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x", addr.addr[0],addr.addr[1],addr.addr[2],addr.addr[3],addr.addr[4],addr.addr[5]);
				displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
#endif
				break;

			case gecko_evt_sm_bonded_id:
#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
#endif
				break;

			case gecko_evt_sm_bonding_failed_id:
#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				displayPrintf(DISPLAY_ROW_CONNECTION, "Bonding Failed");
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

				if (((evt->data.evt_system_external_signal.extsignals) & PB0_FLAG) != 0) {
					gecko_cmd_sm_passkey_confirm(passkey_handle, 1);

					/* Enhance the security of a connection to current security requirements */
					gecko_cmd_sm_increase_security(evt->data.evt_le_connection_opened.connection);

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
					displayPrintf(DISPLAY_ROW_PASSKEY, " ");
					displayPrintf(DISPLAY_ROW_ACTION, "Passkey Confirmed");
#endif
				}
				break;

			case gecko_evt_le_connection_closed_id:
				gecko_cmd_system_set_tx_power(0);
				/* Restart advertising after client has disconnected */
				gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);

				/* Turning the connection flag to stop the system from taking temperature */
				ble_connection_flag = false;

#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
				displayPrintf(DISPLAY_ROW_BTADDR, " ");
				displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
				displayPrintf(DISPLAY_ROW_PASSKEY, " ");
				displayPrintf(DISPLAY_ROW_ACTION, " ");
				displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
#endif
				break;
		}

#else
		/* DEVICE IS CLIENT */
		evt = gecko_wait_event();

		gecko_update(evt);

		switch BGLIB_MSG_ID(evt->header) {

			/* System boot case */
			case gecko_evt_system_boot_id:
				gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_general_discoverable);

				#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
					struct gecko_msg_system_get_bt_address_rsp_t * rsp;
					bd_addr addr;
					rsp = gecko_cmd_system_get_bt_address();
					addr = rsp->address;
					displayPrintf(DISPLAY_ROW_BTADDR, "%x:%x:%x:%x:%x:%x", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3], addr.addr[4], addr.addr[5]);

					displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
				#endif

		    	LOG_INFO("EVT -> System Boot\n");
				break;

			/* Scan response case */
			case gecko_evt_le_gap_scan_response_id:
				// This event reports any advertising or scan response packet that is received by the device's radio while in scanning mode

				// Address and address type of received advertisement
				recvAddr = evt->data.evt_le_gap_scan_response.address;
				recvAddrType = evt->data.evt_le_gap_scan_response.address_type;

				// Connecting to server when address matches
				if((recvAddr.addr[0] == SERVER_BT_ADDRESS(0)) &
					(recvAddr.addr[1] == SERVER_BT_ADDRESS(1)) &
					(recvAddr.addr[2] == SERVER_BT_ADDRESS(2)) &
					(recvAddr.addr[3] == SERVER_BT_ADDRESS(3)) &
					(recvAddr.addr[4] == SERVER_BT_ADDRESS(4)) &
					(recvAddr.addr[5] == SERVER_BT_ADDRESS(5))){

					// Stopping discovery of adveritising packets
					gecko_cmd_le_gap_end_procedure();
					displayPrintf(DISPLAY_ROW_BTADDR2, "%x:%x:%x:%x:%x:%x", recvAddr.addr[0], recvAddr.addr[1], recvAddr.addr[2], recvAddr.addr[3], recvAddr.addr[4], recvAddr.addr[5]);

					// Connecting to desired server
					gecko_cmd_le_gap_connect(recvAddr, recvAddrType, le_gap_phy_1m);
				}

				LOG_INFO("EVT -> Scan Response --- Received Address -> %x:%x:%x:%x:%x:%x\n", recvAddr.addr[0], recvAddr.addr[1], recvAddr.addr[2], recvAddr.addr[3], recvAddr.addr[4], recvAddr.addr[5]);
				break;

			/* Connection open case */
			case gecko_evt_le_connection_opened_id:
				// Setting connection parameters for connection
				gecko_cmd_le_connection_set_parameters(evt->data.evt_le_connection_opened.connection, MIN_INTERVAL, MAX_INTERVAL, SLAVE_LATENCY, TIMEOUT);

				handle.connection = evt->data.evt_le_connection_opened.connection;

				// Health Thermometer service UUID: 1809
				HTM_service.data[0] = 0x09;
				HTM_service.data[1] = 0x18;
				HTM_service.size = 2;

				// Discovering remote services
				gecko_cmd_gatt_discover_primary_services_by_uuid(handle.connection, HTM_service.size, HTM_service.data);

				GATT_state = GATT_WAITING_FOR_SERVICE_DISCOVERY;

				#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
					displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
				#endif

				LOG_INFO("EVT -> Connection Open\n");
				break;

			/* GATT service case */
			case gecko_evt_gatt_service_id:
				// Getting service handle
				handle.service = evt->data.evt_gatt_service.service;
				LOG_INFO("EVT -> GATT Service --- Service ID: %d\n", handle.service);
				break;

			/* GATT characteristic case */
			case gecko_evt_gatt_characteristic_id:
				// Getting characteristic handle
				handle.characteristic = evt->data.evt_gatt_characteristic.characteristic;
				LOG_INFO("EVT -> GATT Characteristic --- Characteristic ID: %d\n", handle.characteristic);
				break;

			/* Characteristic (temperature) value case */
			case gecko_evt_gatt_characteristic_value_id:
				handle.connection = evt->data.evt_gatt_characteristic.connection;

				// Sending confirmation to server when data is received
				if(evt->data.evt_gatt_characteristic_value.att_opcode == gatt_handle_value_indication)
					gecko_cmd_gatt_send_characteristic_confirmation(handle.connection);

				// Conversion of received temperature data from uint32 to float
				uint8_t *tempServerByte = evt->data.evt_gatt_characteristic_value.value.data;
				tempServerByte++;
				tempServer = gattUint32ToFloat(tempServerByte);

				#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
					displayPrintf(DISPLAY_ROW_TEMPVALUE, "%.2f", tempServer);
				#endif

				LOG_INFO("EVT -> GATT Characteristic Value --- Temperature: %.2f\n", tempServer);
				break;

			/* GATT procedure complete case */
			case gecko_evt_gatt_procedure_completed_id:
				LOG_INFO("EVT -> GATT Procedure Complete --- GATT State: %d\n", GATT_state);
				if(GATT_state == GATT_WAITING_FOR_SERVICE_DISCOVERY){
					handle.connection = evt->data.evt_gatt_procedure_completed.connection;

					// Temperature Measurement characteristic UUID: 2A1C
					HTM_characteristic.data[0] = 0x1C;
					HTM_characteristic.data[1] = 0x2A;
					HTM_characteristic.size = 2;

					// Discovering remote characteristics
					gecko_cmd_gatt_discover_characteristics_by_uuid(handle.connection, handle.service, HTM_characteristic.size, HTM_characteristic.data);

					GATT_state = GATT_WAITING_FOR_CHARACTERISTICS_DISCOVERY;
				}

				else if(GATT_state == GATT_WAITING_FOR_CHARACTERISTICS_DISCOVERY){
					handle.connection = evt->data.evt_gatt_procedure_completed.connection;

					// Enabling characteristic notification
					gecko_cmd_gatt_set_characteristic_notification(handle.connection, handle.characteristic, gatt_indication);
					GATT_state = GATT_WAITING_FOR_CHARACTERISTIC_VALUE;
				}

				else if(GATT_state == GATT_WAITING_FOR_CHARACTERISTIC_VALUE){
					displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
					GATT_state = GATT_NO_ACTION;
				}
				break;

				/* Connection close case */
			case gecko_evt_le_connection_closed_id:
				// Starting discovery of advertising packets
				gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_general_discoverable);

				#if ECEN5823_INCLUDE_DISPLAY_SUPPORT
		    		displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
		    		displayPrintf(DISPLAY_ROW_BTADDR2, " ");
		    		displayPrintf(DISPLAY_ROW_TEMPVALUE, " ");
				#endif

		    	LOG_INFO("EVT -> Connection Close\n\n");
				break;
		}

#endif
 	}
}

/**
 * @return a float value based on a UINT32 value written by FLT_TO_UINT32 and
 * UINT32_TO_BITSTREAM
 * @param value_start_little_endian is a pointer to the first byte of the float
 * which is represented in UINT32 format from FLT_TO_UINT32/UINT32_TO_BITSTREAM
 */
float gattUint32ToFloat(const uint8_t *value_start_little_endian)
{
    int8_t exponent = (int8_t)value_start_little_endian[3];
    uint32_t mantissa = value_start_little_endian[0] +
                        (((uint32_t)value_start_little_endian[1]) << 8) +
                        (((uint32_t)value_start_little_endian[2]) << 16);
    return ((float)mantissa*pow(10,exponent));
}

