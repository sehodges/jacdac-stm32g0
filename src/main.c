#include "jdsimple.h"

#ifdef BUTTON_V0
#define PIN_LED PB_3
#define PIN_LED2 PB_4

#define PIN_LOG0 PB_1
#define PIN_LOG1 PA_1
#define PIN_LOG2 PA_0
#define PIN_LOG3 PB_2
#elif defined(JDM_V0)
#define PIN_LED PB_1
#define PIN_LED2 -1
#define PIN_LOG0 -1 // sig_write
#define PIN_LOG1 -1
#define PIN_LOG2 -1 // sig error
#define PIN_LOG3 -1

#define PIN_PWR PA_3
#define PIN_P0 PA_1
#define PIN_P1 PA_2
#define PIN_ASCK PA_5
#define PIN_AMOSI PA_7
#define PIN_SERVO PA_6
#elif defined(JDM_V2)
#define PIN_LED PA_1
#define PIN_LED_GND PA_0
#define PIN_LED2 -1
#define PIN_LOG0 -1 // sig_write
#define PIN_LOG1 -1
#define PIN_LOG2 -1 // sig error
#define PIN_LOG3 -1

#define PIN_PWR PA_3
#define PIN_P0 PA_2
#define PIN_P1 -1
#define PIN_ASCK PA_5
#define PIN_AMOSI PA_7
#define PIN_SERVO PA_6
#else
#define PIN_LED PC_6
#define PIN_LOG0 PA_10
#define PIN_LOG1 PA_9
#endif

static uint64_t led2off;

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
    switch (line) {
    case 4:
        pin_set(PIN_LOG0, v);
        break;
    case 1:
        pin_set(PIN_LOG1, v);
        break;
    case 2:
        if (v) {
            pin_set(PIN_LED2, 1);
            led2off = tim_get_micros() + 5000;
        }
        pin_set(PIN_LOG2, v);
        break;
    case 0:
        // pin_set(PIN_LOG3, v);
        break;
    }
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

#define NUM_PIXELS 10
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

int main(void) {
    led_init();

    tim_init();
    px_init();
    adc_init_random(); // 300b

    tick();

    jd_init();

#ifdef PWM_TEST
    pwm_init(1000, 200);
    int d = 0;
#endif

    // rainbow();

    acc_init();

    uint64_t lastBlink = tim_get_micros();
    while (1) {
        if (tim_get_micros() - lastBlink > 300000) {
            lastBlink = tim_get_micros();
            led_toggle();

            acc_process();

#ifdef PWM_TEST
            d++;
            if (d > 5)
                d = 0;
            pwm_set_duty(d * 100);
            // pulse_log_pin();
#endif
        }

        if (tim_get_micros() > led2off)
            pin_set(PIN_LED2, 0);

        app_process();
    }
}

void jd_panic(void) {
    DMESG("PANIC!");
    target_disable_irq();
    while (1) {
        led_toggle();
        target_wait_us(100000);
    }
}
