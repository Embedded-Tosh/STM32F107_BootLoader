/* BootCmd.c - command loop and per-session state */
#include <string.h>
#include "BootConfig.h"
#include "BootCmd.h"
#include "Protocol.h"
#include "Uart.h"
#include "FlashIf.h"
#include "FwValidate.h"
#include "AppJump.h"
#include "Debug.h"

/* Per-session state (reset by CMD_SYNC) */
static uint8_t  s_check_ok = 0;   /* CMD_CHECK passed in this session */
static uint8_t  s_erased   = 0;   /* app region erased and ready for writes */
#if DEBUG_BUILD
static uint32_t s_total_size = 0; /* declared image size (total bytes), from CMD_CHECK - progress display only */
#endif

void BootloaderCommandLoop(void)
{
    uint8_t  cmd;
    uint16_t payload_len;

    /* static, not on the stack: at MAX_PAYLOAD=1024 these two buffers alone are
     * 2 KB, which would overflow the bootloader's small default stack (see
     * Stack_Size in startup_stm32f10x_cl.s). RAM is otherwise idle here. */
    static uint8_t payload[MAX_PAYLOAD];
    static uint8_t padded[MAX_PAYLOAD];

    for (;;)
    {
        /* Block indefinitely waiting for frames - the host controls the pace. */
        UART_SelectActive(0); /* wait for a byte on UART4 or UART5 */
        if (!ReceiveFrame(&cmd, payload, &payload_len, 0))
        {
            UART_SendByte(NACK_BYTE);
            continue;
        }

        switch (cmd)
        {
            case CMD_SYNC:
                Flash_ResetWritePtr();     /* reset write pointer on every sync */
                s_check_ok = 0;            /* every session must pass CMD_CHECK again */
                s_erased   = 0;
#if DEBUG_BUILD
                s_total_size = 0;
#endif
                UART_SendByte(ACK_BYTE);
                break;

            case CMD_CHECK:
                if (payload_len == FW_HEADER_SIZE && Header_IsValid_ForCheck(payload))
                {
                    s_check_ok = 1;
#if DEBUG_BUILD
                    {
                        uint32_t declared_image_size;
                        memcpy(&declared_image_size, payload + 12, 4); /* FwMetadata_t.image_size */
                        s_total_size = declared_image_size;   /* image_size = total bytes to be written */
                    }
#endif
                    UART_SendByte(ACK_BYTE);
                    DEBUG_MSG("CMD_CHECK: OK");
                }
                else
                {
                    s_check_ok = 0;
                    UART_SendByte(NACK_BYTE);
                    DEBUG_MSG("CMD_CHECK: rejected");
                }
                break;

            case CMD_ERASE:
                if (s_check_ok && Flash_EraseAppRegion())
                {
                    s_erased = 1;
                    UART_SendByte(ACK_BYTE);
                    DEBUG_MSG("CMD_ERASE: OK");
                }
                else
                {
                    s_erased = 0;
                    UART_SendByte(NACK_BYTE);   /* no valid CMD_CHECK, or erase failed */
                    DEBUG_MSG("CMD_ERASE: rejected/failed");
                }
                break;

            case CMD_WRITE:
            {
                uint16_t padded_len = (payload_len + 3) & ~((uint16_t)3); /* round up to mult of 4 */

                if (!s_erased)
                {
                    UART_SendByte(NACK_BYTE);
                    break;
                }

                /* No mid-stream header check here: the check that matters runs
                 * once, right before the bootloader would ever jump to this app
                 * (App_IsValid(), from CMD_GO and on every normal boot), so a bad
                 * or incomplete image simply never runs. */

                memset(padded, 0xFF, sizeof(padded));
                memcpy(padded, payload, payload_len);

                if (Flash_WriteChunk(padded, padded_len))
                {
                    UART_SendByte(ACK_BYTE);
                    DEBUG_PROGRESS(Flash_GetWritePtr() - APP_ADDRESS, s_total_size);
                }
                else
                {
                    UART_SendByte(NACK_BYTE);
                    DEBUG_HEX("CMD_WRITE: failed at", Flash_GetWritePtr());
                }
                break;
            }

            case CMD_GO:
                DEBUG_MSG("CMD_GO: received");
                UART_SendByte(ACK_BYTE);
                /* small delay so the ACK byte actually leaves the TX shift register */
                {
                    volatile uint32_t d;
                    for (d = 0; d < 100000; d++) { }
                }
                JumpToApplication();
                /* if we get here, the jump was refused -> report and keep looping */
                UART_SendByte(NACK_BYTE);
                break;

            default:
                UART_SendByte(NACK_BYTE);
                break;
        }
    }
}




