#ifndef __JDSIMPLE_H
#define __JDSIMPLE_H

#define DEVICE_DMESG_BUFFER_SIZE 1024

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#if defined(STM32F0)
#include "stm32f0.h"
#elif defined(STM32G0)
#include "stm32g0.h"
#else
#error "invalid CPU"
#endif

#include "dmesg.h"
#include "pinnames.h"
#include "jdprotocol.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RAM_FUNC __attribute__((noinline, long_call, section(".data")))

typedef void (*cb_t)(void);

// main.c
void panic(void);
void led_toggle(void);
void led_set(int state);
void pulse_log_pin(void);
void set_log_pin(int v);
void set_log_pin2(int v);
void set_log_pin3(int v);
void set_log_pin4(int v);
void set_log_pin5(int v);

// utils.c
void target_enable_irq(void);
void target_disable_irq(void);
void wait_us(int n);
int itoa(int n, char *s);
int string_reverse(char *s);
uint32_t random(void);
uint32_t random_around(uint32_t v);
uint64_t device_id(void);
void seed_random(uint32_t s);

// exti.c
void exti_set_callback(GPIO_TypeDef *port, uint32_t pin, cb_t callback);
void exti_trigger(cb_t cb);
#define exti_disable(pin) LL_EXTI_DisableIT_0_31(pin)
#define exti_enable(pin) LL_EXTI_EnableIT_0_31(pin)
#define exti_enable(pin) LL_EXTI_EnableIT_0_31(pin)
#define exti_clear(pin) LL_EXTI_ClearFallingFlag_0_31(pin)

// tim.c
void tim_init(void);
uint64_t tim_get_micros(void);
void tim_set_timer(int delta, cb_t cb);

// uart.c
void uart_init(void);
int uart_start_tx(const void *data, uint32_t numbytes);
void uart_start_rx(void *data, uint32_t maxbytes);
void uart_disable(void);
void uart_wait_high(void);

// crc.c
uint16_t crc16(const void *data, uint32_t size);

// jdlow.c
void jd_init(void);
void jd_queue_packet(jd_packet_t *pkt);
uint32_t jd_get_num_pending_tx(void);
uint32_t jd_get_free_queue_space(void);
// callbacks from uart.c
void jd_tx_completed(int errCode);
void jd_rx_completed(int dataLeft);
void jd_line_falling(void);

// jdrouting.c
typedef void (*pkt_handler_t)(void *userData, jd_packet_t *pkt);
void jd_register_client(uint32_t serviceClass, pkt_handler_t handler, void *userData);
void jd_register_host(uint32_t serviceClass, pkt_handler_t handler, void *userData);

// dspi.c
void dspi_init(void);
void dspi_tx(const void *data, uint32_t numbytes, cb_t doneHandler);

// pins.c
void pin_set(int pin, int v);
void pin_setup_output(int pin);
void pin_toggle(int pin);
int pin_get(int pin);
// pull: -1, 0, 1
void pin_setup_input(int pin, int pull);
void pin_setup_output_af(int pin, int af);

// adc.c
void adc_init_random(void);

// pwm.c
void pwm_init(uint32_t period, uint32_t duty);
void pwm_set_duty(uint32_t duty);

// jdapp.c
void app_queue_annouce(void);
void app_handle_packet(jd_packet_t *pkt);
void app_process(void);

#ifdef __cplusplus
}
#endif

#endif
