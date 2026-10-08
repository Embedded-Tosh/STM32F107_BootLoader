/* BootEntry.c - stay in the bootloader, or go straight to the app? */
#include "stm32f10x.h"
#include "BootConfig.h"
#include "BootEntry.h"
#include "Protocol.h"
#include "Uart.h"
#include "FwValidate.h"
#include "Debug.h"

uint8_t ShouldEnterBootloader(void)
{
    uint8_t  cmd;
    uint16_t payload_len;
    static uint8_t payload[MAX_PAYLOAD]; /* static: see note in BootCmd.c */

    /* --- Option A: backup register flag set by the app before a software
     *     reset (app wants an update). Requires PWR/BKP clocks. --- */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    if (BKP_ReadBackupRegister(BKP_DR1) == BOOT_REQUEST_MAGIC)
    {
        BKP_WriteBackupRegister(BKP_DR1, 0x0000); /* clear the flag */
        DEBUG_MSG("entry: app requested update");
        return 1;
    }

    /* --- Option B: if no valid application is present, must stay --- */
    if (!App_IsValid())
    {
        DEBUG_MSG("entry: no valid app, staying in bootloader");
        return 1;
    }

    /* --- Option C: short window listening for a CMD_SYNC frame --- */
    if (UART_SelectActive(BOOT_ENTRY_TIMEOUT_MS) &&
        ReceiveFrame(&cmd, payload, &payload_len, BOOT_ENTRY_TIMEOUT_MS) && cmd == CMD_SYNC)
    {
        UART_SendByte(ACK_BYTE);
        DEBUG_MSG("entry: SYNC received");
        return 1;
    }

    DEBUG_MSG("entry: timeout, jumping to app");
    return 0;
}
