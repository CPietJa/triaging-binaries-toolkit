#include "shingle_table.h"

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <err.h>
#include <openssl/md5.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static void EXPECT(bool test, char *fmt, ...)
{
    fprintf(stdout, "Checking '");

    va_list vargs;
    va_start(vargs, fmt);
    vprintf(fmt, vargs);
    va_end(vargs);

    if (test)
        fprintf(stdout, "': (passed)\n");
    else
        fprintf(stdout, "': (failed!)\n");
}

static uint8_t *gen_rand_buf(uint64_t size)
{
    uint8_t *buf = malloc(sizeof(uint8_t) * size);
    if (buf == NULL)
        errx(EXIT_FAILURE, "buffer malloc!");

    for (uint64_t i = 0; i < size; i++) {
        buf[i] = rand() % 256;
    }

    return buf;
}

int main()
{
    srand(time(NULL) * getpid());

    shingle_t sh, sh2, sh_tmp;

    /* Test shingle_table_malloc */
    printf("----( Check shingle_table_malloc )----\n");

    EXPECT((shingle_table_malloc(0) == NULL),
           "shingle_table_malloc(0) == NULL");
    shingle_table_t *table = shingle_table_malloc(SHINGLE_TABLE_DEFAULT_SIZE);
    EXPECT((table != NULL),
           "shingle_table_malloc(SHINGLE_TABLE_DEFAULT_SIZE) != NULL");

    printf("\n");

    /* Test shingle_table_get_size */
    printf("----( Check shingle_table_get_size )----\n");

    EXPECT((shingle_table_get_size(NULL) == 0),
           "shingle_table_get_size(NULL) == 0");
    EXPECT((shingle_table_get_size(table) == SHINGLE_TABLE_DEFAULT_SIZE),
           "shingle_table_get_size(table) == SHINGLE_TABLE_DEFAULT_SIZE");

    printf("\n");

    /* Test shingle_table_is_empty */
    printf("----( Check shingle_table_is_empty )----\n");

    shingle_table_t *table2 = shingle_table_malloc(SHINGLE_TABLE_DEFAULT_SIZE);

    EXPECT((shingle_table_is_empty(NULL) == true),
           "shingle_table_is_empty(NULL) == true");
    EXPECT((shingle_table_is_empty(table2) == true),
           "shingle_table_is_empty(table_empty) == true");

    EXPECT((shingle_table_malloc(0) == NULL),
           "shingle_table_malloc(0) != NULL");

    sh.buffer_size = 100;
    sh.buffer = gen_rand_buf(sh.buffer_size);
    MD5(sh.buffer, sh.buffer_size, sh.md5_digest);

    shingle_table_insert(table2, sh);

    EXPECT((shingle_table_is_empty(table2) == false),
           "shingle_table_is_empty(table_1_elt) == false");
    shingle_table_free(table2);

    printf("\n");

    /* Test shingle_table_is_full */
    printf("----( Check shingle_table_is_full )----\n");

    table2 = shingle_table_malloc(1);

    EXPECT((shingle_table_is_full(NULL) == false),
           "shingle_table_is_full(NULL) == false");
    EXPECT((shingle_table_is_full(table2) == false),
           "shingle_table_is_full(table_empty) == false");

    shingle_table_insert(table2, sh);

    EXPECT((shingle_table_is_full(table2) == true),
           "shingle_table_is_full(table_full) == true");
    shingle_table_free(table2);

    printf("\n");

    /* Test shingle_table_insert */
    printf("----( Check shingle_table_insert )----\n");

    EXPECT((shingle_table_insert(NULL, sh) == ERROR_INSERT),
           "shingle_table_insert(NULL, sh) == ERROR_INSERT");

    table2 = shingle_table_malloc(1);
    shingle_table_insert(table2, sh);
    EXPECT((shingle_table_insert(table2, sh) == ERROR_TABLE_FULL_INSERT),
           "shingle_table_insert(table_full, sh) == ERROR_TABLE_FULL_INSERT");

    shingle_table_free(table2);

    EXPECT((shingle_table_insert(table, sh) == SUCCESSFUL_INSERT),
           "shingle_table_insert(table, sh) == SUCCESSFUL_INSERT");
    EXPECT((shingle_table_insert(table, sh) == NO_INSERT),
           "shingle_table_insert(table, sh) == NO_INSERT");

    sh2.buffer_size = 50;
    sh2.buffer = gen_rand_buf(sh2.buffer_size);
    MD5(sh2.buffer, sh2.buffer_size, sh2.md5_digest);

    EXPECT((shingle_table_insert(table, sh2) == SUCCESSFUL_INSERT),
           "shingle_table_insert(table, sh2) == SUCCESSFUL_INSERT");
    EXPECT((shingle_table_insert(table, sh2) == NO_INSERT),
           "shingle_table_insert(table, sh2) == NO_INSERT");

    printf("\n");

    /* Test shingle_table_get_elt_nb */
    printf("----( Check shingle_table_get_elt_nb )----\n");

    EXPECT((shingle_table_get_elt_nb(NULL) == 0),
           "shingle_table_get_elt_nb(NULL) == 0");

    table2 = shingle_table_malloc(SHINGLE_TABLE_DEFAULT_SIZE);

    EXPECT((shingle_table_get_elt_nb(table2) == 0),
           "shingle_table_get_elt_nb(table_empty) == 0");

    shingle_table_free(table2);

    EXPECT((shingle_table_get_elt_nb(table) == 2),
           "shingle_table_get_elt_nb(table) == 2");

    printf("\n");

    /* Test shingle_table_expand_size */
    printf("----( Check shingle_table_expand_size )----\n");

    EXPECT((shingle_table_expand_size(NULL) == ERROR_EXPAND),
           "shingle_table_expand_size(NULL) == ERROR_EXPAND");
    shingle_table_t *p_null = NULL;
    EXPECT((shingle_table_expand_size(&(p_null)) == ERROR_EXPAND),
           "shingle_table_expand_size(&(pointer_null)) == ERROR_EXPAND");

    EXPECT((shingle_table_expand_size(&table) == SUCCESSFUL_EXPAND),
           "shingle_table_expand_size(&table) == SUCCESSFUL_EXPAND");

    printf("\n");

    /* Test shingle_table_remove_first */
    printf("----( Check shingle_table_remove_first )----\n");

    EXPECT((shingle_table_remove_first(NULL, NULL) == ERROR_REMOVE),
           "shingle_table_remove_first(NULL, NULL) == ERROR_REMOVE");
    EXPECT((shingle_table_remove_first(NULL, &sh_tmp) == ERROR_REMOVE),
           "shingle_table_remove_first(NULL, &sh_tmp) == ERROR_REMOVE");
    EXPECT((shingle_table_remove_first(table, NULL) == ERROR_REMOVE),
           "shingle_table_remove_first(table, NULL) == ERROR_REMOVE");

    EXPECT((shingle_table_remove_first(table, &sh_tmp) == SUCCESSFUL_REMOVE),
           "shingle_table_remove_first(table_2_elt, &sh_tmp) == "
           "SUCCESSFUL_REMOVE");
    EXPECT((shingle_table_remove_first(table, &sh_tmp) == SUCCESSFUL_REMOVE),
           "shingle_table_remove_first(table_1_elt, &sh_tmp) == "
           "SUCCESSFUL_REMOVE");
    EXPECT((shingle_table_remove_first(table, &sh_tmp) == ERROR_REMOVE),
           "shingle_table_remove_first(table_empty, &sh_tmp) == ERROR_REMOVE");

    printf("\n");

    shingle_table_free(table);

    free(sh.buffer);
    free(sh2.buffer);

    /* Test multiples insertions */
    printf("----( Check multiples insertions )----\n");

    printf("[+] Create table of size %u...\n", SHINGLE_TABLE_DEFAULT_SIZE);
    shingle_table_t *big_table =
        shingle_table_malloc(SHINGLE_TABLE_DEFAULT_SIZE);

    printf("[+] Fully fill the table...\n");
    uint64_t count = 0;
    while (!shingle_table_is_full(big_table)) {
        sh.buffer_size = 100;
        sh.buffer = gen_rand_buf(sh.buffer_size);
        MD5(sh.buffer, sh.buffer_size, sh.md5_digest);

        switch (shingle_table_insert(big_table, sh)) {
        case SUCCESSFUL_INSERT:
            count++;
            break;
        default:
            free(sh.buffer);
        }
    }
    printf("[!] Insertions: %" PRIu64 "\n", count);

    printf("[+] Expand table...\n");
    shingle_table_expand_size(&big_table);
    printf("[!] New size: %" PRIu64 "\n", shingle_table_get_size(big_table));
    printf("[!] Nb elements: %" PRIu64 "\n",
           shingle_table_get_elt_nb(big_table));

    printf("[+] Fully fill the table...\n");
    count = 0;
    while (!shingle_table_is_full(big_table)) {
        sh.buffer_size = 100;
        sh.buffer = gen_rand_buf(sh.buffer_size);
        MD5(sh.buffer, sh.buffer_size, sh.md5_digest);

        switch (shingle_table_insert(big_table, sh)) {
        case SUCCESSFUL_INSERT:
            count++;
            break;
        default:
            free(sh.buffer);
        }
    }
    printf("[!] Insertions: %" PRIu64 "\n", count);

    printf("[+] Remove all elements...\n");
    count = 0;
    while (!shingle_table_is_empty(big_table)) {
        shingle_table_remove_first(big_table, &sh);
        free(sh.buffer);
        count++;
        switch (shingle_table_remove_first(big_table, &sh)) {
        case SUCCESSFUL_REMOVE:
            count++;
            free(sh.buffer);
            break;
        default:
            break;
        }
    }
    printf("[!] Deletions: %" PRIu64 "\n", count);

    printf("[+] Free table...\n");
    shingle_table_free(big_table);

    return EXIT_SUCCESS;
}
