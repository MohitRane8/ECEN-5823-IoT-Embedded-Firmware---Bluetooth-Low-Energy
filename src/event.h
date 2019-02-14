/*
 * event.h
 *
 *  Created on: Feb 12, 2019
 *      Author: Mohit
 */

#ifndef SRC_EVENT_H_
#define SRC_EVENT_H_

#include "stdbool.h"

/* Defining a struct for events */
struct tempEvents{
	bool UF_flag;
	bool COMP1_flag;
	bool I2CTransactionDone;
	bool I2CTransactionError;
};

#endif /* SRC_EVENT_H_ */
