/*
 * scheduler.h
 *
 *  Created on: Feb 18, 2019
 *      Author: Mohit
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

//#include "event.h";
#include "stdbool.h"
#include "configSLEEP.h"
#include "log.h"
#include "letimer.h"
#include "i2c.h"

void scheduler(void);

/* Defining a struct for events */
struct tempEvents{
	bool UF_flag;
	bool COMP1_flag;
	bool I2CTransactionDone;
	bool I2CTransactionError;
	bool NoEvent;
} TEMP_EVENT;

//#define NO_EVENT 0x01
#define UF_FLAG 0x01
#define COMP1_FLAG 0x02
#define I2C_TRANSACTION_DONE 0x04
#define I2C_TRANSACTION_ERROR 0x08

//uint32_t ext_evt_status;

/* Defining states for state machine */
enum temp_sensor_state {
	TEMP_SENSOR_POWER_OFF,
	TEMP_SENSOR_WAIT_FOR_POWER_UP,
	TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE,
	TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE,
	TEMP_SENSOR_I2C_ERROR = -1
};

#endif /* SRC_SCHEDULER_H_ */
