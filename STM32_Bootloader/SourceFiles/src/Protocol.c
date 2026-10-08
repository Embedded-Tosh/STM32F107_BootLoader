/* Protocol.c - frame reception */
#include "BootConfig.h"
#include "Protocol.h"
#include "Uart.h"
#include "Crc.h"

uint8_t ReceiveFrame(uint8_t *cmd, uint8_t *payload, uint16_t *payload_len,
                     uint32_t byte_timeout_ms)
{
    uint8_t  b;
    uint8_t  len_lo, len_hi;
    uint16_t len;
    uint16_t crc_calc, crc_rx;
    uint8_t  crc_lo, crc_hi;

    crc_calc = 0xFFFF;

    /* 1) wait for start byte (not covered by CRC) */
    if (!UART_ReadByte(&b, byte_timeout_ms)) return 0;
    if (b != FRAME_START_BYTE) return 0;

    /* 2) CMD */
    if (!UART_ReadByte(cmd, byte_timeout_ms)) return 0;
    crc_calc = Crc16_Update(crc_calc, *cmd);

    /* 3) LEN (little endian) */
    if (!UART_ReadByte(&len_lo, byte_timeout_ms)) return 0;
    crc_calc = Crc16_Update(crc_calc, len_lo);
    if (!UART_ReadByte(&len_hi, byte_timeout_ms)) return 0;
    crc_calc = Crc16_Update(crc_calc, len_hi);
    len = (uint16_t)len_lo | ((uint16_t)len_hi << 8);

    if (len > MAX_PAYLOAD) return 0;

    /* 4) payload - fed straight into the running CRC as each byte arrives,
     *    so no second MAX_PAYLOAD-sized buffer is needed */
    {
        uint16_t i;
        for (i = 0; i < len; i++)
        {
            if (!UART_ReadByte(&payload[i], byte_timeout_ms)) return 0;
            crc_calc = Crc16_Update(crc_calc, payload[i]);
        }
    }
    *payload_len = len;

    /* 5) CRC16 sent by the host */
    if (!UART_ReadByte(&crc_lo, byte_timeout_ms)) return 0;
    if (!UART_ReadByte(&crc_hi, byte_timeout_ms)) return 0;
    crc_rx = (uint16_t)crc_lo | ((uint16_t)crc_hi << 8);

    return (crc_calc == crc_rx) ? 1 : 0;
}
