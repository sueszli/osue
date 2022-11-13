/**
 * @file fileActions.h
 * @author Simon Josef Kreuzpointner 12021358
 * @date 28 October 2021
 *
 * @brief the file actions module
 * 
 * @details This module provides a few functions to open or close files, 
 * aswell as reading from multiple files or writing to a single file.
 */

#ifndef FILEACTIONS_H
#define FILEACTIONS_H

#include <stdio.h>

/**
 * @brief Closes the given file.
 * 
 * @details This function closes the given file using fclose(3). If a
 * If an error ocurres while trying to close a file the program outputs an error message via
 * printError() and returns -1.
 * 
 * @param file A file pointer to the file that should be closed.
 * @return Returns 0 upon success or if the file equals null and -1 on failure.
 */
int closeFile(FILE *file);

/**
 * @brief Closes all used files
 *
 * @details This function calles int closeFile(FILE* file) on every file in the given array.
 * If an error ocurres while closing the files the program outputs an error message via
 * printError() and returns -1.
 *
 * @param files An Array of previously opened files.
 * @param fileCount The number of the files given.
 * @return Returns 0 upon success and -1 on failure.
 */
int closeFiles(FILE *files[], int fileCount);

/**
 * @brief Opens the given file
 * 
 * @details This function opens the given file with the specified mode.
 * If an error ocurres while trying to open a file the program outputs an error message via
 * printError() and returns -1.
 * 
 * @param file Empty file pointer, that gets filled when the file is opened successfully.
 * @param fileName Name of the file, that should be opened.
 * @param mode The mode in which the file should be opened. Corresponds to the mode in fopen(3)
 * @return Returns 0 upon success and -1 on failure.
 */
int openFile(FILE **file, char *fileName, const char *mode);

/**
 * @brief Opens all the given files
 * 
 * @details This function calls openFile(FILE** file, char* fileName, const char* mode) on every file in the given file array.
 * If an error ocurres while opening the files the program outputs an error message via
 * printError() and returns -1.
 * 
 * @param files Array of empty file pointers, that get filled after a successfull operation.
 * @param fileNames Array of the names of the files that should be opened.
 * @param mode The mode in which every file should be opened.
 * @param fileCount The amount of files given.
 * @return Returns 0 upon success and -1 on failure.
 */
int openFiles(FILE **files[], char **fileNames, const char *mode, int fileCount);

/**
 * @brief Gets the content of every given file and concatenates it.
 * 
 * @details This function reads everything from every file given. The contents are concatinated into the string dest.
 * The files that are passed have to be opened beforehand.
 * If an error ocurres while trying to get the files content the program outputs an error message via
 * printError() and returns -1.
 * It should be noted, that the functionality is only guaranteed for ASCII characters.
 * 
 * @param files An array of already opened file pointers.
 * @param fileCount The amount of files given.
 * @param dest A string which will be filled after a successfull operation.
 * @return Returns 0 upon success and -1 on failure.
 */
int getFilesContent(FILE *files[], int fileCount, char **dest);

/**
 * @brief Writes the content to a given file.
 * 
 * @details This function writes the given file and closes it afterwards.
 * if an error ocurres while trying to write to a file the program outputs an error message via
 * printError() and returns -1.
 * 
 * @param file Already opened file pointer.
 * @param fileName The name of the file.
 * @param content The string that should be written to the file.
 * @return Returns 0 upon success and -1 on failure.
 */
int writeToFile(FILE *file, char *fileName, char *content);

#endif