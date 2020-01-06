#include "jdsimple.h"
#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_utils.h"

void set_timer(int delta) {
    LL_TIM_OC_SetCompareCH1(TIM1, TIM1->CNT + delta);
}

void tim_init() {
    uint32_t tim_prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 1000000);
    // uint32_t TimOutClock = SystemCoreClock / 1;
    // uint32_t tim_period = __LL_TIM_CALC_ARR(TimOutClock, tim_prescaler, 10000);

    LL_TIM_InitTypeDef TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);

    /* TIM1 interrupt Init */
    NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);
    NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

    NVIC_SetPriority(TIM1_CC_IRQn, 0);
    NVIC_EnableIRQ(TIM1_CC_IRQn);

    TIM_InitStruct.Prescaler = tim_prescaler;
    TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    TIM_InitStruct.Autoreload = 0xffff;
    TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    TIM_InitStruct.RepetitionCounter = 0;
    LL_TIM_Init(TIM1, &TIM_InitStruct);
    LL_TIM_DisableARRPreload(TIM1);
    LL_TIM_SetClockSource(TIM1, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM1, LL_TIM_TRGO_RESET);
    LL_TIM_SetTriggerOutput2(TIM1, LL_TIM_TRGO2_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM1);

    LL_TIM_ClearFlag_UPDATE(TIM1);
    LL_TIM_EnableIT_UPDATE(TIM1);

    LL_TIM_OC_InitTypeDef TIM_OC_InitStruct = {0};
    TIM_OC_InitStruct.OCMode = LL_TIM_OCMODE_FROZEN;
    TIM_OC_InitStruct.OCState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.OCNState = LL_TIM_OCSTATE_DISABLE;
    TIM_OC_InitStruct.CompareValue = 1000;
    TIM_OC_InitStruct.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    LL_TIM_OC_Init(TIM3, LL_TIM_CHANNEL_CH1, &TIM_OC_InitStruct);
    LL_TIM_OC_DisableFast(TIM3, LL_TIM_CHANNEL_CH1);

    LL_TIM_EnableIT_CC1(TIM1);

    LL_TIM_EnableCounter(TIM1);

    set_timer(5000);

    /* Configure the Cortex-M SysTick source to have 1ms time base */
    // LL_Init1msTick(SystemCoreClock);
    //    LL_mDelay(200);
}

/*
 * @brief This function handles TIM1 break, update, trigger and commutation interrupts.
 */
void TIM1_BRK_UP_TRG_COM_IRQHandler(void) {
    /* Check whether update interrupt is pending */
    if (LL_TIM_IsActiveFlag_UPDATE(TIM1) == 1) {
        /* Clear the update interrupt flag */
        LL_TIM_ClearFlag_UPDATE(TIM1);

        pulse_log_pin();
        pulse_log_pin();
    }
}

void TIM1_CC_IRQHandler() {
    if (LL_TIM_IsActiveFlag_CC1(TIM1) == 1) {
        LL_TIM_ClearFlag_CC1(TIM1);
        pulse_log_pin();
        set_timer(5000);
    }
}