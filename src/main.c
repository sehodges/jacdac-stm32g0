#include "jdsimple.h"

static const uint8_t output_pins[] = {
    PIN_LOG0, PIN_LOG1,     PIN_LOG2,    PIN_LOG3,    PIN_LED,    PIN_LED2,    PIN_PWR,
    PIN_P0,   PIN_P1,       PIN_ASCK,    PIN_AMOSI,   PIN_SERVO,  PIN_LED_GND, PIN_GLO0,
    PIN_GLO1, PIN_ACC_MOSI, PIN_ACC_SCK, PIN_ACC_VCC, PIN_ACC_CS,
};

static const uint8_t ain_pins[] = {
    PIN_GLO_SENSE0, PIN_GLO_SENSE1, PIN_ACC_MISO, PIN_UNUSED0,
    PIN_UNUSED1,    PIN_UNUSED2,    PIN_UNUSED3,  PIN_UNUSED4,
};

void led_init() {
    for (unsigned i = 0; i < sizeof(output_pins); ++i)
        pin_setup_output(output_pins[i]);

    for (unsigned i = 0; i < sizeof(ain_pins); ++i)
        pin_setup_analog_input(ain_pins[i]);

    pin_set(PIN_PWR, 1); // PWR is reverse polarity

    //__HAL_RCC_GPIOC_CLK_DISABLE();
    //__HAL_RCC_GPIOF_CLK_DISABLE();
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
    led_set(1);

    alloc_init();

    tim_init();
    px_init();
    adc_init_random(); // 300b

    jd_init();

    rtc_init();

    app_init_services();

    led_off_time = tim_get_micros() + 200000;

    // we run without sleep at the beginning to allow connecting debugger
    uint64_t start_time = led_off_time + 3000000;

    while (1) {
        uint64_t now_long = tim_get_micros();
        now = (uint32_t)now_long;

        if (led_off_time && now_long > led_off_time) {
            led_off_time = 0;
            led_set(0);
        }

        if (start_time && now_long > start_time) {
            start_time = 0;
        }

        app_process();

        if (!led_off_time && !start_time && now > 3000000) {
            rtc_sleep();
        }
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
