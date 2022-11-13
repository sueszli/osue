/**
 * @file compare.h
 * @author Yuhang Jiang <e12019854@student.tuwien.ac.at>
 * @date 27.10.2021
 *
 * @brief Compares lines in files and writes them to an output file
 */
#ifndef AUFGABE1A_COMPARE_H
#define AUFGABE1A_COMPARE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * @brief Compares each line of the file with the keyword. If a part of the line matches the keyword, then the line
 * is written to the outfile.
 * @param file File to be compared.
 * @param outfile Results are written to this file.
 * @param keyword Keyword to be compared.
 * @param case_insensitive 1 enables case-insensitive comparison (the keyword will also be converted to lower case)
 */
void compare_lines(FILE *file, FILE *outfile, char *keyword, int case_insensitive);

#endif //AUFGABE1A_COMPARE_H
