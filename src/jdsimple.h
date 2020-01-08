
#ifndef __JDSIMPLE_H
#define __JDSIMPLE_H

#define DEVICE_DMESG_BUFFER_SIZE 0

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "stm32g0xx_ll_gpio.h"

#include "dmesg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define JD_SERIAL_PAYLOAD_SIZE 255

typedef struct {
    uint16_t crc;
    uint8_t size;
    uint8_t service_number;
    uint64_t device_identifier;
} __attribute((__packed__)) __attribute__ ((aligned (4))) jd_packet_header_t;

typedef struct {
    jd_packet_header_t header;
    uint8_t data[JD_SERIAL_PAYLOAD_SIZE + 1];
} jd_packet_t;

typedef void (*cb_t)(void);

// main.c
void panic(void);
void led_toggle();
void led_set(int state);
void pulse_log_pin();
void set_log_pin(int v);
void set_log_pin2(int v);
void set_log_pin3(int v);
void set_log_pin4(int v);
void set_log_pin5(int v);

// utils.c
void target_enable_irq();
void target_disable_irq();
void wait_us(int n);
int itoa(int n, char *s);
int string_reverse(char *s);
uint32_t random();
uint32_t random_around(uint32_t v);
uint64_t device_id();

// exti.c
void exti_set_callback(GPIO_TypeDef *port, uint32_t pin, cb_t callback);
void exti_trigger(cb_t cb);
#define exti_disable(pin) LL_EXTI_DisableIT_0_31(pin)
#define exti_enable(pin) LL_EXTI_EnableIT_0_31(pin)
#define exti_enable(pin) LL_EXTI_EnableIT_0_31(pin)
#define exti_clear(pin) LL_EXTI_ClearFallingFlag_0_31(pin)

// tim.c
void tim_init();
uint64_t tim_get_micros();
void tim_set_timer(int delta, cb_t cb);

// uart.c
void uart_init();
int uart_start_tx(const void *data, uint32_t numbytes);
void uart_start_rx(void *data, uint32_t maxbytes);
void uart_disable();
void uart_wait_high();

// crc.c
uint16_t crc16(const void *data, uint32_t size);
uint16_t crc16soft(const void *data, uint32_t size);

// jdlow.c
void jd_init();
void jd_tx_completed(int errCode);
void jd_rx_completed(int dataLeft);
void jd_line_falling();
void jd_queue_packet(jd_packet_t *pkt);
uint32_t jd_get_num_pending_tx();
uint32_t jd_get_free_queue_space();

// jdapp.c
void app_queue_annouce();
void app_handle_packet(jd_packet_t *pkt);
void app_process();

#ifdef __cplusplus
}
#endif

#endif
