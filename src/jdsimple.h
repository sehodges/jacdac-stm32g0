#ifndef __JDSIMPLE_H
#define __JDSIMPLE_H

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

#include "board.h"
#include "dmesg.h"
#include "pinnames.h"
#include "jdlow.h"
#include "services.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RAM_FUNC __attribute__((noinline, long_call, section(".data")))

// main.c
void led_toggle(void);
void led_set(int state);
void led_blink(int ms);

// utils.c
int itoa(int n, char *s);
int string_reverse(char *s);
uint64_t device_id(void);
RAM_FUNC
void target_wait_cycles(int n);

// exti.c
void exti_set_callback(GPIO_TypeDef *port, uint32_t pin, cb_t callback);
void exti_trigger(cb_t cb);
#define exti_disable(pin) LL_EXTI_DisableIT_0_31(pin)
#define exti_enable(pin) LL_EXTI_EnableIT_0_31(pin)
#define exti_enable(pin) LL_EXTI_EnableIT_0_31(pin)
#define exti_clear(pin) LL_EXTI_ClearFallingFlag_0_31(pin)

// jdrouting.c
typedef void (*pkt_handler_t)(void *userData, jd_packet_t *pkt);
void jd_register_client(uint32_t serviceClass, pkt_handler_t handler, void *userData);
void jd_register_host(uint32_t serviceClass, pkt_handler_t handler, void *userData);

// dspi.c
void dspi_init(void);
void dspi_tx(const void *data, uint32_t numbytes, cb_t doneHandler);
void px_init(void);
void px_tx(const void *data, uint32_t numbytes, cb_t doneHandler);
void px_set(const void *data, uint32_t index, uint32_t color);

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
uint8_t pwm_init(uint8_t pin, uint32_t period, uint32_t duty);
void pwm_set_duty(uint8_t pwm_id, uint32_t duty);

// jdapp.c
void app_process(void);

// txq.c
void txq_flush(void);
int txq_is_idle(void);
void *txq_push(unsigned service_num, unsigned service_cmd, unsigned service_arg, const void *data,
               unsigned service_size);

extern uint32_t now;

// check if given timestamp is already in the past, regardless of overflows on 'now'
// the moment has to be no more than ~500 seconds in the past
static inline bool in_past(uint32_t moment) {
    return ((now - moment) >> 29) == 0;
}
static inline bool in_future(uint32_t moment) {
    return !in_past(moment);
}

// keep sampling at period, using state at *sample
bool should_sample(uint32_t *sample, uint32_t period);

// sensor helpers
struct sensor_state {
    uint16_t status;
    uint32_t sample_interval;
    uint32_t next_sample;
};
typedef struct sensor_state sensor_state_t;

#define SENSOR_STREAMING 0x01

int sensor_handle_packet(sensor_state_t *state, jd_packet_t *pkt);
int sensor_should_stream(sensor_state_t *state);


    // services
#define ACC_SERVICE_NUM 1
void acc_init(void);
void acc_process(void);
void acc_handle_packet(jd_packet_t *pkt);

#ifdef __cplusplus
}
#endif

#endif
