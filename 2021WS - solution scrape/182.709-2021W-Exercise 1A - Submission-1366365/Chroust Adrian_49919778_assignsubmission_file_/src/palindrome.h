/**
 * @file palindrome.h
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 12.11.2021
 * @brief Program module which detects if a string is a palindrome.
 * @details Provides functions which detect and print char arrays that are palindromes.
 */

#ifndef PALINDROME_H
#define PALINDROME_H

#include <stdbool.h>

/**
 * @brief A flag for signaling that the find_palindromes function has run out of memory.
 * @details The flag is used as a return value of the find_palindromes function.
 */
#define FIND_PALINDROMES_OUT_OF_MEMORY_ERROR 1

/**
 * @brief A flag for signaling that the find_palindromes function could not write to the output file.
 * @details The flag is used as a return value of the find_palindromes function.
 */
#define FIND_PALINDROMES_WRITE_ERROR 2

/**
 * @brief This function checks if a certain set of characters are a palindrome.
 * @param sequence The char array which contains the characters to check. Cannot be NULL.
 * @param size The amount of characters to check in the sequence. Has to be smaller or equal to the total size of the sequence parameter.
 * @param case_insensitive Determines if the function requires the characters to be of the same case or not.
 * @param ignore_whitespace Determines if white space inside or around the character array can be skipped.
 * @return A boolean value, true if the character sequence is a palindrome, false otherwise.
 */
bool is_palindrome(const unsigned char *sequence, unsigned int size, bool case_insensitive, bool ignore_whitespace);

/**
 * @brief Reads an input file and checks for each line, if it is a palindrome. The result is then written to the output file.
 * @details If a palindrome is detected on a line "... is a palindrome" is written to the output file,
 *          "... is not a palindrome" is written otherwise.
 * @param input The input file for reading lines of sequences, can be stdin. Cannot be NULL.
 * @param output The output file for printing if a line of the input file is a palindrome or not, can be stdout. Cannot be NULL.
 * @param case_insensitive Determines if the function requires the characters to be of the same case or not.
 * @param ignore_whitespace Determines if white space inside or around the character array can be skipped.
 * @return The FIND_PALINDROMES_OUT_OF_MEMORY_ERROR if the function ran out of memory while trying to read palindromes.
 */
int find_palindromes(FILE *input, FILE *output, bool case_insensitive, bool ignore_whitespace);

#endif // PALINDROME_H
