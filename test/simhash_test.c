#include "simhash.h"

#include <stdlib.h>

void get_hash(char *file, char **hash)
{
    FILE *fd_file = fopen(file, "r");

    elf_data data = elf_get_data(fd_file);

    fclose(fd_file);

    *hash = simhash_compute(data);

    elf_free(data);
}

int main(void)
{
    /* Comparison */
    printf("----( Check Simple Comparison )----\n");
    char *elf_file_1 = "samples/hello_1";
    char *elf_file_2 = "samples/hello_2";
    char *elf_file_3 = "samples/J.G-sudoku_H4";
    char *elf_file_4 = "samples/J.G-sudoku_H5";
    char *elf_file_5 = "samples/J.G-sudoku_P";

    /* Compute sim hash */
    char *hash_1 = NULL;
    char *hash_2 = NULL;
    char *hash_3 = NULL;
    char *hash_4 = NULL;
    char *hash_5 = NULL;

    get_hash(elf_file_1, &hash_1);
    get_hash(elf_file_2, &hash_2);
    get_hash(elf_file_3, &hash_3);
    get_hash(elf_file_4, &hash_4);
    get_hash(elf_file_5, &hash_5);

    /* Print */
    printf("SimHash %s = %s\n", elf_file_1, hash_1);
    printf("SimHash %s = %s\n", elf_file_2, hash_2);
    printf("SimHash %s = %s\n", elf_file_3, hash_3);
    printf("SimHash %s = %s\n", elf_file_4, hash_4);
    printf("SimHash %s = %s\n\n", elf_file_5, hash_5);

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
