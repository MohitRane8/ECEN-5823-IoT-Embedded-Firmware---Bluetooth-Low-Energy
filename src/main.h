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
#include "math.h"
#include "ble_device_type.h"

/* Parameters for setting connection */
#define MIN_INTERVAL 60
#define MAX_INTERVAL 60
#define SLAVE_LATENCY 3
#define TIMEOUT 600

#if DEVICE_IS_BLE_SERVER

/* Parameters required for setting advertise timing */
#define ADV_MIN_INTERVAL 400
#define ADV_MAX_INTERVAL 400

/* Parameters defining minimum and maximum Tx power */
#define TX_MAX 80
#define TX_MIN -260

char passkey[32];
uint8_t passkey_handle;
uint8_t first_time_press = 1;

#endif /* DEVICE_IS_BLE_SERVER */

#if !DEVICE_IS_BLE_SERVER

/* GATT states required for gatt_procedure_completed event */
typedef enum gGATT_state{
	GATT_WAITING_FOR_SERVICE_DISCOVERY,
	GATT_WAITING_FOR_CHARACTERISTICS_DISCOVERY,
	GATT_WAITING_FOR_CHARACTERISTIC_VALUE,
	GATT_NO_ACTION
}gGATT_state_t;

volatile enum gGATT_state GATT_state;

/* Handles required for client in main.c */
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

/* Declaring server address and address received by client */
bd_addr serverBtAddr, recvAddr;
uint8 recvAddrType;

/* Final temperature data to be displayed on LCD */
float tempServer;

float gattUint32ToFloat(const uint8_t *value_start_little_endian);

#endif /* !DEVICE_IS_BLE_SERVER */

#endif /* SRC_MAIN_H_ */
