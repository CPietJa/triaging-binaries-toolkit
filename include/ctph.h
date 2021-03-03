#ifndef _CTPH_
#define _CTPH_

#include "elf_manager.h"

#include <stdbool.h>

/* Return the hash of the ELF data in Base64 */
char *ctph_hash(elf_data data);

/* Return a score of matching between two strings */
int ctph_compare(const char *str1, const char *str2);

#endif