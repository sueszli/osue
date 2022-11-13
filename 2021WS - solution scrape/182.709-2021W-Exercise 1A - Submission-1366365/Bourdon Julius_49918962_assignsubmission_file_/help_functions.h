/**
 * @file help_functions.h
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief This module provides help functions the program 'mycompress'.
 *      
 **/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdbool.h>
#include <errno.h>

/**
 * @brief   This function compresses the input from 'file_to_read_from' and writes the compressed string into the intermediate buffer.
 *
 * @param buffer             The pointer to the provided intermediate buffer.  
 * @param buffer_pointer     The pointer to the current index in the provided intermediate buffer. 
 * @param file_to_read_from  The stream from which the input is read. 
 *
 * @return Returns the number of characters read.
 */
int compress_string(char *buffer, int* buffer_pointer, FILE* file_to_read_from);


/**
 * @brief  This function calculated the size of the provided file.
 *
 * @param file  The pointer to the file, of which the size is to be calculated.   
 *
 * @return Returns the size of the provided file.
 */
long size_of_file(FILE *file);