/**
 * @file header.h
 * @author Josef Taha <e11920555@student.tuwien.ac.at>
 * @date 03.11.2021
 *
 * @brief Provides functions to identify keywords in lines and for writing/reading lines.
 *
 * The header module. It contains function for identify keywords in lines and writing/reading lines.
 **/

#ifndef MYGREP_H__
#define MYGREP_H__

/**
 * Reads every line of all input files, checks if the line contains a given keyword and writes every line back in output file if satisfied
 * @brief This function reads through all lines of a given set of files (input parameter), checks whether this line contains
 * a given keyword (keyword parameter), accordingly writes this line in a File (output parameter). By setting i-flag to 1
 * lower/uppercase differences are ignored when checking if a line contains the keyword.
 * @details The function does not check if the i-flag parameter values are 0 or 1. If the given output paramter is not found
 * one with the same name will be created.
 * @param *input the file in which all lines are written.
 * @param **output the files from which all lines are read.
 * @param keyword string to check if contained in a line.
 * @param iflag ignores lower/uppercase differences of characters in lines if set to 1.
 * @param prog name of the program
 */
int io(FILE **input, FILE *output, char *keyword, int iflag, char *prog);


#endif /* MYGREP_H__ */