#ifndef CRC_H
#define CRC_H

#include "demac_config.h"

EXTERN const uint32_t crctab32[]; 

STATIC_DECLARE uint32_t ape_initcrc(void);
STATIC_DECLARE uint32_t ape_updatecrc(unsigned char *block, int count, uint32_t crc);
STATIC_DECLARE uint32_t ape_finishcrc(uint32_t crc);

#endif // CRC_H
