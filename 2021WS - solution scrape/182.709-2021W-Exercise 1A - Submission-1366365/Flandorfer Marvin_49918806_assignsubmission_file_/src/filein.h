/**
 * Filein module
 * @file filein.h
 * @author Marvin Flandorfer, 52004069
 * @date 27.10.2021
 * 
 * @brief Input module.
 * Reads input from either stdin or (possibly) multiple files.
 * 
 * @details This module uses standard I/O library functions to assert portability.
 * Functionality can only be assured for ASCII characters.
 * For all other characters unprediceted behaviour will occur through subsequent function calls.
 * */

#ifndef FILEIN_H
#define FILEIN_H

/**
 * Function for reading from stdin
 * @brief All input from stdin will be read until the EOF symbol is reached.
 * @details This function is only a wrapper function. 
 * The subsequent function calls cover all possible errors and write error messages accordingly.
 * The returned pointer (on success) points to allocated memory and needs to be freed after usage.
 * 
 * @return On succes a char pointer to the read input will be returned. On failure (due to an error) a null-pointer will be returned.
 * */
char *read_content_from_stdin(void);

/**
 * Function for reading from a file
 * @brief All symbols from a file will be read until the EOF symbol is reached.
 * @details This function is a partial wrapper function.
 * I/O actions (like file opening/closin) will be handled in this function, everything else will be covered in subsequent function calls.
 * All possible errors are covered and error messages will be written accordingly.
 * The returned pointer (on success) points to allocated memory and needs to be freed after usage.
 * 
 * @param filepath Filepath to the input file.
 * @return On success a char pointer to the read input will be returned. On failure (due to an error) a null-pointer will be returned.
 * */
char *read_content_from_file(char filepath[]);

#endif