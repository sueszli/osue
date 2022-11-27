/**
 * @file fileutil.h
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @brief Utilities for mygrep
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef FILEUTIL_H
#define FILEUTIL_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Opens the file located at path with specified mode and handles errors
 * 
 * @param path the location fo the file to open
 * @param mode the mode of file to open (like fopen)
 * @return FILE* 
 */
FILE* openFile(char* path, char* mode);

/**
 * @brief Closes file and handles errors
 * 
 * @param file file to close
 * @return 0 on success
 */
int closeFile(FILE* file);
#endif
