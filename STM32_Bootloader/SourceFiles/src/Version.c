
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"                  
#include "Version.h"

/**********************************************************************
* Definitions
***********************************************************************/

#define APP_VERSION_RAM_ADDR   ((char *)0x2000FF00UL)
#define APP_VERSION_BUF_SIZE   64

#define VERSION_MAJOR    "1"
#define VERSION_MINOR    "00"
#define VERSION_TINY     "01"

#define BOOTLOADER_VERSION    "V" VERSION_MAJOR "." VERSION_MINOR "." VERSION_TINY  /* bootloader release */

/**********************************************************************
* Variables
***********************************************************************/
static const char *g_buildDate = __DATE__;   /* build date */
static const char *g_buildTime = __TIME__;   /* build time */
						
static char s_verBuf[APP_VERSION_BUF_SIZE]; /* version string buffer */



/**********************************************************************
* Local Function Prototypes
***********************************************************************/


/**********************************************************************
* Public Functions
***********************************************************************/

/**********************************************************************
* Function: 	Version_BuildString
* Purpose:		Cache the version string
* Parameters:	None
* Returns:   	Nothing
***********************************************************************/
void Version_Initialise(void)
{
    char *p = s_verBuf;
    const char *s;

    for (s = "DAQ Bootloader "; *s; ) *p++ = *s++;
    for (s = BOOTLOADER_VERSION; *s; ) *p++ = *s++;
    *p++ = ' ';

    *p++ = (g_buildDate[4] == ' ') ? '0' : g_buildDate[4];  /* day, tens digit */
    *p++ = g_buildDate[5];                                   /* day, ones digit */
    *p++ = '-';
    *p++ = g_buildDate[0]; *p++ = g_buildDate[1]; *p++ = g_buildDate[2]; /* month */
    *p++ = '-';
    *p++ = g_buildDate[7]; *p++ = g_buildDate[8];             /* year */
    *p++ = g_buildDate[9]; *p++ = g_buildDate[10];
		*p++ = ' ';
		*p++ = g_buildTime[0];
		*p++ = g_buildTime[1];
		*p++ = g_buildTime[2];
		*p++ = g_buildTime[3];
		*p++ = g_buildTime[4];
		*p++ = g_buildTime[5];
		*p++ = g_buildTime[6];
		*p++ = g_buildTime[7];

    *p = '\0';
}

/**********************************************************************
* Function: 	Version_GetString
* Purpose:		Get the version string
* Parameters:	None
* Returns:   	Version string
***********************************************************************/
const char *Version_GetString(void)
{
    return s_verBuf;
}

/**********************************************************************
* Function: 	Version_CopyForApplication
* Purpose:		Copy the version string to RAM (0x20020000) for the
*				application to display
* Parameters:	None
* Returns:   	Nothing
***********************************************************************/
void Version_CopyForApplication(void)
{
    strcpy(APP_VERSION_RAM_ADDR, s_verBuf);
}


