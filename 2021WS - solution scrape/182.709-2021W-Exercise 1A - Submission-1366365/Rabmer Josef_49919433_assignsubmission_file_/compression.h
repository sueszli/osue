/**
 * @file compression.h
 * @author Josef Rabmer 11911128
 * 
 * @brief the header file declares the function compress_input_print_to_output, which is used to
 *        actually compress something.
 * 
 * @details it includes stdio.h, to define the FILE type.
 * 
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>

/**
 * @brief the function takes an input stream, from which it reads and compresses a sequence of characters and prints it
 *        to the output stream.
 * 
 * @details the function uses the posix file stream methods to extract data from the input, character by character. It remembers
 *          the previous character and if it is equal to the current char, it increments a counter. Once the current char, is not equal
 *          to the previous one anymore, it converts the counter into a char array and puts it into the output. It then starts again with the
 *          current character.
 * 
 *          The function converts an integer into a char array by first finding out how many digits it takes, to represent the number. It then
 *          reserves that amount. It uses sprintf to parse the int into a char array and stores it.
 * 
 * @param in input file stream
 * @param out output file stream
 * @param char_count_array pointer to two dimensional array, that stores the number of written characters and the number of read characters
 */
void compress_input_print_to_output(FILE *in, FILE *out, int *char_count_array);