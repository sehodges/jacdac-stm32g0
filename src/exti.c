#include "jdsimple.h"
#include "stm32g0xx_ll_exti.h"

static cb_t callbacks[16];

static void check_line(int ln) {
    uint32_t pin = 1 << ln;

    if (LL_EXTI_IsActiveFallingFlag_0_31(pin) != RESET) {
        LL_EXTI_ClearFallingFlag_0_31(pin);
        callbacks[ln]();
    }
}

void EXTI0_1_IRQHandler() {
    check_line(0);
    check_line(1);
}
void EXTI2_3_IRQHandler() {
    check_line(2);
    check_line(3);
}
void EXTI4_15_IRQHandler() {
    for (int i = 4; i <= 15; ++i)
        check_line(i);
}

void exti_disable(uint32_t pin) {
    LL_EXTI_DisableIT_0_31(pin);
}

void exti_enable(uint32_t pin) {
    LL_EXTI_ClearFallingFlag_0_31(pin);
    LL_EXTI_EnableIT_0_31(pin);
}

void exti_set_callback(GPIO_TypeDef *port, uint32_t pin, cb_t callback) {
    uint32_t extiport = 0;

    if (port == GPIOA)
        extiport = LL_EXTI_CONFIG_PORTA;
    else if (port == GPIOB)
        extiport = LL_EXTI_CONFIG_PORTB;
    else if (port == GPIOC)
        extiport = LL_EXTI_CONFIG_PORTC;
    else
        panic();

    int numcb = 0;
    for (uint32_t pos = 0; pos < 16; ++pos) {
        if (callbacks[pos])
            numcb++;
        if (pin & (1 << pos)) {
            uint32_t line = (pos >> 2) | ((pos & 3) << 19);
            LL_EXTI_SetEXTISource(extiport, line);
            LL_EXTI_EnableIT_0_31(1 << pos);
            LL_EXTI_EnableFallingTrig_0_31(1 << pos);
            callbacks[pos] = callback;
        }
    }

    if (!numcb) {
        NVIC_SetPriority(EXTI0_1_IRQn, 3);
        NVIC_EnableIRQ(EXTI0_1_IRQn);
        NVIC_SetPriority(EXTI2_3_IRQn, 3);
        NVIC_EnableIRQ(EXTI2_3_IRQn);
        NVIC_SetPriority(EXTI4_15_IRQn, 3);
        NVIC_EnableIRQ(EXTI4_15_IRQn);
    }
}