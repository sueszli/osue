/**
 * @file io_driver.h
 * @author Lukas Thurner MatrNr.: 01427205 <lukas.thurner@tuwien.ac.at>
 * @date 2.11.2021
 *
 * @brief Provides functions for read and write files
 * 
 * The io module contains functions for reading and writing files
 */  

#ifndef IO_HANDLER_FILE
#define IO_HANDLER_FILE

#include <stdio.h>
#include <stdbool.h>


/**
 * Read the input files, search for the keyword and writes to the output stream (file or stdout).
 * @brief This functions opens all inputfiles, search for the keyword and write the result to the output file or the stdout.
 * @param filename Names of all input files
 * @param keyword The keyword to be searched for
 * @param output_filename The name of the output file
 * @param dif_up_low Flag for case sensitive or case insensitive (true = case sensitive)
 */ 
void searchInputFile(char* filename, char* keyword, char* output_filename, bool dif_up_low);

/**
 * Read the stdin, search for the keyword and writes to the stdout
 * @brief This functions listen the stdin, search for the keyword and write the result to the stdout.
 * @param keyword The keyword to be searched for
 * @param dif_up_low Flag for case sensitive or case insensitive (true = case sensitive)
 */
void searchStdInput(char* keyword, bool dif_up_low);

#endif
