#ifndef _CTPH_
#define _CTPH_

#include "elf_manager.h"

#include <stdbool.h>

#include <inttypes.h>

/* Return the hash of the ELF data in Base64 */
uint8_t *ctph_hash(elf_data data);

#endif