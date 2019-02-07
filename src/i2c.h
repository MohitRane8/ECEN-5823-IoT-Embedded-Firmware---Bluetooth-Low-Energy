/*
 * i2c.h
 *
 *  Created on: Feb 6, 2019
 *      Author: Mohit
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "i2cspm.h"
#include "log.h"
#include "em_i2c.h"

uint8_t i2c_rxBuffer;
uint16_t tempData;
uint16_t celsTemp;

I2C_TransferReturn_TypeDef status;

void initI2CSPM(void);
void performI2CTransfer(void);

#endif /* SRC_I2C_H_ */
