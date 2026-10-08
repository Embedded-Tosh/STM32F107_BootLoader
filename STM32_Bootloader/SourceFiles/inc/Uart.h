#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "stm32f10x.h"

/* UART4 (PC10 TX / PC11 RX) and UART5 (PC12 TX / PD2 RX), fixed pins on the
 * F107, both listened to at once. The "active" port is the one the host last
 * sent a byte on; replies go back on it. */

void           UART_Init(void);
void           UART_DeInit_ForApp(void);

/* Waits until either port has a byte waiting (without reading it) and makes
 * that port active. Returns 1 if a port was selected, 0 on timeout
 * (timeout_ms == 0 means wait forever). */
uint8_t        UART_SelectActive(uint32_t timeout_ms);

USART_TypeDef *UART_GetActive(void);
void           UART_SendByte(uint8_t b);

/* Returns 1 on success, 0 on timeout (timeout_ms == 0 means poll forever) */
uint8_t        UART_ReadByte(uint8_t *out, uint32_t timeout_ms);

#endif
