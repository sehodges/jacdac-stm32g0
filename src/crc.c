#include "stm32g0xx_ll_crc.h"
#include "stm32g0xx_ll_bus.h"

static uint8_t inited;

// software implementation also possible, but needs 0.5k of memory, or is slow

uint16_t crc16(const void *data, uint32_t size) {
    if (!inited) {
        LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_CRC);
        // reset value
        //LL_CRC_SetInputDataReverseMode(CRC, LL_CRC_INDATA_REVERSE_NONE);
        //LL_CRC_SetOutputDataReverseMode(CRC, LL_CRC_OUTDATA_REVERSE_NONE);
        LL_CRC_SetPolynomialCoef(CRC, 0x1021);
        LL_CRC_SetPolynomialSize(CRC, LL_CRC_POLYLENGTH_16B);
    }
    LL_CRC_SetInitialData(CRC, LL_CRC_DEFAULT_CRC_INITVALUE);
    const uint8_t *ptr = (const uint8_t *)data;
    while (size--)
        LL_CRC_FeedData8(CRC, *ptr++);
    return LL_CRC_ReadData16(CRC);
}
