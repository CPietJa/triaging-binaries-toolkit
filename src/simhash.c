#include "simhash.h"

#include <stdlib.h>

#include <openssl/md5.h>

#include "shingle_table.h"

/* clang-format off */
// uint64_t SHINGLE_SIZE[SECTION_END] =
// {
//     [INIT]      = 10000,
//     [PLT]       = 10,
//     [PLT_GOT]   = 10000,
//     [FINI]      = 1,
//     [TEXT]      = 2,
//     [RODATA]    = 24,
//     [DATA]      = 2
// };
// uint64_t SHINGLE_SIZE[SECTION_END] =
// {
//     [INIT]      = 19,
//     [PLT]       = 1,
//     [PLT_GOT]   = 1,
//     [FINI]      = 1,
//     [TEXT]      = 1,
//     [RODATA]    = 1,
//     [DATA]      = 1
// };
uint64_t SHINGLE_SIZE[SECTION_END] =
{
    [INIT]      = 10,
    [PLT]       = 100,
    [PLT_GOT]   = 10,
    [FINI]      = 2,
    [TEXT]      = 100000,
    [RODATA]    = 100000,
    [DATA]      = 100000
};
/* clang-format on */

bool simhash_compute(elf_data data, uint8_t **hash)
{
    if (data == NULL || hash == NULL)
        goto err_null;

    shingle_table_t *table = shingle_table_malloc(SHINGLE_TABLE_DEFAULT_SIZE);
    if (table == NULL)
        goto err_null;

    uint8_t *final_hash = malloc(sizeof(uint8_t) * MD5_LENGTH);
    if (final_hash == NULL)
        goto err_f_table;

    /* Get shingles */
    shingle_t sh;

    /* Sections */
    for (uint8_t i = 0; i < SECTION_END; i++) {
        uint64_t sh_size = SHINGLE_SIZE[i];
        if (data[i].len < SHINGLE_SIZE[i])
            sh_size = data[i].len;

        /* buffer */
        for (uint64_t j = 0; j < (data[i].len - sh_size); j++) {
            sh.buffer = &(data[i].data[j]);
            sh.buffer_size = sh_size;
            MD5(sh.buffer, sh.buffer_size, sh.md5_digest);

            if (shingle_table_insert(table, sh) == ERROR_TABLE_FULL_INSERT) {
                if (shingle_table_expand_size(&table) == ERROR_EXPAND)
                    goto err_f_table;

                shingle_table_insert(table, sh);
            }
        }
    }

    /* Compute hash */
    int64_t tmp_hash[MD5_LENGTH * 8];
    for (uint8_t i = 0; i < (MD5_LENGTH * 8); i++)
        tmp_hash[i] = 0;

    while (!shingle_table_is_empty(table)) {
        shingle_table_remove_first(table, &sh);

        for (uint8_t i = 0; i < MD5_LENGTH; i++) {
            for (uint8_t bit = 0; bit < 8; bit++) {
                uint8_t val_bit = ((sh.md5_digest[i]) >> bit) & 1;

                tmp_hash[i * 8 + bit] += (val_bit > 0) ? 1 : -1;
            }
        }
    }

    shingle_table_free(table);

    for (uint8_t octet = 0; octet < MD5_LENGTH; octet++) {
        uint8_t tmp_octet = 0;

        uint8_t bit = 8;
        do {
            bit--;
            uint8_t final_val = (tmp_hash[octet * 8 + bit] > 0) ? 1 : 0;

            tmp_octet <<= 1;
            tmp_octet += final_val;

        } while (bit != 0);
        final_hash[octet] = tmp_octet;
    }

    /* Update arguments */
    (*hash) = final_hash;

    return true;

err_f_table:
    shingle_table_free(table);

err_null:
    return false;
}

float simhash_compare(uint8_t *hash_1, uint8_t *hash_2)
{
    if (hash_1 == NULL || hash_2 == NULL)
        return 0;

    /* Compute hamming distance */
    uint64_t dist = 0;
    for (uint64_t i = 0; i < SIM_HASH_SIZE; i++) {
        uint64_t xor_val = hash_1[i] ^ hash_2[i];
        dist += __builtin_popcountll(xor_val);
    }

    return (1.0 - (dist / 128.0)) * 100.0;
}

char *simhash_to_string(uint8_t hash[])
{
    if (hash == NULL)
        return NULL;

    char *string = malloc(sizeof(char) * (SIM_HASH_SIZE * 2 + 1));
    if (string == NULL)
        return NULL;

    for (uint8_t i = 0; i < SIM_HASH_SIZE; i++)
        snprintf(&(string[i * 2]), 3, "%02x", hash[i]);

    return string;
}

uint8_t *simhash_string_to_uint(char *hash)
{
    if (hash == NULL)
        return NULL;

    uint8_t *hash_tab = malloc(sizeof(uint8_t) * SIM_HASH_SIZE);
    if (hash_tab == NULL)
        return NULL;

    char tmp[5] = "0x00";

    for (uint8_t i = 0; hash[i] != '\0'; i += 2) {
        tmp[2] = hash[i];
        tmp[3] = hash[i + 1];

        hash_tab[i / 2] = strtol(tmp, NULL, 16);
    }

    return hash_tab;
}