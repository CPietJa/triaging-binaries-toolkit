#ifndef SHINGLE_TABLE_H
#define SHINGLE_TABLE_H

#include <stdbool.h>
#include <stdint.h>

#define SHINGLE_TABLE_DEFAULT_SIZE 10000

#define MD5_LENGTH 16

#define ERROR_EXPAND 0
#define SUCCESSFUL_EXPAND 1

#define ERROR_INSERT 0
#define ERROR_TABLE_FULL_INSERT 1
#define NO_INSERT 2
#define SUCCESSFUL_INSERT 3

#define ERROR_REMOVE 0
#define SUCCESSFUL_REMOVE 1

/* Shingle represention */
typedef struct {
    uint8_t md5_digest[MD5_LENGTH];
    uint8_t *buffer;
    uint64_t buffer_size;
} shingle_t;

/* Hash Table (forward declaration to hide the implementation) */
typedef struct _shingle_table_t shingle_table_t;

/* Create a Hash table (a set) */
shingle_table_t *shingle_table_malloc(uint64_t size);

/*
 * Expand the table by a factor 2
 * Return:
 * - ERROR_EXPAND
 * - SUCCESSFUL_EXPAND
 */
uint8_t shingle_table_expand_size(shingle_table_t **table);

/* free the hash table */
void shingle_table_free(shingle_table_t *table);

/* Get the table size */
uint64_t shingle_table_get_size(shingle_table_t *table);

/* Get the number of shingles in the table */
uint64_t shingle_table_get_elt_nb(shingle_table_t *table);

/* Check if the table is empty */
bool shingle_table_is_empty(shingle_table_t *table);

/* Check if the table is full */
bool shingle_table_is_full(shingle_table_t *table);

/*
 * Insert a shingle in the table
 * Return:
 * - ERROR_INSERT
 * - ERROR_TABLE_FULL_INSERT
 * - NO_INSERT : if the shingle is already in the table
 * - SUCCESSFUL_INSERT
 */
uint8_t shingle_table_insert(shingle_table_t *table, shingle_t shingle);

/*
 * Remove the first shingle in the table
 * Return:
 * - ERROR_REMOVE
 * - SUCCESSFUL_REMOVE
 */
uint8_t shingle_table_remove_first(shingle_table_t *table, shingle_t *shingle);

#endif