/**
 * Fileout module
 * @file fileout.h
 * @author Marvin Flandorfer, 52004069
 * @date 28.10.2021
 * 
 * @brief Output module.
 * This module is used to write content to a specific file.
 * 
 * @details This module uses standard I/O library functions to assert portability.
 * */

#ifndef FILEOUT_H
#define FILEOUT_H

/**
 * Function for writting into a file
 * @brief Writes a String into a file.
 * @details Using standard I/O library functions, this function writes a String into the given file (via filepath).
 * The function covers all possible I/O errors and writes error messages accordingly.
 * 
 * @param filepath Filepath to the output file.
 * @param str String that will be written to the output file.
 * @return On success returns 0. On failure (due to an error) returns -1.
 * */
int write_into_file(char filepath[], char str[]);

#endif