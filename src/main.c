#include "jdsimple.h"

#ifdef BUTTON_V0
#define PIN_LED PB_3
#define PIN_LOG0 PA_0 // LED
#define PIN_LOG1 PA_1
#else
#define PIN_LED PC_6
#define PIN_LOG0 PA_10
#define PIN_LOG1 PA_9
#endif

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
    adc_init_random(); // 300b

    tick();

    jd_init();

    pwm_init(1000, 200);
    int d = 0;

    uint64_t lastBlink = tim_get_micros();
    while (1) {
        if (tim_get_micros() - lastBlink > 300000) {
            lastBlink = tim_get_micros();
            led_toggle();

            d++;
            if (d > 5)
                d = 0;
            pwm_set_duty(d * 100);
            pulse_log_pin();
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
