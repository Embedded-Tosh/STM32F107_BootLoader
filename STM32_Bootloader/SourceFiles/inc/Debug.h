/*******************************************************************************
 * Debug.h - plain-text debug output on the IDLE one of UART4/UART5
 *
 * Whichever port the host is using for the flash protocol is "active"; the
 * other one is idle, so it carries debug text (no extra pins needed). Watch it
 * with a USB-serial adapter at UART_BAUD (115200). Before the host picks a
 * port, debug goes out UART5.
 *
 * With DEBUG_BUILD = 0 every macro below compiles to nothing.
 ******************************************************************************/
#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "BootConfig.h"

#if DEBUG_BUILD

void Debug_Print(const char *s);
void Debug_PrintHex32(const char *label, uint32_t val);   /* "label: 0xXXXXXXXX" */
void Debug_PrintProgress(uint32_t done, uint32_t total);  /* "\r<done>/<total> bytes (<pct>%)" */

#define DEBUG_MSG(s)                Debug_Print(s "\r\n")          /* s must be a string literal */
#define DEBUG_STR(str)              do { Debug_Print(str); Debug_Print("\r\n"); } while (0) /* runtime string */
#define DEBUG_HEX(label, val)       Debug_PrintHex32(label, val)
#define DEBUG_PROGRESS(done, total) Debug_PrintProgress(done, total)

#else

#define DEBUG_MSG(s)                ((void)0)
#define DEBUG_STR(str)              ((void)0)
#define DEBUG_HEX(label, val)       ((void)0)
#define DEBUG_PROGRESS(done, total) ((void)0)

#endif /* DEBUG_BUILD */
uint8_t IsDebuggerAttached(void) ;
#endif /* DEBUG_H */
