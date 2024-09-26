/*
 * PollingRoutine.h
 *
 *  Created on: Oct 24, 2023
 *      Author: karl.yamashita
 *
 *
 *      Template
 */

#ifndef INC_POLLINGROUTINE_H_
#define INC_POLLINGROUTINE_H_


/*

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#define Nop() asm(" NOP ")

#include "PollingRoutine.h"

*/
#ifndef __weak
#define __weak __attribute__((weak))
#endif


#define ADC_RESOLUTION (3.3 / 4096)

typedef struct
{
	uint16_t data[2];  // leave as 16
	bool adcRdy;
	uint16_t adcLast;
}ADC_Data_t;


void PollingInit(void);
void PollingRoutine(void);

void Volume_Check(void);
void ADC_Check(void);

void LED_Toggle(void);




#endif /* INC_POLLINGROUTINE_H_ */
