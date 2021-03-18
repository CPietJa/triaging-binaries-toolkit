#ifndef SIMHASH_H
#define SIMHASH_H

#include <stdbool.h>
#include <stdint.h>

#include "elf_manager.h"

/* Compute SimHash value of elf data */
char *simhash_compute(elf_data data);

/*
 * Return the percentage of similarity betwwen the two hash
 * Using hamming distance
 */
float simhash_compare(char *hash_1, char *hash_2);

#endif