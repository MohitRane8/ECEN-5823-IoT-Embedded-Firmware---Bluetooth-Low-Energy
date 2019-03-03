/*
 * ble_device_type.h
 *
 *  Created on: Feb 16, 2019
 *      Author: danwa
 */

#ifndef SRC_BLE_DEVICE_TYPE_H_
#define SRC_BLE_DEVICE_TYPE_H_
#include <stdbool.h>
#include <stdint.h>

/**
 * Set to 1 to configure this build as a BLE server.
 * Set to 0 to configure as a BLE client
 */
#define DEVICE_IS_BLE_SERVER 1

/* Set address of desired server to connect to */
#define SERV_ADDR0 0x3d
#define SERV_ADDR1 0x62
#define SERV_ADDR2 0x08
#define SERV_ADDR3 0x6f
#define SERV_ADDR4 0x0d
#define SERV_ADDR5 0x00

#define SERVER_BT_ADDRESS(X) SERV_ADDR##X

#if DEVICE_IS_BLE_SERVER
#define BUILD_INCLUDES_BLE_SERVER 1
#define BUILD_INCLUDES_BLE_CLIENT 0
#define BLE_DEVICE_TYPE_STRING "Server"
static inline bool IsServerDevice() { return true; }
static inline bool IsClientDevice() { return false; }
#else
#define BUILD_INCLUDES_BLE_SERVER 0
#define BUILD_INCLUDES_BLE_CLIENT 1
#define BLE_DEVICE_TYPE_STRING "Client"
static inline bool IsClientDevice() { return true;}
static inline bool IsServerDevice() { return false; }
#endif

#endif /* SRC_BLE_DEVICE_TYPE_H_ */
