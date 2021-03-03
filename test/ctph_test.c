
#include "../include/ctph.h"
#include "../include/edit_dist.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

char *gen_hash(char *path)
{
    FILE *f = fopen(path, "rb");
    elf_data data = elf_get_data(f);
    fclose(f);
    char *hash = ctph_hash(data);
    elf_free(data);
    return hash;
}

int main(void)
{
    /* clang-format off */
    char * str[8] = {
        "samples/hw",
        "samples/hw_2f1",
        "samples/hw_4f1",
        "samples/hw_4f1_strCon",
        "samples/hw_4f1_strDef",
        "samples/hw_f1",
        "samples/hw_f1-2",
        "samples/hw_v"
    };

    char *hash[8] = {
        gen_hash(str[0]),
        gen_hash(str[1]),
        gen_hash(str[2]),
        gen_hash(str[3]),
        gen_hash(str[4]),
        gen_hash(str[5]),
        gen_hash(str[6]),
        gen_hash(str[7])
    };
    /* clang-format on */

    // int res = edit_distn(h1, strlen(h1), h2, strlen(h2));
    // printf("%d\n", res);
    for (uint8_t i = 0; i < 7; i++) {
        printf("[File] %s : %s\n", str[i], hash[i]);
        for (uint8_t j = i + 1; j < 8; j++) {
            int res = ctph_compare(hash[i], hash[j]);
            printf("\t[ %02d%% ] %s : %s\n", res, str[j], hash[j]);
        }
        printf("\n");
    }

    char *file1 = "samples/sudoku_hw4";
    char *file2 = "samples/sudoku_hw5";

    char *h1 = gen_hash(file1);
    char *h2 = gen_hash(file2);

    printf("[ %s ] %s\n", file1, h1);
    printf("[ %s ] %s\n", file2, h2);
    int res = ctph_compare(h1, h2);
    printf("[ %02d%% ] %s : %s\n", res, file1, file2);

    return EXIT_SUCCESS;
}