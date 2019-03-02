/*
 * main.h
 * This file contains the header files and definitions required by the assignment
 *
 *  Created on: Feb 20, 2019
 *      Author: Mohit
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

#include "log.h"

#if DEVICE_IS_BLE_SERVER
#include "letimer.h"
#include "configSLEEP.h"
#include "i2c.h"
#include "scheduler.h"
#endif

#include "gecko_ble_errors.h"
#include "gatt_db.h"
#include "display.h"
#include "ble_device_type.h"

/* Parameters required for setting advertise timing */
#define ADV_MIN_INTERVAL 400
#define ADV_MAX_INTERVAL 400

/* Parameters defining minimum and maximum Tx power */
#define TX_MAX 80
#define TX_MIN -260

/* Parameters for setting connection */
#define MIN_INTERVAL 60
#define MAX_INTERVAL 60
#define SLAVE_LATENCY 3
#define TIMEOUT 600

#if !DEVICE_IS_BLE_SERVER
typedef enum gGATT_state{
	GATT_NO_ACTION,
	GATT_WAITING_FOR_SERVICE_DISCOVERY,
	GATT_WAITING_FOR_CHARACTERISTICS_DISCOVERY,
	GATT_WAITING_FOR_CHARACTERISTIC_VALUE,
	GATT_WAITING_FOR_CHARACTERISTIC_INDICATION
}gGATT_state_t;

volatile enum gGATT_state GATT_state;

struct allHandles{
	uint8_t connection;
	uint32_t service;
	uint16_t characteristic;
}handle = {0};

/* A struct to store service UUID info */
struct myServices{
	uint8_t data[2];
	uint8_t size;
} HTM_service;

/* A struct to store characteristics UUID info */
struct myCharacteristics{
	uint8_t data[2];
	uint8_t size;
} HTM_characteristic;

#endif

#endif /* SRC_MAIN_H_ */
