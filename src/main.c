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
}

void led_toggle() {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
}
void led_set(int state) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, state);
}

void handle_pkt(uint32_t serviceId, const void *data, uint32_t size)
{

}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    led_init();

    uart_init();

    uint32_t k = 0;

    uint32_t lastBlink = HAL_GetTick();
    while (1) {
        if (HAL_GetTick() - lastBlink > 300) {
            lastBlink = HAL_GetTick();
            led_toggle();
            if (k++ > 5) {
                k = 0;
                uart_start_tx("Hello world!", 12);
            }
        }

        // process_packets();
    }
}

void panic(void) {
    while (1) {
        led_toggle();
        HAL_Delay(100);
    }
}
