SERIES = F0
DEFINES += -DSTM32F031x6
CFLAGS += -mcpu=cortex-m0

HALPREF = $(DRV)/STM32F0xx_HAL_Driver/Src/
HALSRC =  \
$(HALPREF)/stm32f0xx_hal.c \
$(HALPREF)/stm32f0xx_hal_adc.c \
$(HALPREF)/stm32f0xx_hal_adc_ex.c \
$(HALPREF)/stm32f0xx_hal_can.c \
$(HALPREF)/stm32f0xx_hal_cec.c \
$(HALPREF)/stm32f0xx_hal_comp.c \
$(HALPREF)/stm32f0xx_hal_cortex.c \
$(HALPREF)/stm32f0xx_hal_crc.c \
$(HALPREF)/stm32f0xx_hal_crc_ex.c \
$(HALPREF)/stm32f0xx_hal_dac.c \
$(HALPREF)/stm32f0xx_hal_dac_ex.c \
$(HALPREF)/stm32f0xx_hal_dma.c \
$(HALPREF)/stm32f0xx_hal_exti.c \
$(HALPREF)/stm32f0xx_hal_flash.c \
$(HALPREF)/stm32f0xx_hal_flash_ex.c \
$(HALPREF)/stm32f0xx_hal_gpio.c \
$(HALPREF)/stm32f0xx_hal_i2c.c \
$(HALPREF)/stm32f0xx_hal_i2c_ex.c \
$(HALPREF)/stm32f0xx_hal_i2s.c \
$(HALPREF)/stm32f0xx_hal_irda.c \
$(HALPREF)/stm32f0xx_hal_iwdg.c \
$(HALPREF)/stm32f0xx_hal_pcd.c \
$(HALPREF)/stm32f0xx_hal_pcd_ex.c \
$(HALPREF)/stm32f0xx_hal_pwr.c \
$(HALPREF)/stm32f0xx_hal_pwr_ex.c \
$(HALPREF)/stm32f0xx_hal_rcc.c \
$(HALPREF)/stm32f0xx_hal_rcc_ex.c \
$(HALPREF)/stm32f0xx_hal_rtc.c \
$(HALPREF)/stm32f0xx_hal_rtc_ex.c \
$(HALPREF)/stm32f0xx_hal_smartcard.c \
$(HALPREF)/stm32f0xx_hal_smartcard_ex.c \
$(HALPREF)/stm32f0xx_hal_smbus.c \
$(HALPREF)/stm32f0xx_hal_spi.c \
$(HALPREF)/stm32f0xx_hal_spi_ex.c \
$(HALPREF)/stm32f0xx_hal_tim.c \
$(HALPREF)/stm32f0xx_hal_tim_ex.c \
$(HALPREF)/stm32f0xx_hal_tsc.c \
$(HALPREF)/stm32f0xx_hal_uart.c \
$(HALPREF)/stm32f0xx_hal_uart_ex.c \
$(HALPREF)/stm32f0xx_hal_usart.c \
$(HALPREF)/stm32f0xx_hal_usart_ex.c \
$(HALPREF)/stm32f0xx_hal_wwdg.c \
$(HALPREF)/stm32f0xx_ll_adc.c \
$(HALPREF)/stm32f0xx_ll_comp.c \
$(HALPREF)/stm32f0xx_ll_crc.c \
$(HALPREF)/stm32f0xx_ll_crs.c \
$(HALPREF)/stm32f0xx_ll_dac.c \
$(HALPREF)/stm32f0xx_ll_dma.c \
$(HALPREF)/stm32f0xx_ll_exti.c \
$(HALPREF)/stm32f0xx_ll_gpio.c \
$(HALPREF)/stm32f0xx_ll_i2c.c \
$(HALPREF)/stm32f0xx_ll_pwr.c \
$(HALPREF)/stm32f0xx_ll_rcc.c \
$(HALPREF)/stm32f0xx_ll_rtc.c \
$(HALPREF)/stm32f0xx_ll_spi.c \
$(HALPREF)/stm32f0xx_ll_tim.c \
$(HALPREF)/stm32f0xx_ll_usart.c \
$(HALPREF)/stm32f0xx_ll_usb.c \
$(HALPREF)/stm32f0xx_ll_utils.c \
