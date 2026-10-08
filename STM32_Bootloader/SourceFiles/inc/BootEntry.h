#ifndef BOOT_ENTRY_H
#define BOOT_ENTRY_H

#include <stdint.h>

/* Decides whether to stay in the bootloader (returns 1) or go to the app
 * (returns 0). Stays if: the app set the backup-register request flag, there
 * is no valid application, or a CMD_SYNC arrives within BOOT_ENTRY_TIMEOUT_MS. */
uint8_t ShouldEnterBootloader(void);

#endif
