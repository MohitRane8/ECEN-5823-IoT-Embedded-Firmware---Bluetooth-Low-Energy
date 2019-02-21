/*
 * scheduler.c
 * This file contains the state machine which handles all the events of
 * temperature measurement.
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
	/* Scheduler */
	switch(current_state){
		/* Power Off state */
		case TEMP_SENSOR_POWER_OFF:
			if(TEMP_EVENT.UF_flag){
				TEMP_EVENT.UF_flag = false;
				TEMP_EVENT.NoEvent = true;

				// Enable temperature sensor
				GPIO_PinOutSet(gpioPortD, 15);

				// Setting the timer for the gpio pin
				timerSetEventInUs(80000);

				next_state = TEMP_SENSOR_WAIT_FOR_POWER_UP;
			}
			else{
				LOG_INFO("Error");
			}
			break;

		/* Wait for Power Up state */
		case TEMP_SENSOR_WAIT_FOR_POWER_UP:
			if(TEMP_EVENT.COMP1_flag){
				TEMP_EVENT.COMP1_flag = false;
				TEMP_EVENT.NoEvent = true;

				// sleep block begin
				SLEEP_SleepBlockBegin(sleepEM2);

				//i2c transfer init for write
				tempSensorStartI2CWrite();

				next_state = TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE;
			}
			break;

		/* Wait for I2C Write Complete state */
		case TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE:
			if(TEMP_EVENT.I2CTransactionDone){
				TEMP_EVENT.I2CTransactionDone = false;
				TEMP_EVENT.NoEvent = true;

				tempSensorStartI2CRead();
				next_state = TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE;
			}

			if(TEMP_EVENT.I2CTransactionError){
				TEMP_EVENT.I2CTransactionError = false;
				TEMP_EVENT.NoEvent = true;
				// sleep block end
				SLEEP_SleepBlockEnd(sleepEM2);
				//power off sensor
				GPIO_PinOutClear(gpioPortD, 15);
				next_state = TEMP_SENSOR_I2C_ERROR;
			}
			break;

		/* Wait for I2C Read Complete state */
		case TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE:
			if(TEMP_EVENT.I2CTransactionDone){
				TEMP_EVENT.I2CTransactionDone = false;
				TEMP_EVENT.NoEvent = true;
				// sleep block end
				SLEEP_SleepBlockEnd(sleepEM2);
				//power off sensor
				GPIO_PinOutClear(gpioPortD, 15);
				//displayTemperature
				uint8_t temp[5];
				uint8_t *p = temp;
				uint8_t flags = 0x00;   /* flags set as 0 for Celsius, no time stamp and no temperature type. */
				UINT8_TO_BITSTREAM(p, flags);

				float celtemp;
				celtemp = tempConv();

				uint32_t tempBit = FLT_TO_UINT32((celtemp*1000), -3);
				UINT32_TO_BITSTREAM(p, tempBit);

				// sending the temperature data to app via bluetooth
				gecko_cmd_gatt_server_send_characteristic_notification(0xFF, gattdb_temperature_measurement, 5, temp);
				next_state = TEMP_SENSOR_POWER_OFF;
			}

			if(TEMP_EVENT.I2CTransactionError){
				TEMP_EVENT.I2CTransactionError = false;
				TEMP_EVENT.NoEvent = true;
				// sleep block end
				SLEEP_SleepBlockEnd(sleepEM2);
				//power off sensor
				GPIO_PinOutClear(gpioPortD, 15);
				next_state = TEMP_SENSOR_I2C_ERROR;
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
//			timestamp = loggerGetTimestamp();
//			LOG_INFO("%d: ", timestamp);
		LOG_INFO("Temp sensor transitioned from state %d to state %d\n", current_state, next_state);
		current_state = next_state;
	}
}
