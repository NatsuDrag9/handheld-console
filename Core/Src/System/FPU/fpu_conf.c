/*
 * fpu_conf.c
 *
 *  Created on: Jan 15, 2025
 *      Author: rohitimandi
 */


#include "System/FPU/fpu_conf.h"

void fpu_init() {
	// Enable FPU
	    SCB->CPACR |= ((3UL << 20) | (3UL << 22));
//	    __DSB();
//	    __ISB();
}
