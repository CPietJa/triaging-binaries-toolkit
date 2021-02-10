#ifndef ELF_MANAGER
#define ELF_MANAGER

#include <stdbool.h>
#include <stdio.h>

#include <inttypes.h>

/* Stock data of an ELF section */
typedef struct {
    uint64_t len;
    uint8_t *data;
} section_data;

typedef section_data *elf_data;

/* clang-format off */
typedef enum
{
  INIT,
  PLT,
  PLT_GOT,
  FINI,
  TEXT,
  RODATA,
  DATA,
  SECTION_END
} section_e;
/* clang-format on */

extern char *SECTION_NAME[SECTION_END];

bool elf_check_header(FILE *fd);
elf_data elf_get_data(FILE *elf_fd);
void elf_free(elf_data data);
void elf_print_section(section_data data, section_e section);
void elf_print_data(elf_data data);
uint64_t elf_get_data_size(elf_data data);

#endif /* ELF_MANAGER */