#include "shingle_table.h"

#include <stdio.h>
#include <stdlib.h>

/* Internal structure (hiden from outside) to represent a hash table */
struct _shingle_table_t {
    uint64_t size;
    uint64_t elt_count;
    uint64_t index_first;

    shingle_t **table;
};

/* Static Functions */
static uint64_t get_hash(uint8_t md5_digest[MD5_LENGTH], uint64_t table_size)
{
    uint64_t hash = 1;

    for (uint64_t i = 0; i < MD5_LENGTH; i++)
        hash *= md5_digest[i];
    hash %= table_size;

    for (uint64_t i = 0; i < MD5_LENGTH; i++)
        hash += md5_digest[i];
    hash %= table_size;

    return hash;
}

static bool is_md5_equal(uint8_t md5_digest_1[MD5_LENGTH],
                         uint8_t md5_digest_2[MD5_LENGTH])
{
    for (uint64_t i = 0; i < MD5_LENGTH; i++)
        if (md5_digest_1[i] != md5_digest_2[i])
            return false;

    return true;
}

static void copy_md5(uint8_t src[MD5_LENGTH], uint8_t dest[MD5_LENGTH])
{
    for (uint8_t i = 0; i < MD5_LENGTH; i++)
        dest[i] = src[i];
}

static shingle_t *shingle_malloc(void)
{
    shingle_t *shingle = malloc(sizeof(shingle_t));
    return shingle;
}

/* External functions */
shingle_table_t *shingle_table_malloc(uint64_t size)
{
    if (size == 0)
        return NULL;

    shingle_table_t *table = malloc(sizeof(shingle_table_t));
    if (table == NULL)
        goto err_table;

    table->table = malloc(sizeof(shingle_t *) * size);
    if (table->table == NULL)
        goto err_table_shingle;

    table->size = size;
    table->elt_count = 0;
    table->index_first = (uint64_t) -1;

    /* Init all elements to NULL */
    for (uint64_t i = 0; i < size; i++)
        table->table[i] = NULL;

    return table;

    /* Errors */
err_table_shingle:
    free(table);
err_table:
    return NULL;
}

uint8_t shingle_table_expand_size(shingle_table_t **table)
{
    if (table == NULL || (*table) == NULL)
        return ERROR_EXPAND;

    shingle_table_t *old_table = *table;
    shingle_table_t *new_table = shingle_table_malloc(old_table->size * 2);
    if (new_table == NULL)
        return ERROR_EXPAND;

    /* Insert shingle from previous table to the new table */
    shingle_t sh;
    while (!shingle_table_is_empty(old_table)) {
        shingle_table_remove_first(old_table, &sh);
        shingle_table_insert(new_table, sh);
    }

    shingle_table_free(old_table);

    *table = new_table;

    return SUCCESSFUL_EXPAND;
}

void shingle_table_free(shingle_table_t *table)
{
    if (table == NULL)
        return;

    if (table->table != NULL) {

        if (!shingle_table_is_empty(table))
            for (uint64_t i = 0; i < table->size; i++)
                if (table->table[i] != NULL)
                    free(table->table[i]);

        free(table->table);
    }

    free(table);
}

uint64_t shingle_table_get_size(shingle_table_t *table)
{
    if (table == NULL)
        return 0;
    return table->size;
}

uint64_t shingle_table_get_elt_nb(shingle_table_t *table)
{
    if (table == NULL)
        return 0;
    return table->elt_count;
}

bool shingle_table_is_empty(shingle_table_t *table)
{
    if (table == NULL)
        return true;
    return (table->elt_count == 0);
}

bool shingle_table_is_full(shingle_table_t *table)
{
    if (table == NULL)
        return false;
    return (table->elt_count == table->size);
}

uint8_t shingle_table_insert(shingle_table_t *table, shingle_t shingle)
{
    if (table == NULL)
        return ERROR_INSERT;

    if (shingle_table_is_full(table))
        return ERROR_TABLE_FULL_INSERT;

    /* Compute index */
    uint64_t index = get_hash(shingle.md5_digest, table->size);

    /* Find an available index */
    uint64_t new_ind;
    bool found = false;
    for (uint64_t i = index; !found; i = (i + 1) % table->size) {
        if (table->table[i] == NULL) {
            new_ind = i;
            found = true;
        } else if (is_md5_equal(shingle.md5_digest,
                                table->table[i]->md5_digest))
            return NO_INSERT;
    }

    /* Insert in the table */
    table->table[new_ind] = shingle_malloc();
    if (table->table[new_ind] == NULL)
        return ERROR_INSERT;

    copy_md5(shingle.md5_digest, table->table[new_ind]->md5_digest);
    table->table[new_ind]->buffer = shingle.buffer;
    table->table[new_ind]->buffer_size = shingle.buffer_size;

    (table->elt_count)++;
    if (new_ind < table->index_first)
        table->index_first = new_ind;

    return SUCCESSFUL_INSERT;
}

uint8_t shingle_table_remove_first(shingle_table_t *table, shingle_t *shingle)
{
    if (table == NULL || shingle == NULL)
        return ERROR_REMOVE;
    if (shingle_table_is_empty(table))
        return ERROR_REMOVE;

    shingle_t *sh = table->table[table->index_first];
    copy_md5(sh->md5_digest, shingle->md5_digest);
    shingle->buffer = sh->buffer;
    shingle->buffer_size = sh->buffer_size;

    /* Remove shingle from table */
    free(sh);
    table->table[table->index_first] = NULL;
    (table->elt_count)--;

    /* Find new first element */
    if (shingle_table_is_empty(table)) {
        table->index_first = (uint64_t) -1;
    } else {
        uint64_t i = (table->index_first + 1) % table->size;
        while (table->table[i] == NULL)
            i = (i + 1) % table->size;

        table->index_first = i;
    }

    return SUCCESSFUL_REMOVE;
}