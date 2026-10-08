#ifndef _MAIN_H
#define	_MAIN_H
		
////////////////////////////////////////////////////////////////////
/************************ EXTERNAL VARIABLES **********************/
////////////////////////////////////////////////////////////////////
extern uint8_t ucMain_RxIndex,ucMain_RxCmd,ucMain_ErrorCode;
extern uint32_t ulMain_LogInterval;

#define BOOT_REQUEST_MAGIC  0xB007 
/********************** FUNCTIONS DECLARATIONS ********************/
void vBSPInit(void);
void vMain_Delay_uSec(__IO uint32_t nCount);
void App_RequestBootloaderUpdate(void);

 #endif  //__MAIN_H__
