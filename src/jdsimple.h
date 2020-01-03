
#ifndef __JDSIMPLE_H
#define __JDSIMPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx_hal.h"
#include <stdint.h>


void panic(void);
void led_toggle();
void led_set(int state);


#ifdef __cplusplus
}
#endif

#endif
