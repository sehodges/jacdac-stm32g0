
#ifndef __JDSIMPLE_H
#define __JDSIMPLE_H

#define DEVICE_DMESG_BUFFER_SIZE 1024

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "stm32g0xx_ll_gpio.h"

#include "dmesg.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32g0xx_hal.h"
#include <stdint.h>

void panic(void);
void led_toggle();
void led_set(int state);
void pulse_log_pin();
void set_log_pin(int v);

void target_enable_irq();
void target_disable_irq();
void wait_us(int n);
int itoa(int n, char *s);
int string_reverse(char *s);

typedef void (*cb_t)(void *);
void set_exti_callback(GPIO_TypeDef *port, uint32_t pin, cb_t callback, void *cb_arg);

void tim_init();

void uart_init();
void uart_start_tx(const void *data, uint32_t numbytes);
void uart_start_rx(void *data, uint32_t maxbytes);

#ifdef __cplusplus
}
#endif

#endif
