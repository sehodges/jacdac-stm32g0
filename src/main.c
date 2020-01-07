#include "jdsimple.h"

void SystemClock_Config(void);

void led_init() {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.Pin = GPIO_PIN_6;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    HAL_GPIO_WritePin(GPIOC, GPIO_InitStructure.Pin, GPIO_PIN_RESET);

    GPIO_InitStructure.Pin = GPIO_PIN_7;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void set_log_pin(int v) {
    if (v)
        LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_7);
    else
        LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);
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

int main(void) {
    HAL_Init();
    SystemClock_Config();
    led_init();

    for (int i = 0; i < 0; ++i) {
        led_toggle();
        HAL_Delay(250);
    }

    jd_init();

    uint32_t lastBlink = HAL_GetTick();
    char buf[20];
    strcpy(buf, "Hello world!");
    int n = 0;
    while (1) {
        if (HAL_GetTick() - lastBlink > 300) {
            lastBlink = HAL_GetTick();
            led_toggle();
            buf[12] = n++;
            // uart_start_tx(buf, 13);
        }

        // process_packets();
    }
}

void panic(void) {
    DMESG("PANIC!");
    while (1) {
        led_toggle();
        HAL_Delay(100);
    }
}
