/* Uart.c - UART4 + UART5 driver (Standard Peripheral Library) */
#include "BootConfig.h"
#include "Uart.h"
#include "SysTick.h"

/* The port the current frame arrived on; replies go back on it. */
static USART_TypeDef *s_uart = UART4;

static void UART_InitOne(USART_TypeDef *u)
{
    USART_InitTypeDef usart;

    usart.USART_BaudRate            = UART_BAUD;
    usart.USART_WordLength          = USART_WordLength_8b;
    usart.USART_StopBits            = USART_StopBits_1;
    usart.USART_Parity              = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(u, &usart);
    USART_Cmd(u, ENABLE);
}

void UART_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD |
                           RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 | RCC_APB1Periph_UART5, ENABLE);

    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    /* UART4: PC10 = TX, AF push-pull */
    gpio.GPIO_Pin  = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &gpio);

    /* UART5: PC12 = TX, AF push-pull */
    gpio.GPIO_Pin  = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &gpio);

    /* UART4: PC11 = RX. Pull-up keeps an unconnected line idle-high so
     * noise on it cannot be mistaken for incoming data. */
    gpio.GPIO_Pin  = GPIO_Pin_11;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &gpio);

    /* UART5: PD2 = RX, same reason */
    gpio.GPIO_Pin  = GPIO_Pin_2;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &gpio);

    UART_InitOne(UART4);
    UART_InitOne(UART5);
}

void UART_DeInit_ForApp(void)
{
    USART_Cmd(UART4, DISABLE);
    USART_DeInit(UART4);
    USART_Cmd(UART5, DISABLE);
    USART_DeInit(UART5);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4 | RCC_APB1Periph_UART5, DISABLE);
}

uint8_t UART_SelectActive(uint32_t timeout_ms)
{
    uint32_t start = millis();
    for (;;)
    {
        if (USART_GetFlagStatus(UART4, USART_FLAG_RXNE) != RESET) { s_uart = UART4; return 1; }
        if (USART_GetFlagStatus(UART5, USART_FLAG_RXNE) != RESET) { s_uart = UART5; return 1; }
        if (timeout_ms != 0 && (millis() - start) > timeout_ms)
            return 0;
    }
}

USART_TypeDef *UART_GetActive(void)
{
    return s_uart;
}

void UART_SendByte(uint8_t b)
{
    while (USART_GetFlagStatus(s_uart, USART_FLAG_TXE) == RESET) { }
    USART_SendData(s_uart, b);
}

uint8_t UART_ReadByte(uint8_t *out, uint32_t timeout_ms)
{
    uint32_t start = millis();
    while (USART_GetFlagStatus(s_uart, USART_FLAG_RXNE) == RESET)
    {
        if (timeout_ms != 0 && (millis() - start) > timeout_ms)
            return 0;
    }
    *out = (uint8_t)USART_ReceiveData(s_uart);
    return 1;
}
