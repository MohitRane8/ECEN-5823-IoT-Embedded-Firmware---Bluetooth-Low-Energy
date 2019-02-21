/*
 * scheduler.c
 * This file contains the state machine which handles all the internal events of
 * temperature measurement.
 *
 * This includes:
 * - temperature measurement every 3 seconds
 * - load power management
 * - writing command to temperature sensor
 * - reading temperature data from sensor
 * - sending temperature data to BLE client
 * - turning off sensor
 *
 *  Created on: Feb 18, 2019
 *      Author: Mohit
 */

#include "scheduler.h"

/* Defining the initial state */
enum temp_sensor_state current_state = TEMP_SENSOR_POWER_OFF;
enum temp_sensor_state next_state = TEMP_SENSOR_WAIT_FOR_POWER_UP;

void scheduler(void)
{
	/* Scheduler / State Machine
	 * 4 states present:
	 * - TEMP_SENSOR_POWER_OFF
	 * - TEMP_SENSOR_WAIT_FOR_POWER_UP
	 * - TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE
	 * - TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE
	 * - TEMP_SENSOR_I2C_ERROR */
	switch(current_state){
		/* Power Off state */
		case TEMP_SENSOR_POWER_OFF:
			if(TEMP_EVENT.UF_flag){
				TEMP_EVENT.UF_flag = false;
				TEMP_EVENT.NoEvent = true;

				/* Enabling temperature sensor */
				GPIO_PinOutSet(gpioPortD, 15);

				/* Setting timer for load power management */
				timerSetEventInUs(80000);

				next_state = TEMP_SENSOR_WAIT_FOR_POWER_UP;
			}
			else{
				LOG_INFO("Error in TEMP_SENSOR_POWER_OFF case\n");
			}
			break;

		/* Wait for Power Up state */
		case TEMP_SENSOR_WAIT_FOR_POWER_UP:
			if(TEMP_EVENT.COMP1_flag){
				TEMP_EVENT.COMP1_flag = false;
				TEMP_EVENT.NoEvent = true;

				/* Blocking sleep in EM2 for I2C transfers */
				SLEEP_SleepBlockBegin(sleepEM2);

				/* Writing command to temperature sensor via I2C */
				tempSensorStartI2CWrite();

				next_state = TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE;
			}
			else{
				LOG_INFO("Error in TEMP_SENSOR_WAIT_FOR_POWER_UP case\n");
			}
			break;

		/* Wait for I2C Write Complete state */
		case TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE:
			if(TEMP_EVENT.I2CTransactionDone){
				TEMP_EVENT.I2CTransactionDone = false;
				TEMP_EVENT.NoEvent = true;

				/* Reading temperature from temperature sensor via I2C */
				tempSensorStartI2CRead();

				next_state = TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE;
			}

			if(TEMP_EVENT.I2CTransactionError){
				TEMP_EVENT.I2CTransactionError = false;
				TEMP_EVENT.NoEvent = true;

				/* Ending sleep block after unsuccessful I2C transaction */
				SLEEP_SleepBlockEnd(sleepEM2);

				/* Turning off temperature sensor */
				GPIO_PinOutClear(gpioPortD, 15);

				next_state = TEMP_SENSOR_I2C_ERROR;
			}

			else{
				LOG_INFO("Error in TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE case\n");
			}
			break;

		/* Wait for I2C Read Complete state */
		case TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE:
			if(TEMP_EVENT.I2CTransactionDone){
				TEMP_EVENT.I2CTransactionDone = false;
				TEMP_EVENT.NoEvent = true;

				/* Ending sleep block after successful I2C transaction */
				SLEEP_SleepBlockEnd(sleepEM2);

				/* Turning off temperature sensor */
				GPIO_PinOutClear(gpioPortD, 15);

				/* Declaring variables for temperature */
				uint8_t temp[5];				// temperature data buffer to be sent to BLE client
				uint8_t *p = temp;
				uint8_t flags = 0x00;   		// flags set as 0 for Celsius, no time stamp and no temperature type.
				UINT8_TO_BITSTREAM(p, flags);	// Appending flags to data buffer
				float celtemp;

				/* Read temperature via I2C */
				celtemp = tempConv();

				uint32_t tempBit = FLT_TO_UINT32((celtemp*1000), -3);	// Converting data type of temperature data
				UINT32_TO_BITSTREAM(p, tempBit);						// Appending temperature data to buffer

				// Sending temperature data to BLE client
				gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_temperature_measurement, 5, temp);

				next_state = TEMP_SENSOR_POWER_OFF;
			}

			if(TEMP_EVENT.I2CTransactionError){
				TEMP_EVENT.I2CTransactionError = false;
				TEMP_EVENT.NoEvent = true;

				/* Ending sleep block after unsuccessful I2C transaction */
				SLEEP_SleepBlockEnd(sleepEM2);

				/* Turning off temperature sensor */
				GPIO_PinOutClear(gpioPortD, 15);

				next_state = TEMP_SENSOR_I2C_ERROR;
			}

			else{
				LOG_INFO("Error in TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE case\n");
			}
			break;

		/* Error state */
		case TEMP_SENSOR_I2C_ERROR:
			TEMP_EVENT.NoEvent = true;
			LOG_INFO("ERROR\n");
			next_state = TEMP_SENSOR_POWER_OFF;
	}

	/* Changing the current state and logging the change */
	if(current_state != next_state){
//		timestamp = loggerGetTimestamp();
//		LOG_INFO("%d: ", timestamp);
		LOG_INFO("Temp sensor transitioned from state %d to state %d\n", current_state, next_state);
		current_state = next_state;
	}
}
