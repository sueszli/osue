/**
 * @file palindrome.h
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 12.11.2021
 * @brief Implementation of the module defined in "palindrome.h".
 */

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>

#include "palindrome.h"

/**
 * @brief The function checks, if a sequence is a palindrome by comparing
 *        each character on the left to its supposed equivalent on the right.
 * @details The function sets indexes on the left and right, compares them and then
 *          moves inwards to repeat the process until left and right index pass each other.
 *          If the parameter ignore_whitespace is true, then an index is skipped, if it matches a whitespace character.
 *          If the parameter case_insensitive is true, then the characters are converted to lowercase before being compared.
 */
bool is_palindrome(const unsigned char *sequence, unsigned int size, bool case_insensitive, bool ignore_whitespace) {
    // Left and right index of the sequence.
    unsigned long left_index = 0;
    unsigned long right_index = size - 1;

    // As long as left and right index have not passed each other, the process should be repeated.
    while (left_index <= right_index) {
        // The respective characters found at the left_index and right_index.
        unsigned char left_char = sequence[left_index];
        unsigned char right_char = sequence[right_index];

        // If ignore_whitespace is true, then whitespace characters are skipped.
        if (ignore_whitespace) {
            if (left_char == ' ') {
                left_index++;
                continue;
            }
            if (right_char == ' ') {
                right_index--;
                continue;
            }
        }

        // If case_insensitive is true, then the characters are converted to lowercase before being compared.
        if (case_insensitive) {
            left_char = tolower(left_char);
            right_char = tolower(right_char);
        }

        // If the left and right character do not match, then the sequence cannot be a palindrome.
        // Therefore, the function can return immediately.
        if (left_char != right_char) return false;

        // Otherwise, the function continues to the next pair of characters.
        left_index++;
        right_index--;
    }

    // If all characters in the checked range of the sequence match from left to right,
    // then the sequence is identified as a palindrome.
    return true;
}

/**
 * @brief This is a static helper function for the function is_palindrome.
 *        It prints the result of a check for a palindrome on a sequence of characters.
 * @details If a palindrome is detected in the sequence "... is a palindrome" is written to the output file,
 *          "... is not a palindrome" is written otherwise.
 * @param output The output file for printing if a line of the input file is a palindrome or not, can be stdout. Cannot be NULL.
 * @param sequence The char array which contains the characters to write. Cannot be NULL.
 * @param size The amount of characters to write in the sequence. Has to be smaller or equal to the total size of the sequence parameter.
 * @param is_palindrome A boolean value, true if the sequence is said to be a palindrome, false otherwise.
 * @return The flag FIND_PALINDROMES_WRITE_ERROR if an error occurred while trying to write to the output file, 0 otherwise.
 */
static int print_palindrome(FILE *output, const unsigned char *sequence, unsigned int size, bool is_palindrome) {
    // Print each character to the output file.
    for (unsigned int i = 0; i < size; i++) {
        if (fprintf(output, "%c", sequence[i]) < 0) return FIND_PALINDROMES_WRITE_ERROR;
    }

    // Print if the sequence is said to be a palindrome or not.
    if (is_palindrome) {
        if (fprintf(output, " is a palindrome\n") < 0) return FIND_PALINDROMES_WRITE_ERROR;
    } else {
        if (fprintf(output, " is not a palindrome\n") < 0) return FIND_PALINDROMES_WRITE_ERROR;
    }

    return 0;
}

/**
 * @brief A static constant, that defines the default size of memory to allocate
 *        for reading lines in the function find_palindromes.
 */
static const int FIND_PALINDROMES_DEFAULT_MALLOC_SIZE = 100;

/**
 * @brief A static constant, that defines the size of memory to add
 *        when running out of memory while reading a line inside the function find_palindromes.
 */
static const int FIND_PALINDROMES_MALLOC_INCREMENT = 100;

/**
 * @brief The function finds palindromes by copying each lines to dynamically allocated memory
 *        and then passing the sequence to the function is_palindrome.
 * @details The function starts with allocated memory of the size set by the static constant FIND_PALINDROMES_DEFAULT_MALLOC_SIZE.
 *          If the function runs out of allocated memory, it reallocates by adding the value from the
 *          static constant FIND_PALINDROMES_MALLOC_INCREMENT. If not enough memory can be allocated,
 *          the flag FIND_PALINDROMES_OUT_OF_MEMORY_ERROR is returned.
 */
int find_palindromes(FILE *input, FILE *output, bool case_insensitive, bool ignore_whitespace) {
    // Initially allocates the dynamic memory.
    unsigned int malloc_size = FIND_PALINDROMES_DEFAULT_MALLOC_SIZE;
    unsigned char *sequence = malloc(malloc_size);

    // Returns if memory could not be allocated.
    if (sequence == NULL) return FIND_PALINDROMES_OUT_OF_MEMORY_ERROR;

    unsigned int char_count = 0;

    // The loop repeats until the end of file is reached
    // or returns early if not enough memory could be allocated for reading a line
    while (true) {
        int read_status = fgetc(input);
        unsigned char c = read_status;

        // Check if the read characters are newlines or an end of file.
        if ((c == '\n') || (c == '\r') || (read_status == EOF)) {
            // Empty sequences are discarded.
            if (char_count > 0) {
                // Checks for the existence of a palindrome and prints the result to the output file.
                bool is = is_palindrome(sequence, char_count, case_insensitive, ignore_whitespace);
                int write_status = print_palindrome(output, sequence, char_count, is);

                // Returns early if an error occurred while writing to the output file.
                if (write_status != 0) {
                    free(sequence);
                    return write_status;
                }
            }
            char_count = 0;
        } else {
            // Otherwise, the line continues and the character is added to the existing sequence.
            char_count++;

            // If the character count reaches the limit of the dynamic memory, its size has to be increased.
            if (char_count == malloc_size) {
                // Reallocate the memory by a given size.
                malloc_size += FIND_PALINDROMES_MALLOC_INCREMENT;
                unsigned char *new_sequence = realloc(sequence, malloc_size);

                // If the memory could not be reallocated, the function returns early with an error code
                if (new_sequence == NULL) {
                    free(sequence);
                    return FIND_PALINDROMES_OUT_OF_MEMORY_ERROR;
                } else {
                    // In case of success the loop can continue.
                    sequence = new_sequence;
                }
            }
            // The character is added to the sequence.
            sequence[char_count - 1] = c;
        }

        // If the end of file is reached, the loop breaks.
        if (read_status == EOF) break;
    }

    // Free the dynamically allocated memory and signal success by returning 0.
    free(sequence);
    return 0;
}
