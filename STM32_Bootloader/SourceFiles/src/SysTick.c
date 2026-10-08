/* SysTick.c - 1 ms tick, used only for the entry / frame timeouts */
#include "stm32f10x.h"
#include "SysTick.h"

static volatile uint32_t s_systick_ms = 0;

/* Name is fixed by the startup file's vector table. If your project also has
 * stm32f10x_it.c defining SysTick_Handler, remove one of the two. */
void SysTick_Handler(void)
{
    s_systick_ms++;
}

void SysTick_Init_1ms(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

uint32_t millis(void)
{
    return s_systick_ms;
}
