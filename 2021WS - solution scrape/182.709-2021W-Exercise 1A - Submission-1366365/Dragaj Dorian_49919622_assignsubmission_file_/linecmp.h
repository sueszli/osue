/**
 * @file linecmp.h
 * @author Dorian Dragaj <e11702371@student.tuwien.ac.at>
 * @date 03.11.2021
 *  
 * @brief Provides utility a function useful for the main function.
 *
 * The linecmp module. It contains a function to compare the equality of two lines.
 */

#ifndef LINECMP_H   /* Include guard */
#define LINECMP_H
#include <stdio.h>

/**
 * Compare two lines for equality.
 * @brief This function compares two lines for their equality and return the number of mismatching characters.
 * @details If one of the lines is shorter, both lines will only be compared untill the length of the
 * shorter line. This function ignores the newline character. Here we have also an option to include case insensitivity
 * @param line1 The first line to be compared.
 * @param line2 The second line to be compared.
 * @param opt_i Option whether program should include case insensitivity, if opt_i == 1, case insensitivity will be included, otherwise not.
 * @return Returns number of different characters.
 */
int line_cmp(char *line1, char *line2, int opt_i);

#endif
