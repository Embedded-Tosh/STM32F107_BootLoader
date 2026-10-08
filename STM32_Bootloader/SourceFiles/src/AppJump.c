/* AppJump.c - hand control to the application */
#include "stm32f10x.h"
#include "BootConfig.h"
#include "AppJump.h"
#include "FwValidate.h"
#include "Uart.h"

typedef void (*pFunction)(void);

void JumpToApplication(void)
{
    uint32_t appStack = *(__IO uint32_t *)APP_ADDRESS;

    /* Only jump if the image is plausible AND carries this machine's ID/CRC */
    if (App_IsValid())
    {
        pFunction AppEntry = (pFunction) *(__IO uint32_t *)(APP_ADDRESS + 4);

        UART_DeInit_ForApp();

        /* Disable SysTick and any bootloader interrupts before handing over */
        SysTick->CTRL = 0;
        __disable_irq();

        /* Reset peripheral clocks the app will re-init itself; not strictly
         * required but keeps things clean */
        RCC_DeInit();

        __set_MSP(appStack);
        __enable_irq();
        AppEntry();

        /* Never returns */
    }
    /* else: no valid app present -> return, caller stays in the bootloader */
}
