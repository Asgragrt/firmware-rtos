#ifndef _FLASH_H_
#define _FLASH_H_

#include "hardware/flash.h"

#define FLASH_TARGET_OFFSET ( 1024 * 1024 )

extern const uint8_t* flash_target_contents;

// Sector size is 16 times the FLASH_PAGE_SIZE
#endif /* _FLASH_H_ */