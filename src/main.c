#include "jdsimple.h"

void SystemClock_Config(void);

void led_init() {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Pin = GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOC, GPIO_InitStructure.Pin, GPIO_PIN_RESET);

    GPIO_InitStructure.Pin = GPIO_PIN_7|GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.Pin = GPIO_PIN_15|GPIO_PIN_10|GPIO_PIN_9;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void set_log_pin(int v) {
    if (v)
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
    else
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
}

void set_log_pin2(int v) {
    if (v)
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_1);
    else
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_1);
}

void set_log_pin3(int v) {
    if (v)
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_15);
    else
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_15);
}

void set_log_pin4(int v) {
    if (v)
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_10);
    else
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_10);
}

void set_log_pin5(int v) {
    if (v)
        LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_9);
    else
        LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_9);
}

void pulse_log_pin() {
    set_log_pin(1);
    set_log_pin(0);
}

void led_toggle() {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
}
void led_set(int state) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, state);
}

// uint8_t crcBuf[1024];

int main(void) {
    HAL_Init();
    SystemClock_Config();
    led_init();


#if 0
    int mi = 200, mx = 0, sm = 0;
    for (int i = 0; i < 10000; ++i) {
        int v = random_around(100);
        if (v < mi)
            mi = v;
        if (v > mx)
            mx = v;
        sm += v;
    }
    DMESG("%d-%d av %d", mi, mx, sm / 10000);
#endif

    for (int i = 0; i < 0; ++i) {
        led_toggle();
        HAL_Delay(250);
    }

    jd_init();

#if 0
    uint8_t bufx[5] = {0x00, 0x11, 0x22, 0x33, 0x44};
    DMESG("crc: %x %x", crc16(bufx, 5), crc16soft(bufx, 5));
    DMESG("crc+1: %x %x", crc16(bufx + 1, 4), crc16soft(bufx + 1, 4));

    for (int i = 0; i < sizeof(crcBuf); ++i) {
        crcBuf[i] = random();
    }
    uint64_t t0 = tim_get_micros();
    uint16_t crc = 0;
    for (int i = 0; i < 10; ++i)
    crc += crc16soft(crcBuf, sizeof(crcBuf));
    DMESG("res %x -> %d us", crc, (uint32_t)(tim_get_micros() - t0));
#endif

    uint32_t lastBlink = HAL_GetTick();
    while (1) {
        if (HAL_GetTick() - lastBlink > 300) {
            lastBlink = HAL_GetTick();
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
