/**
 * @file util.h
 * @author Markus Paoli 01417212
 * @date 2021/10/31
 *
 * @brief The utilities module.
 *
 * @details This module contains utility functions for use by other modules.
 *          It includes a function for reading a line from a stream and one for replacing
 *          all tabs in a line with spaces up to a specified column width.
 */

/**
 * The read_line function
 * @brief Reads a line of arbitrary length from the stream in.
 *
 * @details This function reads a line of arbitrary length up to '\n' or EOF whichever is encountered first,
 *          '\0' terminates it and returns a pointer to it.
 * @param in The input stream to read from. Reading it must be enabled.
 * @return Returns a pointer to the string read or NULL in case of an error.
 */
char* read_line(FILE *in);

/**
 * The replace_tabs function
 * @brief Replaces all tabs in a line with spaces.
 *
 * @details This function takes a pointer to a string and replaces every
 *          occurrence of '\t' replaces by a number of spaces so that the
 *          next character is at position p = tabstop * ((x / tabstop) + 1).
 * @param str The pointer to the string.
 * @param tabstop The column width.
 * @return Returns the pointer to the string or NULL in case of an error.
 */
char* replace_tabs(char *str, long tabsize);

