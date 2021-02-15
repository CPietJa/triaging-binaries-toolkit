/**
 * @file ctph.c
 * @author CPietJa
 * @brief Module which implements Context-Triggered Piecewise Hashing (CTPH)
 * @version 0.1
 * @date 2021-02-10
 *
 * To implement CTPH, we use a rolling hash (Adler-32) and a FNV Hash :
 * - Adler-32 : https://en.wikipedia.org/wiki/Adler-32#The_algorithm
 * - FNV-1 Hash :
 * https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */

#include "ctph.h"

#include <stdlib.h>

#include <math.h>
#include <string.h>

#define WINDOW_SIZE 7    /* Bytes */
#define MIN_BLOCK_SIZE 3 /* Bytes */
#define SIGN_LENGTH 64   /* Desired signature length */
#define MOD_ADLER 65521  /* Largest prime number smaller than 2^16 */

/* clang-format off */
typedef struct {
    uint16_t a; /* 16-bit checksum : Sum of all bytes in the stream plus one */
    uint16_t b; /* 16-bit checksum : Sum of the individual values of 'a' from each step */
    uint32_t h; /* 32-bit checksum : Concatenation of B and A in this order */
} rh_state;
/* clang-format on */

/* clang-format off */
#define FNV_OFFSET_BASIS 0xcbf29ce484222325 /* Initial value of the FNV hash */
#define FNV_PRIME 0x100000001b3             /* Prime number */
/* clang-format on */

/* The FNV Hash is a 64-bit value */
typedef uint64_t fnv_hash;

/**
 * @brief Initialize the rolling hash states.
 *
 * @param state rolling hash states
 * @return true if no problem
 * @return false if state NULL
 */
static bool rh_init(rh_state *state)
{
    if (state == NULL)
        return false;

    state->a = 1;
    state->b = 0;
    state->h = 0;

    return true;
}

/**
 * @brief Update the rolling hash with one byte
 *
 * @param state rolling hash states
 * @param byte byte to add
 * @return true if no problem
 * @return false if state NULL
 */
static bool rh_add_byte(rh_state *state, uint8_t byte)
{
    if (state == NULL)
        return false;

    state->a = (state->a + byte) % MOD_ADLER;
    state->b = (state->b + state->a) % MOD_ADLER;

    state->h = (state->b << 16) | state->a;

    return true;
}

/**
 * @brief Update the FNV hash with one byte
 *
 * @param hash hash to update
 * @param byte byte to add
 * @return true if no problem
 * @return false if hash NULL
 */
static bool fnv_add_byte(fnv_hash *hash, uint8_t byte)
{
    if (hash == NULL)
        return false;

    (*hash) *= FNV_PRIME;
    (*hash) ^= byte;

    return true;
}

/* Base 64 */
static const char *b64 =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * @brief Compute and return the hash of the ELF data in Base64
 *
 * @param data the ELF Data
 * @param sign_length put the size of the hash in it
 * @param coef adjust the block size for the trigger.
 * @return uint8_t* the hash in Base64, NULL otherwise
 */
static uint8_t *ctph_hash_engine(elf_data data, uint16_t *sign_length,
                                 uint8_t coef)
{
    if (!data || coef == 0)
        return NULL;

    rh_state state;
    rh_init(&state);

    fnv_hash hash = FNV_OFFSET_BASIS;

    uint8_t signature[SIGN_LENGTH + 1];

    uint8_t window = 1;
    uint32_t B =
        MIN_BLOCK_SIZE *
        pow(2, log2(elf_get_data_size(data) /
                    (SIGN_LENGTH - MIN_BLOCK_SIZE))); /* Trigger Value */
    B /= coef;
    uint16_t count = 0; /* Number of Trigger */

    /* Moving the window */
    for (uint8_t i = 0; i < SECTION_END; i++) {
        if (data[i].len) {
            for (uint64_t byte = 0; byte < data[i].len; byte++) {
                /* Update Rolling Hash Value */
                rh_add_byte(&state, data[i].data[byte]);

                /* Update FNV Hash */
                fnv_add_byte(&hash, data[i].data[byte]);

                /* Check Window Size */
                if (window < WINDOW_SIZE)
                    window++;
                else {
                    /* Check Trigger Point */
                    if (state.h % B)
                        continue;
                    else { /* We have trigger */
                        /* Update Signature */
                        signature[count] = b64[hash & 0x3F];
                        count++;

                        /* Reset FNV Hash */
                        hash = FNV_OFFSET_BASIS;
                    }
                }
            }
        }
    }
    /* Last hash between last trigger point and end of the data */
    if (hash != FNV_OFFSET_BASIS) {
        signature[count] = b64[hash & 0x3F];
        count++;
    }

    (*sign_length) = count - 1;
    signature[count] = '\0';

    /* Copy Signature */
    uint8_t *final_hash = malloc(sizeof(uint8_t) * (count + 1));
    if (final_hash == NULL)
        return NULL;

    memcpy(final_hash, signature, count + 1);
    return final_hash;
}

/**
 * @brief Compute and return the hash of the ELF data in Base64
 *
 * @param data the ELF Data
 * @return uint8_t* the hash in Base64, NULL otherwise
 */
uint8_t *ctph_hash(elf_data data)
{
    if (!data)
        return NULL;

    uint8_t *hash = NULL;
    uint16_t hash_length = 0;
    uint8_t coef = 1;

    do {
        hash = ctph_hash_engine(data, &hash_length, coef);
        if (hash == NULL)
            return NULL;
        if (hash_length < 32) {
            free(hash);
            coef *= 2;
        }
    } while (hash_length < 32);

    return hash;
}
