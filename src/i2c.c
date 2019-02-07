/*
 * i2c.c
 * This file contains the code for the appropriate initialization parameters required for I2CSPM.
 * It also records the temperature data in Celcius.
 *
 *  Created on: Feb 6, 2019
 *      Author: Mohit Rane
 */

#include "i2c.h"

// Command for temperature sensor module to measure temperature
uint8_t si7021_Command = 0xE3;

/* Initialization of I2C */
void initI2CSPM(void)
{
	I2CSPM_Init_TypeDef i2cspmInit = I2CSPM_INIT_DEFAULT;

	// SCL Pin
	i2cspmInit.sclPin = 10;

	// SDA Pin
	i2cspmInit.sdaPin = 11;

	// SCL Port Location
	i2cspmInit.portLocationScl = 14;

	// SDA Port Location
	i2cspmInit.portLocationSda = 16;
	
	// I2CSPM initialization
	I2CSPM_Init(&i2cspmInit);
}

/* Actual I2C transfer */
void performI2CTransfer(void)
{
	/* Transfer structure */
	I2C_TransferSeq_TypeDef writeSeq;

	writeSeq.addr          = (0x40 << 1);
	writeSeq.flags         = I2C_FLAG_WRITE;
	writeSeq.buf[0].data   = &si7021_Command;
	writeSeq.buf[0].len    = 1;

	/* Initializing I2C transfer */
	status = I2CSPM_Transfer(I2C0, &writeSeq);

	/* Logging errors if present */
	if(status == i2cTransferDone)
	{
		LOG_INFO("Transfer SUCCESS");
	}
	else
	{
		LOG_INFO("Error %d", status);
	}

	/* Transfer structure */
	I2C_TransferSeq_TypeDef readSeq;

	readSeq.addr          = (0x40 << 1);
	readSeq.flags         = I2C_FLAG_READ;
	readSeq.buf[0].data   = &i2c_rxBuffer;
	readSeq.buf[0].len    = 2;

	/* Initializing I2C transfer */
	status = I2CSPM_Transfer(I2C0, &readSeq);

	/* Logging errors if present */
	if(status == i2cTransferDone)
	{
		LOG_INFO("Transfer SUCCESS");
	}
	else
	{
		LOG_INFO("Error %d", status);
	}

	/* Storing the tempearture data read */
	tempData |= (i2c_rxBuffer << 8);
	tempData |= (i2c_rxBuffer);
	
	/* Converting raw temperature data in Celcius */
	celsTemp = ((175.72 * tempData / 65536) - 46.85);
	
	/* Logging temperature data */
	LOG_INFO("%ld", celsTemp);
}
