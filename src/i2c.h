/*
 * i2c.h
 * Header file required for i2c.c
 *
 *  Created on: Feb 6, 2019
 *      Author: Mohit
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "i2cspm.h"
#include "log.h"
#include "em_i2c.h"

/* Receive buffer to store the temperature data from I2C */

/* Stores raw temperature data */
uint16_t tempData;

/* Stores the temperature data in celcius */
uint16_t celsTemp;

I2C_TransferReturn_TypeDef status;

void initI2CSPM(void);
void performI2CTransfer(void);

#endif /* SRC_I2C_H_ */
