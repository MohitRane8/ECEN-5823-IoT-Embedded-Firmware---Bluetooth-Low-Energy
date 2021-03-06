/*
 * i2c.h
 * Header file required for i2c.c
 *
 *  Created on: Feb 6, 2019
 *      Author: Mohit
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "ble_device_type.h"

#if DEVICE_IS_BLE_SERVER

#include "log.h"
#include "em_i2c.h"
#include "scheduler.h"
#include "em_gpio.h"
#include "i2cspm.h"
#include "configSLEEP.h"

/* Stores raw temperature data */
uint16_t tempData;

/* Stores the temperature data in celcius */
//uint32_t celsTemp;

I2C_TransferReturn_TypeDef status;
I2C_TransferSeq_TypeDef readSeq;
I2C_TransferSeq_TypeDef writeSeq;

void initI2C(void);
void tempSensorStartI2CWrite(void);
void tempSensorStartI2CRead(void);
float tempConv(void);

#endif /* DEVICE_IS_BLE_SERVER */

#endif /* SRC_I2C_H_ */
