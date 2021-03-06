#ifndef SIMHASH_H
#define SIMHASH_H

#include <stdbool.h>
#include <stdint.h>

#include "elf_manager.h"

#define SIM_HASH_SIZE 16

/* Compute SimHash value of elf data */
bool simhash_compute(elf_data data, uint8_t *hash[]);

/*
 * Return the percentage of similarity betwwen the two hash
 * Using hamming distance
 */
float simhash_compare(uint8_t *hash_1, uint8_t *hash_2);

/* Convert sim hash to a string terminated by '\0' */
char *simhash_to_string(uint8_t hash[]);

/* Convert string hash into uint8_t array */
uint8_t *simhash_string_to_uint(char *hash);

#endif