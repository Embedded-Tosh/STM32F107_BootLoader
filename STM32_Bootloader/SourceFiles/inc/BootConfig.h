/*******************************************************************************
 * BootConfig.h - build-time configuration shared by every bootloader module
 *
 * Memory map assumed:
 *   Bootloader : 0x08000000 - 0x08002FFF   (12 KB)
 *   Application: 0x08003000 - 0x0803FFFF   (244 KB)
 *   FW_METADATA: last 2048 bytes of the application region (see app_scatter.sct)
 *
 * Build the bootloader as its OWN Keil project with its own scatter file:
 *   IROM1 base = 0x08000000, size = 0x00003000
 ******************************************************************************/
#ifndef BOOT_CONFIG_H
#define BOOT_CONFIG_H

#include <stdint.h>

/* Master switch for all debug output (idle-UART text, version print, write
 * progress). Set to 0 for production - everything compiles away. Can also be
 * overridden from the Keil "Define" box (e.g. DEBUG_BUILD=0). */
#ifndef DEBUG_BUILD
#define DEBUG_BUILD         1
#endif

/* ---- Flash layout ---- */
#define APP_ADDRESS         ((uint32_t)0x08003000)
#define APP_MAX_SIZE        ((uint32_t)0x0003D000)   /* 244 KB (249856 bytes) */
#define FLASH_PAGE_SIZE_B   ((uint32_t)0x800)        /* 2 KB pages on connectivity line */

/* ---- Timing / link ---- */
#define BOOT_ENTRY_TIMEOUT_MS   2000    /* time to wait for CMD_SYNC before jumping to app */
#define UART_BAUD               115200
#define MAX_PAYLOAD             1024

/* Backup register magic value: application can request a reboot into
 * bootloader by writing this into BKP_DR1 before doing NVIC_SystemReset(). */
#define BOOT_REQUEST_MAGIC  0xB007

/* ---- Firmware identity: must match fw_metadata.c in the application ---- */
#define FW_PASSWORD            0x31454C45UL   /* bytes in flash: 45 4C 45 31 = "ELE1" */
#ifndef MACHINE_ID
#define MACHINE_ID          0x100C         /* 100 kN load frame */
#endif

/* Firmware ID footer, little-endian, in flash order: magic(4), machine_id(4),
 * version(4), image_size(4), crc32(4). image_size is the length of the actual
 * application image (APP_ADDRESS up to, not including, this footer); crc32 is
 * standard CRC-32 (zlib/PNG/Ethernet) over exactly those image_size bytes. */
#define FW_HEADER_SIZE      20

typedef struct
{
    uint32_t password;
    uint32_t machine_id;
    uint32_t version;
    uint32_t image_size;
    uint32_t crc32;
} FwMetadata_t;

/* FW_METADATA is a FIXED scatter region occupying the last 2048 bytes of
 * flash (only the first FW_HEADER_SIZE bytes are used). */
#define FW_METADATA_REGION_SIZE   2048u   /* must match the scatter file */
#define FW_METADATA_ADDRESS       (APP_ADDRESS + APP_MAX_SIZE - FW_METADATA_REGION_SIZE)

#endif /* BOOT_CONFIG_H */
