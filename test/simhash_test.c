#include "simhash.h"

#include <stdlib.h>

void get_hash(char *file, uint8_t **hash)
{
    FILE *fd_file = fopen(file, "r");

    elf_data data = elf_get_data(fd_file);

    fclose(fd_file);

    simhash_compute(data, hash);

    elf_free(data);
}

int main(void)
{
    /* Test convert function */
    printf("----( Check Convert function )----\n");
    uint8_t hash_verif_uint[SIM_HASH_SIZE] = {
        244, 159, 47, 183, 56, 85, 41, 7, 41, 234, 240, 215, 241, 29, 90, 180};
    char *hash_string = simhash_to_string(hash_verif_uint);
    uint8_t *hash_uint = simhash_string_to_uint(hash_string);

    printf("BASE: [");
    for (uint8_t i = 0; i < SIM_HASH_SIZE; i++) {
        if (i + 1 == SIM_HASH_SIZE)
            printf("%u", hash_verif_uint[i]);
        else
            printf("%u, ", hash_verif_uint[i]);
    }
    printf("]\n");
    printf("BASE -> STRING: %s\n", hash_string);
    printf("STRING -> UINT: [");
    for (uint8_t i = 0; i < SIM_HASH_SIZE; i++) {
        if (i + 1 == SIM_HASH_SIZE)
            printf("%u", hash_uint[i]);
        else
            printf("%u, ", hash_uint[i]);
    }
    printf("]\n");

    /* Check */
    bool verif = true;
    for (uint8_t i = 0; i < SIM_HASH_SIZE; i++) {
        if (hash_uint[i] != hash_verif_uint[i])
            verif = false;
    }

    printf("BASE == UINT: %s\n\n", verif ? "true" : "false");

    free(hash_string);
    free(hash_uint);

    /* Comparison */
    printf("----( Check Simple Comparison )----\n");
    char *elf_file_1 = "samples/hello_1";
    char *elf_file_2 = "samples/hello_2";
    char *elf_file_3 = "samples/J.G-sudoku_H4";
    char *elf_file_4 = "samples/J.G-sudoku_H5";
    char *elf_file_5 = "samples/J.G-sudoku_P";

    /* Compute sim hash */
    uint8_t *hash_1 = NULL;
    uint8_t *hash_2 = NULL;
    uint8_t *hash_3 = NULL;
    uint8_t *hash_4 = NULL;
    uint8_t *hash_5 = NULL;

    get_hash(elf_file_1, &hash_1);
    get_hash(elf_file_2, &hash_2);
    get_hash(elf_file_3, &hash_3);
    get_hash(elf_file_4, &hash_4);
    get_hash(elf_file_5, &hash_5);

    /* Print */
    char *string_hash = NULL;
    string_hash = simhash_to_string(hash_1);
    printf("SimHash %s = %s\n", elf_file_1, string_hash);
    free(string_hash);
    string_hash = simhash_to_string(hash_2);
    printf("SimHash %s = %s\n", elf_file_2, string_hash);
    free(string_hash);
    string_hash = simhash_to_string(hash_3);
    printf("SimHash %s = %s\n", elf_file_3, string_hash);
    free(string_hash);
    string_hash = simhash_to_string(hash_4);
    printf("SimHash %s = %s\n", elf_file_4, string_hash);
    free(string_hash);
    string_hash = simhash_to_string(hash_5);
    printf("SimHash %s = %s\n\n", elf_file_5, string_hash);
    free(string_hash);

    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_2,
           simhash_compare(hash_1, hash_2));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_3,
           simhash_compare(hash_1, hash_3));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_4,
           simhash_compare(hash_1, hash_4));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_5,
           simhash_compare(hash_1, hash_5));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_3,
           simhash_compare(hash_2, hash_3));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_4,
           simhash_compare(hash_2, hash_4));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_5,
           simhash_compare(hash_2, hash_5));
    printf("--> %s - %s: %.2f %%\n", elf_file_3, elf_file_4,
           simhash_compare(hash_3, hash_4));
    printf("--> %s - %s: %.2f %%\n", elf_file_3, elf_file_5,
           simhash_compare(hash_3, hash_5));
    printf("--> %s - %s: %.2f %%\n", elf_file_4, elf_file_5,
           simhash_compare(hash_4, hash_5));

    free(hash_1);
    free(hash_2);
    free(hash_3);
    free(hash_4);
    free(hash_5);

    return 0;
}
