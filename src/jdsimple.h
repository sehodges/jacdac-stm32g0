
#ifndef __JDSIMPLE_H
#define __JDSIMPLE_H

#define DEVICE_DMESG_BUFFER_SIZE 1024

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "dmesg.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx_hal.h"
#include <stdint.h>

void panic(void);
void led_toggle();
void led_set(int state);

void target_enable_irq();
void target_disable_irq();
int itoa(int n, char *s);
int string_reverse(char *s);

#ifdef __cplusplus
}
#endif

#endif
