#include "lsh.h"

#include <stdlib.h>

void print_hash(uint8_t *hash, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
        printf("%02x", hash[i]);
}

void get_hash(char *file, uint8_t **hash, uint8_t *len)
{
    FILE *fd_file = fopen(file, "r");

    elf_data data = elf_get_data(fd_file);

    fclose(fd_file);

    lsh_sim_hash(data, hash, len);

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
    uint8_t len_1 = 0;
    uint8_t *hash_2 = NULL;
    uint8_t len_2 = 0;
    uint8_t *hash_3 = NULL;
    uint8_t len_3 = 0;
    uint8_t *hash_4 = NULL;
    uint8_t len_4 = 0;
    uint8_t *hash_5 = NULL;
    uint8_t len_5 = 0;

    get_hash(elf_file_1, &hash_1, &len_1);
    get_hash(elf_file_2, &hash_2, &len_2);
    get_hash(elf_file_3, &hash_3, &len_3);
    get_hash(elf_file_4, &hash_4, &len_4);
    get_hash(elf_file_5, &hash_5, &len_5);

    /* Print */
    printf("SimHash %s = ", elf_file_1);
    print_hash(hash_1, len_1);
    printf("\n");
    printf("SimHash %s = ", elf_file_2);
    print_hash(hash_2, len_2);
    printf("\n");
    printf("SimHash %s = ", elf_file_3);
    print_hash(hash_3, len_3);
    printf("\n");
    printf("SimHash %s = ", elf_file_4);
    print_hash(hash_4, len_4);
    printf("\n");
    printf("SimHash %s = ", elf_file_5);
    print_hash(hash_5, len_5);
    printf("\n\n");
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_2,
           get_percent(lsh_hamming_distance(hash_1, hash_2, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_3,
           get_percent(lsh_hamming_distance(hash_1, hash_3, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_4,
           get_percent(lsh_hamming_distance(hash_1, hash_4, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_1, elf_file_5,
           get_percent(lsh_hamming_distance(hash_1, hash_5, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_3,
           get_percent(lsh_hamming_distance(hash_2, hash_3, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_4,
           get_percent(lsh_hamming_distance(hash_2, hash_4, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_2, elf_file_5,
           get_percent(lsh_hamming_distance(hash_2, hash_5, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_3, elf_file_4,
           get_percent(lsh_hamming_distance(hash_3, hash_4, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_3, elf_file_5,
           get_percent(lsh_hamming_distance(hash_3, hash_5, len_1)));
    printf("--> %s - %s: %.2f %%\n", elf_file_4, elf_file_5,
           get_percent(lsh_hamming_distance(hash_4, hash_5, len_1)));

    free(hash_1);
    free(hash_2);
    free(hash_3);
    free(hash_4);
    free(hash_5);

    return 0;
}
