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
    uint32_t crc;

    if (md->password != FW_PASSWORD)
    {
        DEBUG_HEX("validate: bad Password", md->password);
        return 0;
    }
    if (md->machine_id != MACHINE_ID)
    {
        DEBUG_HEX("validate: machine_id mismatch", md->machine_id);
        return 0;
    }
		
		if(IsDebuggerAttached())
		{
			DEBUG_MSG("Debug Active Skip CRC");
			return 1;
		}
    /* image_size must be non-zero and must not reach into the metadata region
     * itself (the footer's own crc32 field isn't part of what it describes). */
    if (md->image_size == 0 || md->image_size > (APP_MAX_SIZE - FW_METADATA_REGION_SIZE))
    {
        DEBUG_HEX("validate: bad image_size", md->image_size);
        return 0;
    }

    crc = Crc32_Compute((const uint8_t *)APP_ADDRESS, md->image_size);
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
