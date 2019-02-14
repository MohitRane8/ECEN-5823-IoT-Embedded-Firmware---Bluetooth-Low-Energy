/************************************************************************
 *	Assignment 3 - I2C Load Power Management
 *	Author: Mohit Rane
 *	Submission Date: February 6th, 2019
 *
 *	In this assignment, we measure the temperature every 3 seconds
 *	using the inbuilt temperature sensor over I2C. The period is
 *	controlled by the LETIMER.
 ************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_core.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

#include "em_device.h"
#include "em_chip.h"
#include "gpio.h"
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif

/* Libraries required by this assignment */
#include "log.h"
#include "letimer.h"
#include "configSLEEP.h"
#include "i2c.h"
#include "event.h"

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config = {
  .config_flags = 0,
  .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
  .bluetooth.max_connections = MAX_CONNECTIONS,
  .bluetooth.heap = bluetooth_stack_heap,
  .bluetooth.heap_size = sizeof(bluetooth_stack_heap),
  .bluetooth.sleep_clock_accuracy = 100, // ppm
  .gattdb = &bg_gattdb_data,
  .ota.flags = 0,
  .ota.device_name_len = 3,
  .ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
  .pa.config_enable = 1, // Enable high power PA
  .pa.input = GECKO_RADIO_PA_INPUT_VBAT, // Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
};


/* Initialization of structure for temperature events */
extern struct tempEvents TEMP_EVENT = {0};

/* Defining states for state machine */
enum temp_sensor_state {
	TEMP_SENSOR_POWER_OFF,
	TEMP_SENSOR_WAIT_FOR_POWER_UP,
	TEMP_SENSOR_WAIT_FOR_I2C_WRITE_COMPLETE,
	TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE,
	TEMP_SENSOR_I2C_ERROR = -1
};

/* Defining the initial state */
enum temp_sensor_state current_state = TEMP_SENSOR_POWER_OFF;
enum temp_sensor_state next_state = TEMP_SENSOR_WAIT_FOR_POWER_UP;

//uint32_t loggerGetTimestamp(void);

uint32_t timestamp;

int main(void)
{
	// Initialize device
	initMcu();

	// Initialize board
	initBoard();

	// Initialize application
	initApp();

	// Log initialization
	logInit();

	// Initialize stack
	gecko_init(&config);

	// Configure Sleep
	configSLEEP();

	// Initialize GPIO
	gpioInit();

	// Initialize I2C
	initI2C();

	// Initialize LETIMER
	initLETIMER();

	/* Infinite loop */
	while(1)
	{
		/* Assignment 4 code */
		EMU_EnterEM3(true);

		/* Scheduler */
		switch(current_state){
			/* Power Off state */
			case TEMP_SENSOR_POWER_OFF:
				if(TEMP_EVENT.UF_flag){
					TEMP_EVENT.UF_flag = false;

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

					tempSensorStartI2CRead();
					next_state = TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE;
				}

				if(TEMP_EVENT.I2CTransactionError){
					TEMP_EVENT.I2CTransactionError = false;
					next_state = TEMP_SENSOR_I2C_ERROR;
				}
				break;

			/* Wait for I2C Read Complete state */
			case TEMP_SENSOR_WAIT_FOR_I2C_READ_COMPLETE:
				if(TEMP_EVENT.I2CTransactionDone){
					TEMP_EVENT.I2CTransactionDone = false;

					// sleep block end
					SLEEP_SleepBlockEnd(sleepEM2);

					//displayTemperature
					tempConv();

					//power off sensor
					GPIO_PinOutClear(gpioPortD, 15);
					next_state = TEMP_SENSOR_POWER_OFF;
				}

				if(TEMP_EVENT.I2CTransactionError){
					TEMP_EVENT.I2CTransactionError = false;
					next_state = TEMP_SENSOR_I2C_ERROR;
				}
				break;

			/* Error state */
			case TEMP_SENSOR_I2C_ERROR:
				LOG_INFO("ERROR\n");
				next_state = TEMP_SENSOR_POWER_OFF;
		}

		if(current_state != next_state){
//			timestamp = loggerGetTimestamp();
//			LOG_INFO("%d: ", timestamp);
			LOG_INFO("Temp sensor transitioned from state %d to state %d\n", current_state, next_state);
			current_state = next_state;
			if(current_state == TEMP_SENSOR_I2C_ERROR) continue;
		}
 	}
}
