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
#include "letimer.h"
#include "configSLEEP.h"
#include "i2c.h"
#include "scheduler.h"
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

#endif /* SRC_MAIN_H_ */
