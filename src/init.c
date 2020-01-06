#include "jdsimple.h"

void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

void NMI_Handler(void) {}

void HardFault_Handler(void) {
    panic();
}

void SVC_Handler(void) {}

void PendSV_Handler(void) {}

void SysTick_Handler(void) {
    HAL_IncTick();
}

static void enable_nrst_pin() {
    DMESG("check NRST", FLASH->OPTR & FLASH_OPTR_NRST_MODE);

    if (FLASH->OPTR & FLASH_OPTR_NRST_MODE_0)
        return;

    uint32_t nrstmode;

    /* Enable Flash access anyway */
    __HAL_RCC_FLASH_CLK_ENABLE();

    /* Unlock flash */
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    while ((FLASH->CR & FLASH_CR_LOCK) != 0x00)
        ;

    /* unlock option byte registers */
    FLASH->OPTKEYR = 0x08192A3B;
    FLASH->OPTKEYR = 0x4C5D6E7F;
    while ((FLASH->CR & FLASH_CR_OPTLOCK) == FLASH_CR_OPTLOCK)
        ;

    /* get current user option bytes */
    nrstmode = (FLASH->OPTR & ~FLASH_OPTR_NRST_MODE);
    nrstmode |= FLASH_OPTR_NRST_MODE_0;

    /* Program option bytes */
    FLASH->OPTR = nrstmode;

    /* Write operation */
    FLASH->CR |= FLASH_CR_OPTSTRT;
    while ((FLASH->SR & FLASH_SR_BSY1) != 0)
        ;

    /* Force OB Load */
    FLASH->CR |= FLASH_CR_OBL_LAUNCH;

    while (1)
        ;
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
    // Initializes the CPU, AHB and APB busses clocks
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN = 8;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        panic();
    }
    // Initializes the CPU, AHB and APB busses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        panic();
    }

    enable_nrst_pin();
}
