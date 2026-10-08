#ifndef BOOT_CMD_H
#define BOOT_CMD_H

/* Command loop: receives frames on UART4/UART5 and executes SYNC / CHECK /
 * ERASE / WRITE / GO (see Protocol.h). Never returns. */
void BootloaderCommandLoop(void);

#endif
