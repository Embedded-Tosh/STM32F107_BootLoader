/* Debug.c - see Debug.h */
#include "stm32f10x.h"                  // Device header
#include "Debug.h"

uint8_t IsDebuggerAttached(void) 
{
    // Read the CoreDebug Halting Control and Status Register (DHCSR)
    // Check if the C_DEBUGEN bit (Bit 0) is set to 1
    if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0) {
        return 1; // Debugger is active
    }
    
    return 0; // Running standalone on external/battery power
}


#if DEBUG_BUILD

#include "Uart.h"

static void Debug_PutChar(char c)
{
    USART_TypeDef *dbg = (UART_GetActive() == UART4) ? UART5 : UART4;
    while (USART_GetFlagStatus(dbg, USART_FLAG_TXE) == RESET) { }
    USART_SendData(dbg, (uint8_t)c);
}

void Debug_Print(const char *s)
{
    while (*s) Debug_PutChar(*s++);
}

/* No sprintf/printf, to keep this out of the bootloader's flash budget. */
void Debug_PrintHex32(const char *label, uint32_t val)
{
    static const char hex[] = "0123456789ABCDEF";
    int8_t i;

    Debug_Print(label);
    Debug_Print(": 0x");
    for (i = 28; i >= 0; i -= 4)
    {
        Debug_PutChar(hex[(val >> i) & 0xF]);
    }
    Debug_Print("\r\n");
}

static void Debug_PrintDec32(uint32_t v)
{
    char tmp[10];
    uint8_t n = 0;

    if (v == 0) { Debug_PutChar('0'); return; }
    while (v > 0) { tmp[n++] = (char)('0' + (v % 10u)); v /= 10u; }
    while (n > 0) Debug_PutChar(tmp[--n]);
}

/* Same look as flash_tool.py. The line is rewritten in place with CR; a
 * newline is added at 100%. */
void Debug_PrintProgress(uint32_t done, uint32_t total)
{
    uint32_t pct;

    if (total == 0) return;
    if (done > total) done = total;     /* last chunk is padded to a multiple of 4 */
    pct = (done * 100u) / total;

    Debug_PutChar('\r');
    Debug_PrintDec32(done);
    Debug_PutChar('/');
    Debug_PrintDec32(total);
    Debug_Print(" bytes (");
    Debug_PrintDec32(pct);
    Debug_Print("%)");
    if (done == total) Debug_Print("\r\n");
}

#else
typedef int Debug_c_is_empty_when_DEBUG_BUILD_is_0;   /* avoid an empty translation unit */
#endif /* DEBUG_BUILD */
