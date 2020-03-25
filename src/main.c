#include "jdsimple.h"

void led_init() {
    pin_setup_output(PIN_LOG0);
    pin_setup_output(PIN_LOG1);
    pin_setup_output(PIN_LOG2);
    pin_setup_output(PIN_LOG3);
    pin_setup_output(PIN_LED);
    pin_setup_output(PIN_LED2);

    pin_setup_output(PIN_PWR);
    pin_setup_output(PIN_P0);
    pin_setup_output(PIN_P1);
    pin_setup_output(PIN_ASCK);
    pin_setup_output(PIN_AMOSI);
    pin_setup_output(PIN_SERVO);
    pin_setup_output(PIN_LED_GND);

    pin_set(PIN_PWR, 1); // PWR is reverse polarity
}

void log_pin_set(int line, int v) {
#ifdef BUTTON_V0
    switch (line) {
    case 4:
        pin_set(PIN_LOG0, v);
        break;
    case 1:
        pin_set(PIN_LOG1, v);
        break;
    case 2:
        pin_set(PIN_LOG2, v);
        break;
    case 0:
        // pin_set(PIN_LOG3, v);
        break;
    }
#else
// do nothing
#endif
}

static uint64_t led_off_time;

void led_toggle() {
    pin_toggle(PIN_LED);
}

void led_set(int state) {
    pin_set(PIN_LED, state);
}

void led_blink(int us) {
    led_off_time = tim_get_micros() + us;
    led_set(1);
}

#if 0
#define NUM_PIXELS 14
static uint32_t px_buffer[(NUM_PIXELS * 9 + 8) / 4];

void rainbow() {
    static int cnt = 0;
    static uint8_t delay;

    pin_set(PIN_PWR, 0); // PWR is reverse polarity
    if (delay++ > 70) {
        cnt++;
        delay = 0;
    }
    if (cnt > NUM_PIXELS + 5)
        cnt = 0;
    for (int i = 0; i < NUM_PIXELS; ++i)
        px_set(px_buffer, i, i < cnt ? 0x0f0000 : 0x000f00);

    px_tx(px_buffer, sizeof(px_buffer), rainbow);
}
#endif

int main(void) {
    led_init();

    alloc_init();

    tim_init();
    px_init();
    adc_init_random(); // 300b

    jd_init();

    app_init_services();

    while (1) {
        if (led_off_time && tim_get_micros() > led_off_time) {
            led_off_time = 0;
            led_set(0);
        }
        app_process();
    }
}

void jd_panic(void) {
    DMESG("PANIC!");
    target_disable_irq();
    while (1) {
        led_toggle();
        target_wait_us(70000);
    }
}

void fail_and_reset() {
    DMESG("FAIL!");
    target_disable_irq();
    for (int i = 0; i < 20; ++i) {
        led_toggle();
        target_wait_us(70000);
    }
    NVIC_SystemReset();
}
