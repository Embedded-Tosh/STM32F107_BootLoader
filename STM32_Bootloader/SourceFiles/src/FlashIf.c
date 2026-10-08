/* FlashIf.c - flash erase/program (Standard Peripheral Library) */
#include <string.h>
#include "stm32f10x.h"
#include "BootConfig.h"
#include "FlashIf.h"

static uint32_t s_write_ptr = APP_ADDRESS;

void Flash_ResetWritePtr(void)
{
    s_write_ptr = APP_ADDRESS;
}

uint32_t Flash_GetWritePtr(void)
{
    return s_write_ptr;
}

uint8_t Flash_EraseAppRegion(void)
{
    uint32_t addr;
    FLASH_Status st;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (addr = APP_ADDRESS; addr < (APP_ADDRESS + APP_MAX_SIZE); addr += FLASH_PAGE_SIZE_B)
    {
        st = FLASH_ErasePage(addr);
        if (st != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return 0;
        }
    }
    FLASH_Lock();
    s_write_ptr = APP_ADDRESS;
    return 1;
}

uint8_t Flash_WriteChunk(const uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint32_t word;
    FLASH_Status st;

    if (s_write_ptr + len > APP_ADDRESS + APP_MAX_SIZE)
        return 0; /* would overflow app region */

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

    for (i = 0; i < len; i += 4)
    {
        memcpy(&word, &data[i], 4);
        st = FLASH_ProgramWord(s_write_ptr, word);
        if (st != FLASH_COMPLETE)
        {
            FLASH_Lock();
            return 0;
        }
        /* read-back verify */
        if (*(__IO uint32_t *)s_write_ptr != word)
        {
            FLASH_Lock();
            return 0;
        }
        s_write_ptr += 4;
    }
    FLASH_Lock();
    return 1;
}
