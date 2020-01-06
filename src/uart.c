#include "jdsimple.h"

#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_system.h"
#include "stm32g0xx_ll_exti.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_utils.h"
#include "stm32g0xx_ll_pwr.h"
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_ll_gpio.h"


void DMA1_Channel2_3_IRQHandler(void) {
    if (LL_DMA_IsActiveFlag_TC2(DMA1)) {
        LL_DMA_ClearFlag_GI2(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    } else if (LL_DMA_IsActiveFlag_TE2(DMA1)) {
        DMESG("USART RX Error");
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    }

    if (LL_DMA_IsActiveFlag_TC3(DMA1)) {
        LL_DMA_ClearFlag_GI3(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    } else if (LL_DMA_IsActiveFlag_TE3(DMA1)) {
        DMESG("USART TX Error");
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    }
}

void USART2_IRQHandler(void) {
    DMESG("USART handler");
}

static void DMA_Init(void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    // NVIC_SetPriority(DMA1_Channel1_IRQn, 0);
    // NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0);
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

static void USART2_UART_Init(void) {
    LL_USART_InitTypeDef USART_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

    /**USART2 GPIO Configuration
    PA2   ------> USART2_TX
    PA3   ------> USART2_RX
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2_RX Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMAMUX_REQ_USART2_RX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);

    /* USART2_TX Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_USART2_TX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

    /* USART2 interrupt Init */
    NVIC_SetPriority(USART2_IRQn, 0);
    NVIC_EnableIRQ(USART2_IRQn);

    /* Enable DMA transfer complete/error interrupts  */
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate = 115200;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USART2, &USART_InitStruct);
    LL_USART_SetTXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(USART2, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_DisableFIFO(USART2);
    LL_USART_ConfigAsyncMode(USART2);

    LL_USART_Enable(USART2);

    /* Polling USART2 initialisation */
    while ((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2)))) {
    }

    /* Polling USART initialisation */
    while ((!(LL_USART_IsActiveFlag_TEACK(USART2))) || (!(LL_USART_IsActiveFlag_REACK(USART2)))) {
    }
}

void uart_init() {
    DMA_Init();
    USART2_UART_Init();
}

void uart_start_tx(const void *data, uint32_t numbytes) {
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, (uint32_t)data,
                           LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_TRANSMIT),
                           LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, numbytes);
    LL_USART_EnableDMAReq_TX(USART2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}

void uart_start_rx(void *data, uint32_t maxbytes) {
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
                           LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE),
                           (uint32_t)data, LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, maxbytes);
    LL_USART_EnableDMAReq_RX(USART2);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}
