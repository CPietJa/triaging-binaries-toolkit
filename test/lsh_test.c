#include "lsh.h"

#include <stdlib.h>

void get_hash(char *file, uint8_t **hash)
{
    FILE *fd_file = fopen(file, "r");

    elf_data data = elf_get_data(fd_file);

    fclose(fd_file);

    lsh_sim_hash(data, hash);

    elf_free(data);
}

double get_percent(uint64_t dist)
{
    return (1.0 - (dist / 128.0)) * 100.0;
}

int main(void)
{
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
    string_hash = lsh_sim_hash_to_string(hash_1);
    printf("SimHash %s = %s\n", elf_file_1, string_hash);
    free(string_hash);
    string_hash = lsh_sim_hash_to_string(hash_2);
    printf("SimHash %s = %s\n", elf_file_2, string_hash);
    free(string_hash);
    string_hash = lsh_sim_hash_to_string(hash_3);
    printf("SimHash %s = %s\n", elf_file_3, string_hash);
    free(string_hash);
    string_hash = lsh_sim_hash_to_string(hash_4);
    printf("SimHash %s = %s\n", elf_file_4, string_hash);
    free(string_hash);
    string_hash = lsh_sim_hash_to_string(hash_5);
    printf("SimHash %s = %s\n\n", elf_file_5, string_hash);
    free(string_hash);

    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_2,
           get_percent(lsh_hamming_distance(hash_1, hash_2, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_3,
           get_percent(lsh_hamming_distance(hash_1, hash_3, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_4,
           get_percent(lsh_hamming_distance(hash_1, hash_4, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_5,
           get_percent(lsh_hamming_distance(hash_1, hash_5, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_3,
           get_percent(lsh_hamming_distance(hash_2, hash_3, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_4,
           get_percent(lsh_hamming_distance(hash_2, hash_4, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_5,
           get_percent(lsh_hamming_distance(hash_2, hash_5, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_3, elf_file_4,
           get_percent(lsh_hamming_distance(hash_3, hash_4, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_3, elf_file_5,
           get_percent(lsh_hamming_distance(hash_3, hash_5, SIM_HASH_SIZE)));
    printf("--> %s - %s: %.2f %%\n", elf_file_4, elf_file_5,
           get_percent(lsh_hamming_distance(hash_4, hash_5, SIM_HASH_SIZE)));

    free(hash_1);
    free(hash_2);
    free(hash_3);
    free(hash_4);
    free(hash_5);

    return 0;
}
