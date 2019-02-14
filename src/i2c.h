/*
 * i2c.h
 * Header file required for i2c.c
 *
 *  Created on: Feb 6, 2019
 *      Author: Mohit
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "log.h"
#include "em_i2c.h"
#include "event.h"
#include "em_gpio.h"
#include "i2cspm.h"

/* Stores raw temperature data */
uint16_t tempData;

/* Stores the temperature data in celcius */
uint16_t celsTemp;

I2C_TransferReturn_TypeDef status;

void initI2C(void);
void tempSensorStartI2CWrite(void);
void tempSensorStartI2CRead(void);
void tempConv(void);

#endif /* SRC_I2C_H_ */
