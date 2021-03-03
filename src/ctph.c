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

#include "edit_dist.h"

#include <stdlib.h>

#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <string.h>

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#define WINDOW_SIZE 7    /* Bytes */
#define MIN_BLOCK_SIZE 3 /* Bytes */
#define SIGN_LENGTH 64   /* Desired signature length */
#define MOD_ADLER 65521  /* Largest prime number smaller than 2^16 */
#define SIZE_MAX_SIGN 150

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
 * @param B Block size for the trigger.
 * @return char* the hash in Base64, NULL otherwise
 */
static char *ctph_hash_engine(elf_data data, uint16_t *sign_length, uint64_t B)
{
    if (data == NULL || sign_length == NULL || B == 0)
        return NULL;

    rh_state state;
    rh_init(&state);

    fnv_hash hash = FNV_OFFSET_BASIS;

    char signature[SIGN_LENGTH + 1];

    uint8_t window = 1;
    uint16_t count = 0; /* Number of Trigger */

    /* Moving the window */
    for (uint8_t i = 0; i < SECTION_END; i++) {
        if (data[i].len) {
            for (uint64_t byte = 0;
                 byte < data[i].len && count < SIGN_LENGTH - 1; byte++) {
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

    /* Copy signature */
    char *final_hash = malloc(sizeof(char) * (count + 1));
    if (final_hash == NULL)
        return NULL;
    snprintf(final_hash, count + 1, "%s", signature);

    return final_hash;
}

/**
 * @brief Return the rightmost 1 of v
 *
 * @param v the value
 * @return uint64_t the rightmost 1
 */
static uint64_t rightmost(const uint64_t v)
{
    /* The two return work. */
    /*return colors_set(__builtin_ctzll(colors));*/
    return v & -v;
}

/**
 * @brief Return the leftmost 1 of v
 *
 * @param v the value
 * @return uint64_t the leftmost 1
 */
static uint64_t leftmost(const uint64_t v)
{
    /* The two return work. */
    /*return colors_set(MAX_COLORS - 1 - __builtin_clzll(colors));*/
    return __builtin_bswap64(rightmost(__builtin_bswap64(v)));
}

/**
 * @brief Compute and return the hash of the ELF data in Base64
 *
 * @param data the ELF Data
 * @return char* the hash in Base64, NULL otherwise
 */
char *ctph_hash(elf_data data)
{
    if (!data)
        return NULL;

    char *hash = NULL;
    char *hash_prec = NULL;
    uint16_t hash_length = 0;      /* Hash with blocksize B */
    uint16_t hash_length_prec = 0; /* Hash with blocksize B*2 */
    uint16_t coef = 1;

    uint64_t B =
        MIN_BLOCK_SIZE *
        pow(2, log2(elf_get_data_size(data) /
                    (SIGN_LENGTH - MIN_BLOCK_SIZE))); /* Trigger Value */
    B = leftmost(B) << 1; /* TODO : Check if leftmost(B) == 2^63 before << 1 */
    uint64_t true_B;

    /* while hash length is inferior to 32 : Generate hash */
    do {
        true_B = B / coef; /* Adjust blocksize */
        hash = ctph_hash_engine(data, &hash_length, true_B);
        if (hash == NULL)
            return NULL;
        if (hash_length < 32) {
            if (hash_prec != NULL)
                free(hash_prec);
            hash_length_prec = hash_length;
            hash_prec = hash;
            coef *= 2;
        }
        if (hash_length >= SIGN_LENGTH) {
            free(hash);
            coef /= 2;
        }
    } while (hash_length < 32);

    if (hash_prec == NULL) {
        if (coef < 1)
            hash_prec = ctph_hash_engine(data, &hash_length_prec, true_B / 2);
        else
            hash_prec = ctph_hash_engine(data, &hash_length_prec, true_B * 2);
    }
    if (hash_prec == NULL) {
        free(hash);
        return NULL;
    }

    /* Concatenate <block size>:<hash>:<hash_prec> */
    uint16_t size = hash_length + hash_length_prec + 20 + 1 + 2;
    char *final_hash = malloc(sizeof(char) * (size));
    if (final_hash == NULL)
        return NULL;

    snprintf(final_hash, size, "%" PRIu64 ":%s:%s", true_B, hash, hash_prec);

    free(hash);
    free(hash_prec);

    return final_hash;
}

/**
 * @brief this is the low level string scoring algorithm. It takes two strings
 * and scores them on a scale of 0-100 where 0 is a terrible match and
 * 100 is a great match. The block_size is used to cope with very small
 * messages.
 */
static uint32_t score_strings(const char *s1, size_t s1len, const char *s2,
                              size_t s2len, unsigned long block_size)
{
    uint32_t score;

    // compute the edit distance between the two strings. The edit distance
    // gives us a pretty good idea of how closely related the two strings are
    score = edit_distn(s1, s1len, s2, s2len);

    // scale the edit distance by the lengths of the two
    // strings. This changes the score to be a measure of the
    // proportion of the message that has changed rather than an
    // absolute quantity. It also copes with the variability of
    // the string lengths.
    score = (score * SIGN_LENGTH) / (s1len + s2len);

    // at this stage the score occurs roughly on a 0-SIGN_LENGTH scale,
    // with 0 being a good match and SIGN_LENGTH being a complete
    // mismatch

    // rescale to a 0-100 scale (friendlier to humans)
    score = (100 * score) / SIGN_LENGTH;

    // now re-scale on a 0-100 scale with 0 being a poor match and
    // 100 being a excellent match.
    // score = 100 - score;

    //  printf ("s1len: %"PRIu32"  s2len: %"PRIu32"\n", (uint32_t)s1len,
    //  (uint32_t)s2len);

    // when the blocksize is small we don't want to exaggerate the match size
    if (block_size >= (99 + WINDOW_SIZE) / WINDOW_SIZE * MIN_BLOCK_SIZE)
        return score;
    if (score > block_size / MIN_BLOCK_SIZE * MIN(s1len, s2len)) {
        score = block_size / MIN_BLOCK_SIZE * MIN(s1len, s2len);
    }
    return 100 - score;
}

/**
 * @brief sequences contain very little information so they tend to just bias
 * the result unfairly
 */
static int copy_eliminate_sequences(char **out, size_t outsize, char **in,
                                    char etoken)
{
    size_t seq = 0;
    char prev = **in, curr;
    if (!prev || prev == etoken)
        return 1;
    if (!outsize--)
        return 0;
    *(*out)++ = prev;
    ++(*in);
    while (1) {
        curr = **in;
        if (!curr || curr == etoken)
            return 1;
        ++(*in);
        if (curr == prev) {
            if (++seq >= 3) {
                seq = 3;
                continue;
            }
            if (!outsize--)
                return 0;
            *(*out)++ = curr;
        } else {
            if (!outsize--)
                return 0;
            *(*out)++ = curr;
            seq = 0;
            prev = curr;
        }
    }
    // unreachable
    return 0;
}

/**
 * @brief Given two hash return a value indicating the degree
 * to which they match.
 */
int ctph_compare(const char *str1, const char *str2)
{
    uint64_t block_size1, block_size2;
    uint32_t score = 0;
    size_t s1b1len, s1b2len, s2b1len, s2b2len;
    char s1b1[SIGN_LENGTH], s1b2[SIGN_LENGTH];
    char s2b1[SIGN_LENGTH], s2b2[SIGN_LENGTH];
    char *s1p, *s2p, *tmp;

    if (NULL == str1 || NULL == str2)
        return -1;

    // each spamsum is prefixed by its block size
    if (sscanf(str1, "%lu:", &block_size1) != 1 ||
        sscanf(str2, "%lu:", &block_size2) != 1) {
        return -1;
    }

    // if the blocksizes don't match then we are comparing
    // apples to oranges. This isn't an 'error' per se. We could
    // have two valid signatures, but they can't be compared.
    if (block_size1 != block_size2 &&
        (block_size1 > ULONG_MAX / 2 || block_size1 * 2 != block_size2) &&
        (block_size1 % 2 == 1 || block_size1 / 2 != block_size2)) {
        return 0;
    }

    // move past the prefix
    s1p = strchr(str1, ':');
    s2p = strchr(str2, ':');

    if (!s1p || !s2p) {
        // badly formed ...
        return -1;
    }

    // there is very little information content is sequences of
    // the same character like 'LLLLL'. Eliminate any sequences
    // longer than 3 while reading two pieces.
    // This is especially important when combined with the
    // has_common_substring() test at score_strings().

    // read the first digest
    ++s1p;
    tmp = s1b1;
    if (!copy_eliminate_sequences(&tmp, SIGN_LENGTH, &s1p, ':'))
        return -1;
    s1b1len = tmp - s1b1;
    if (!*s1p++) {
        // a signature is malformed - it doesn't have 2 parts
        return -1;
    }
    tmp = s1b2;
    if (!copy_eliminate_sequences(&tmp, SIGN_LENGTH, &s1p, ','))
        return -1;
    s1b2len = tmp - s1b2;

    // read the second digest
    ++s2p;
    tmp = s2b1;
    if (!copy_eliminate_sequences(&tmp, SIGN_LENGTH, &s2p, ':'))
        return -1;
    s2b1len = tmp - s2b1;
    if (!*s2p++) {
        // a signature is malformed - it doesn't have 2 parts
        return -1;
    }
    tmp = s2b2;
    if (!copy_eliminate_sequences(&tmp, SIGN_LENGTH, &s2p, ','))
        return -1;
    s2b2len = tmp - s2b2;

    // Now that we know the strings are both well formed, are they
    // identical? We could save ourselves some work here
    if (block_size1 == block_size2 && s1b1len == s2b1len &&
        s1b2len == s2b2len) {
        if (!memcmp(s1b1, s2b1, s1b1len) && !memcmp(s1b2, s2b2, s1b2len)) {
            return 100;
        }
    }

    // each signature has a string for two block sizes. We now
    // choose how to combine the two block sizes. We checked above
    // that they have at least one block size in common
    if (block_size1 <= ULONG_MAX / 2) {
        if (block_size1 == block_size2) {
            uint32_t score1, score2;
            score1 = score_strings(s1b1, s1b1len, s2b1, s2b1len, block_size1);
            score2 =
                score_strings(s1b2, s1b2len, s2b2, s2b2len, block_size1 * 2);
            score = MAX(score1, score2);
        } else if (block_size1 * 2 == block_size2) {
            score = score_strings(s2b1, s2b1len, s1b2, s1b2len, block_size2);
        } else {
            score = score_strings(s1b1, s1b1len, s2b2, s2b2len, block_size1);
        }
    } else {
        if (block_size1 == block_size2) {
            score = score_strings(s1b1, s1b1len, s2b1, s2b1len, block_size1);
        } else if (block_size1 % 2 == 0 && block_size1 / 2 == block_size2) {
            score = score_strings(s1b1, s1b1len, s2b2, s2b2len, block_size1);
        } else {
            score = 0;
        }
    }

    return (int) score;
}