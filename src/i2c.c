/*
 * i2c.c
 * This file contains the code for the appropriate initialization parameters required for I2C.
 * It also records the temperature data in Celcius.
 *
 *  Created on: Feb 6, 2019
 *      Author: Mohit Rane
 */

#include "i2c.h"


/* Structure for temperature events */
struct tempEvents TEMP_EVENT;

/* Receive buffer to store the temperature data from I2C */
uint8_t i2c_rxBuffer[2] = {0};

/* Command for temperature sensor module to measure temperature */
uint8_t si7021_Command = 0xE3;

/**************************************
 *	I2C0 initialization
 **************************************/
void initI2C(void)
{
	I2CSPM_Init_TypeDef i2cspmInit = I2CSPM_INIT_DEFAULT;

	/* I2CSPM initialization */
	I2CSPM_Init(&i2cspmInit);

	/* Setting the temperature sensor */
	GPIO_PinModeSet(gpioPortD, 15, gpioModePushPull, 0);

	/* Enabling NVIC interrupts for I2C0 */
	NVIC_EnableIRQ(I2C0_IRQn);
}

/**************************************
 *	Initializes transfer function for write
 **************************************/
void tempSensorStartI2CWrite(void)
{
	/* Write transfer structure */
	writeSeq.addr 			= (0x40 << 1);
	writeSeq.flags 			= I2C_FLAG_WRITE;
	writeSeq.buf[0].data 	= &si7021_Command;
	writeSeq.buf[0].len 	= 1;

	I2C_TransferInit(I2C0, &writeSeq);
}

/**************************************
 *	Initializes transfer function for read
 **************************************/
void tempSensorStartI2CRead(void)
{
	/* Read transfer structure */
	readSeq.addr 			= (0x40 << 1);
	readSeq.flags 			= I2C_FLAG_READ;
	readSeq.buf[0].data 	= i2c_rxBuffer;
	readSeq.buf[0].len 		= 2;

	I2C_TransferInit(I2C0, &readSeq);
}

/**************************************
 *	Actual I2C transfer
 **************************************/
float tempConv(void)
{
	/* Storing the temperature data read */
	tempData = (((uint16_t)i2c_rxBuffer[0]) << 8);
	tempData |= (i2c_rxBuffer[1]);
	
	/* Converting raw temperature data in Celsius */
	float celsTemp = ((175.72 * (float)tempData / 65536) - 46.85);
	
	/* Logging temperature data */
	LOG_INFO("Temperature: %0.2f\n", celsTemp);

	return celsTemp;
}

/**************************************
 *	I2C0 Interrupt Handling
 **************************************/
void I2C0_IRQHandler(){
//	__disable_irq();

	I2C_TransferReturn_TypeDef reason = I2C_Transfer(I2C0);

	/* Successful transfer */
	if(reason == i2cTransferDone){
		ext_evt_status |= I2C_TRANSACTION_DONE;
		gecko_external_signal(ext_evt_status);
//		TEMP_EVENT.I2CTransactionDone = true;
//		TEMP_EVENT.NoEvent = false;
	}

	/* Transfer failure */
	else if(reason != i2cTransferInProgress){
		ext_evt_status |= I2C_TRANSACTION_ERROR;
		gecko_external_signal(ext_evt_status);
//		TEMP_EVENT.I2CTransactionError = true;
//		TEMP_EVENT.NoEvent = false;
	}

//	__enable_irq();
}
