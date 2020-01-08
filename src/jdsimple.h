
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

#define JD_SERIAL_PAYLOAD_SIZE 255

typedef struct {
    uint16_t crc;
    uint8_t size;
    uint8_t service_number;
    uint64_t device_identifier;
} __attribute((__packed__)) jd_packet_header_t;

typedef struct {
    jd_packet_header_t header;
    uint8_t data[JD_SERIAL_PAYLOAD_SIZE + 1];
} __attribute((__packed__)) jd_packet_t;

typedef void (*cb_t)(void);

// main.c
void panic(void);
void led_toggle();
void led_set(int state);
void pulse_log_pin();
void set_log_pin(int v);

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
void exti_disable(uint32_t pin);
void exti_enable(uint32_t pin);

// tim.c
void tim_init();
uint64_t tim_get_micros();
void tim_set_timer(int delta, cb_t cb);

// uart.c
void uart_init();
int uart_start_tx(const void *data, uint32_t numbytes);
void uart_start_rx(void *data, uint32_t maxbytes);
void uart_disable();

// crc.c
uint16_t crc16(const void *data, uint32_t size);

// jdlow.c
void jd_init();
void jd_tx_completed(int errCode);
void jd_rx_completed(int dataLeft);
void jd_line_falling();
void jd_queue_packet(jd_packet_t *pkt);

// jdapp.c
void app_queue_annouce();
void app_handle_packet(jd_packet_t *pkt);


#ifdef __cplusplus
}
#endif

#endif
