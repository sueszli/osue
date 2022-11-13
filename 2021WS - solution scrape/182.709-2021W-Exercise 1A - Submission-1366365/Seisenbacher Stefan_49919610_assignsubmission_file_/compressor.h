/**
 * @file compressor.h
 * @author Stefan Seisenbacher
 * @date 09.11.2021
 *  
 * @brief compresses a string
 *
 * Stringcompressor
 */
#include "stdlib.h"
#include "stdio.h"

/**
 * Compresses a string
 * @brief Simple reads to a file
 * @details Compresses a string like "sheeesh" to "s1h1e3s1h1"
 * @param input String to compress
 * @return Compressed string
 */
char* compress(char *input);