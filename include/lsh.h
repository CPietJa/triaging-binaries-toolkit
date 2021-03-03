#ifndef LSH_H
#define LSH_H

#include <stdbool.h>
#include <stdint.h>

#include "elf_manager.h"

#define SIM_HASH_SIZE 16

/* Compute SimHash value of elf data */
bool lsh_sim_hash(elf_data data, uint8_t *hash[]);

/**/
uint64_t lsh_hamming_distance(uint8_t *hash_1, uint8_t *hash_2, uint64_t len);

/**/
uint64_t lsh_get_shingle_size(section_e sec);
/**/
void lsh_set_shingle_size(section_e sec, uint64_t new_val);

/* Convert sim hash to a string terminated by '\0' */
char *lsh_sim_hash_to_string(uint8_t hash[]);

#endif