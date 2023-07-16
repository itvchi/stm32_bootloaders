#include "crc32.h"
#include "stm32f4xx_hal.h"
#include "crc.h"
#define CRC32_POLYNOMIAL                        ((uint32_t)0xEDB88320)

static uint32_t revbit(uint32_t uData)
{
	uint32_t uRevData = 0,uIndex = 0;
    uRevData |= ((uData >> uIndex) & 0x01);
    for(uIndex = 1;uIndex < 32;uIndex++)
    {
        uRevData <<= 1;
        uRevData |= ((uData >> uIndex) & 0x01);
    }
    return uRevData;
}

/**
 * @brief  Calc CRC32 for data in bytes
 * @param  pData Buffer pointer
 * @param  uLen  Buffer Length
 * @retval CRC32 Checksum
 */
uint32_t CRC32_ForBytes(uint8_t *pData, uint32_t uLen)
{
    uint32_t uIndex= 0,uData = 0,i;
    uIndex = uLen >> 2;

    __HAL_RCC_CRC_CLK_ENABLE();

    /* Reset CRC generator */
    __HAL_CRC_DR_RESET(&hcrc);

    while(uIndex--)
    {
#ifdef USED_BIG_ENDIAN
        uData = __REV((u32*)pData);
#else
        ((uint8_t *)&uData)[0] = pData[0];
        ((uint8_t *)&uData)[1] = pData[1];
        ((uint8_t *)&uData)[2] = pData[2];
        ((uint8_t *)&uData)[3] = pData[3];
#endif
        pData += 4;
        uData = revbit(uData);
        CRC->DR = uData;
    }
    uData = revbit(CRC->DR);
    uIndex = uLen & 0x03;
    while(uIndex--)
    {
        uData ^= (uint32_t)*pData++;
        for(i = 0;i < 8;i++)
          if (uData & 0x1)
            uData = (uData >> 1) ^ CRC32_POLYNOMIAL;
          else
            uData >>= 1;
    }

    __HAL_RCC_CRC_CLK_DISABLE();

    return uData^0xFFFFFFFF;
}


