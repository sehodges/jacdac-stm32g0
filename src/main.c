#include "jdsimple.h"

#define PIN_LED PC_6
#define PIN_LOG0 PA_10
#define PIN_LOG1 PA_9

void led_init() {
    pin_setup_output(PIN_LOG0);
    pin_setup_output(PIN_LOG1);
    pin_setup_output(PIN_LED);
}

void set_log_pin(int v) {
    pin_set(PIN_LOG0, v);
}

void set_log_pin2(int v) {
    pin_set(PIN_LOG1, v);
}

void set_log_pin3(int v) {}
void set_log_pin4(int v) {}
void set_log_pin5(int v) {}

#ifdef STM32G0
  const uint32_t AHBPrescTable[16UL] = {0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 0UL, 1UL, 2UL, 3UL, 4UL, 6UL, 7UL, 8UL, 9UL};
  const uint32_t APBPrescTable[8UL] =  {0UL, 0UL, 0UL, 0UL, 1UL, 2UL, 3UL, 4UL};
#endif

void pulse_log_pin() {
    set_log_pin(1);
    set_log_pin(0);
}

void led_toggle() {
    pin_toggle(PIN_LED);
}

void led_set(int state) {
    pin_set(PIN_LED, state);
}

// uint8_t crcBuf[1024];

static void tick() {
    // pulse_log_pin();
    tim_set_timer(10000, tick);
}

int main(void) {
    led_init();

    tim_init();
    dspi_init();
    adc_init_random();

    tick();

    jd_init();

    uint64_t lastBlink = tim_get_micros();
    while (1) {
        if (tim_get_micros() - lastBlink > 300000) {
            lastBlink = tim_get_micros();
            led_toggle();
        }

        app_process();
    }
}

void panic(void) {
    DMESG("PANIC!");
    target_disable_irq();
    while (1) {
        led_toggle();
        wait_us(100000);
    }
}
