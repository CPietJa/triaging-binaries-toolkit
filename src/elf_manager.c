
#include "elf_manager.h"

#include <stdlib.h>

#include <libelf/elf.h>

/* clang-format off */
char* SECTION_NAME[SECTION_END] =
{
    [INIT]      = ".init",
    [PLT]       = ".plt",
    [PLT_GOT]   = ".plt.got",
    [FINI]      = ".fini",
    [TEXT]      = ".text",
    [RODATA]    = ".rodata",
    [DATA]      = ".data"
};
/* clang-format on */

/* Check if the file is an ELF file */
bool elf_check_header(FILE *fd)
{
    Fhdr fhdr;
    if (readelf(fd, &fhdr) == -1)
        return false;
    freeelf(&fhdr);

    return true;
}

/* Get all the section data of an ELF file */
elf_data elf_get_data(FILE *elf_fd)
{
    if (elf_fd == NULL)
        return NULL;
    if (!elf_check_header(elf_fd))
        return NULL;

    Fhdr fhdr;
    uint8_t *buf;
    uint64_t len;

    elf_data data = malloc(sizeof(section_data) * SECTION_END);
    if (!data)
        return NULL;

    for (uint8_t i = 0; i < SECTION_END; i++) {
        buf = readelfsection(elf_fd, SECTION_NAME[i], &len, &fhdr);
        if (!buf) {
            data[i].data = NULL;
            data[i].len = 0;
        } else {
            data[i].data = buf;
            data[i].len = len;
        }
        freeelf(&fhdr);
    }

    return data;
}

void elf_free(elf_data data)
{
    if (!data)
        return;

    for (uint8_t i = 0; i < SECTION_END; i++)
        if (data[i].len)
            free(data[i].data);
    free(data);
}

static bool is_printable(uint8_t c)
{
    if (c >= 32 && c <= 126)
        return true;
    /*if(c == 128)
            return true;
    if(c >= 130 && c <= 140)
            return true;
    if(c == 142)
            return true;
    if(c >= 145 && c <= 156)
            return true;
    if(c == 158 || c == 159)
            return true;
    if(c >= 161 && c <= 172)
            return true;
    if(c >= 174)
            return true;*/
    return false;
}

void elf_print_section(section_data data, section_e section)
{
    if (section < 0 || section >= SECTION_END)
        return;

    uint8_t *buf = data.data;
    uint64_t len = data.len;
    uint16_t n_bytes_max = 1024;

    printf("Section \"%s\": (first %d bytes)\n", SECTION_NAME[section],
           n_bytes_max);
    if (!len) {
        printf("\t... No data ...\n");
        return;
    }

    uint8_t n_bytes = 16;
    uint8_t tab[n_bytes];
    uint8_t col = 0, space = 0;

    printf("\t");
    for (uint64_t i = 0; i < len && i < n_bytes_max; i++, col++) {
        if (col % 4 == 0 && i != 0) {
            printf(" ");
            space++;
        }

	if (col >= n_bytes) {
            printf(" ");
            for (uint8_t j = 0; j < n_bytes; j++) {
                if (is_printable(tab[j]))
                    printf("%c", (char) tab[j]);
                else
                    printf(".");
            }
            col = space = 0;
            printf("\n\t");
        }

	tab[col] = buf[i];
        printf("%02x", buf[i]);
    }
    uint8_t offset = (n_bytes - col) * 2 + 5 - space;

    for (uint8_t j = 0; j < offset; j++) {
        printf(" ");
    }

    for (uint8_t j = 0; j < col; j++) {
        if (is_printable(tab[j]))
            printf("%c", (char) tab[j]);
        else
            printf(".");
    }
    printf("\n");
}

void elf_print_data(elf_data data)
{
    if (!data)
        return;

    for (uint8_t i = 0; i < SECTION_END; i++) {
        elf_print_section(data[i], i);
        printf("\n");
    }
}
