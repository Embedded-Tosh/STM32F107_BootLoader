/* FwValidate.c - firmware identity + integrity checks */
#include <string.h>
#include "stm32f10x.h"
#include "BootConfig.h"
#include "FwValidate.h"
#include "Crc.h"
#include "Debug.h"

uint8_t Header_IsValid_ForCheck(const uint8_t *h)
{
    FwMetadata_t md;
    memcpy(&md, h, sizeof(md));
    return (md.password == FW_PASSWORD && md.machine_id == MACHINE_ID) ? 1 : 0;
}

/* Reads everything straight from flash - never trusts the host's pre-write
 * CMD_CHECK payload for image_size or crc32. This is what catches a corrupted
 * or incomplete transfer, not just a wrong-machine image. */
static uint8_t Find_And_ValidateHeader(void)
{
    const FwMetadata_t *md = (const FwMetadata_t *)FW_METADATA_ADDRESS;
    const uint32_t footer_off = FW_METADATA_ADDRESS - APP_ADDRESS;   /* bytes before the footer */
    uint32_t crc;

    if (md->password != FW_PASSWORD)
    {
        DEBUG_HEX("validate: bad password", md->password);
        return 0;
    }
    if (md->machine_id != MACHINE_ID)
    {
        DEBUG_HEX("validate: machine_id mismatch", md->machine_id);
        return 0;
    }

    /* image_size = total bytes written to flash from APP_ADDRESS. It must
     * at least reach the end of the footer, and fit in the application area.
     * (The linker may store the initial values of initialised variables
     * AFTER the footer, so the footer is not necessarily the last thing.) */
    if (md->image_size < (footer_off + FW_HEADER_SIZE) || md->image_size > APP_MAX_SIZE)
    {
        DEBUG_HEX("validate: bad image_size", md->image_size);
        return 0;
    }

    /* CRC over the whole image EXCEPT the 20 footer bytes (the footer holds
     * the CRC itself): the part before it, then the part after it. */
    crc = Crc32_Init();
    crc = Crc32_Update(crc, (const uint8_t *)APP_ADDRESS, footer_off);
    crc = Crc32_Update(crc, (const uint8_t *)(FW_METADATA_ADDRESS + FW_HEADER_SIZE),
                       md->image_size - footer_off - FW_HEADER_SIZE);
    crc = Crc32_Final(crc);
    if (crc != md->crc32)
    {
        DEBUG_HEX("validate: crc32 computed", crc);
        DEBUG_HEX("validate: crc32 expected", md->crc32);
        return 0;
    }

    DEBUG_MSG("validate: OK");
    return 1;
}

uint8_t App_IsValid(void)
{
    uint32_t sp    = *(__IO uint32_t *)APP_ADDRESS;
    uint32_t reset = *(__IO uint32_t *)(APP_ADDRESS + 4);

    if (sp <= 0x20000000UL || sp > 0x20010000UL)                 return 0; /* F107: 64 KB SRAM */
    if (!(reset & 1) || reset < APP_ADDRESS ||
        reset >= (APP_ADDRESS + APP_MAX_SIZE))                   return 0;
    return Find_And_ValidateHeader();
}


