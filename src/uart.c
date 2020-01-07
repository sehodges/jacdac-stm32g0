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

#define USART_IDX 1

#if USART_IDX == 1
#define USARTx USART1
#define IRQn USART1_IRQn
#define IRQHandler USART1_IRQHandler
#define LL_DMAMUX_REQ_USARTx_RX LL_DMAMUX_REQ_USART1_RX
#define LL_DMAMUX_REQ_USARTx_TX LL_DMAMUX_REQ_USART1_TX
#elif USART_IDX == 2
#define USARTx USART2
#define IRQn USART2_IRQn
#define IRQHandler USART2_IRQHandler
#define LL_DMAMUX_REQ_USARTx_RX LL_DMAMUX_REQ_USART2_RX
#define LL_DMAMUX_REQ_USARTx_TX LL_DMAMUX_REQ_USART2_TX
#else
#error "bad usart"
#endif

#if USART_IDX == 1
#define PIN_PORT GPIOB
#define PIN_PIN LL_GPIO_PIN_6
#define PIN_AF LL_GPIO_AF_0
#elif USART_IDX == 2
#define PIN_PORT GPIOA
#define PIN_PIN LL_GPIO_PIN_2
#define PIN_AF LL_GPIO_AF_1
#else
#error "bad usart"
#endif

static void uartOwnsPin(int doesIt) {
    if (doesIt) {
        LL_GPIO_SetPinMode(PIN_PORT, PIN_PIN, LL_GPIO_MODE_ALTERNATE);
        LL_GPIO_SetAFPin_0_7(PIN_PORT, PIN_PIN, PIN_AF);
    } else {
        LL_GPIO_SetPinMode(PIN_PORT, PIN_PIN, LL_GPIO_MODE_INPUT);
        exti_enable(PIN_PIN);
    }
}

void uart_disable() {
    LL_DMA_ClearFlag_GI2(DMA1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

    LL_DMA_ClearFlag_GI3(DMA1);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

    uartOwnsPin(0);
    LL_USART_Disable(USARTx);
}

void DMA1_Channel2_3_IRQHandler(void) {
    uint32_t isr = DMA1->ISR;

    // DMESG("DMA irq %x", isr);

    if (isr & (DMA_ISR_TCIF2 | DMA_ISR_TEIF2)) {
        uart_disable();
        if (isr & DMA_ISR_TCIF2) {
            // overrun?
            DMESG("USARTx RX OK, but how?!");
        } else {
            DMESG("USARTx RX Error");
        }
    }

    if (isr & (DMA_ISR_TCIF3 | DMA_ISR_TEIF3)) {
        LL_DMA_ClearFlag_GI3(DMA1);
        LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

        int errCode = 0;
        if (isr & DMA_ISR_TCIF3) {
            while (!LL_USART_IsActiveFlag_TC(USARTx))
                ;
            LL_USART_RequestBreakSending(USARTx);
            // DMESG("USARTx %x", USARTx->ISR);
            while (LL_USART_IsActiveFlag_SBK(USARTx))
                ;
        } else {
            DMESG("USARTx TX Error");
            errCode = -1;
        }

        uart_disable();

        jd_tx_completed(errCode);
    }
}

static void DMA_Init(void) {
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    // NVIC_SetPriority(DMA1_Channel1_IRQn, 1);
    // NVIC_EnableIRQ(DMA1_Channel1_IRQn);

    NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1);
    NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
}

static void USART_UART_Init(void) {
    LL_USART_InitTypeDef USART_InitStruct = {0};

#if USART_IDX == 2
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
#elif USART_IDX == 1
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
    LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
#else
#error "bad usart"
#endif

    LL_GPIO_SetPinPull(PIN_PORT, PIN_PIN, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinSpeed(PIN_PORT, PIN_PIN, LL_GPIO_SPEED_FREQ_HIGH);
    LL_GPIO_SetPinOutputType(PIN_PORT, PIN_PIN, LL_GPIO_OUTPUT_PUSHPULL);
    uartOwnsPin(0);
    exti_set_callback(PIN_PORT, PIN_PIN, jd_line_falling);

    /* USART_RX Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMAMUX_REQ_USARTx_RX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);

    /* USART_TX Init */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_USARTx_TX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_HIGH);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);

    /* USARTx interrupt Init */
    NVIC_SetPriority(IRQn, 1);
    NVIC_EnableIRQ(IRQn);

    /* Enable DMA transfer complete/error interrupts  */
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_2);

    USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
    USART_InitStruct.BaudRate = 1000000;
    USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
    USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
    USART_InitStruct.Parity = LL_USART_PARITY_NONE;
    USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX;
    USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
    LL_USART_Init(USARTx, &USART_InitStruct);

    LL_USART_SetTXFIFOThreshold(USARTx, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_SetRXFIFOThreshold(USARTx, LL_USART_FIFOTHRESHOLD_1_8);
    LL_USART_DisableFIFO(USARTx);
    LL_USART_ConfigHalfDuplexMode(USARTx);
    LL_USART_EnableIT_ERROR(USARTx);
    // LL_USART_EnableDMADeactOnRxErr(USARTx);

    // while (!(LL_USART_IsActiveFlag_REACK(USARTx)))
    //    ;
}

void uart_init() {
    DMA_Init();
    USART_UART_Init();
}

static void check_idle() {
    if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_3))
        panic();
    if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_2))
        panic();
    if (LL_USART_IsEnabled(USARTx))
        panic();
}

void uart_start_tx(const void *data, uint32_t numbytes) {
    check_idle();

    // DMESG("start TX %x %d", data, numbytes);

    exti_disable(PIN_PIN);

    LL_GPIO_SetPinMode(PIN_PORT, PIN_PIN, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_ResetOutputPin(PIN_PORT, PIN_PIN);
    wait_us(9);
    LL_GPIO_SetOutputPin(PIN_PORT, PIN_PIN);

    // from here...
    uartOwnsPin(1);
    LL_USART_DisableDirectionRx(USARTx);
    LL_USART_EnableDirectionTx(USARTx);
    USARTx->ICR = USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE; // clear error flags before we start
    LL_USART_Enable(USARTx);
    while (!(LL_USART_IsActiveFlag_TEACK(USARTx)))
        ;

    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, (uint32_t)data, (uint32_t) & (USARTx->TDR),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, numbytes);
    LL_USART_EnableDMAReq_TX(USARTx);
    // to here, it's about 1.3us

    // the USART takes a few us to start transmiting
    // this value gives 60us from the end of low pulse to start bit
    wait_us(57);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}

void uart_start_rx(void *data, uint32_t maxbytes) {
    check_idle();

    exti_disable(PIN_PIN);

    uartOwnsPin(1);
    LL_USART_DisableDirectionTx(USARTx);
    LL_USART_EnableDirectionRx(USARTx);
    USARTx->ICR = USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE; // clear error flags before we start
    LL_USART_Enable(USARTx);
    while (!(LL_USART_IsActiveFlag_REACK(USARTx)))
        ;

    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2, (uint32_t) & (USARTx->RDR), (uint32_t)data,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, maxbytes);
    LL_USART_EnableDMAReq_RX(USARTx);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
}

// this is only enabled for error events
void IRQHandler(void) {
    pulse_log_pin();
    uint32_t dataLeft = LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_2);
    uart_disable();
    jd_rx_completed(dataLeft);
}
