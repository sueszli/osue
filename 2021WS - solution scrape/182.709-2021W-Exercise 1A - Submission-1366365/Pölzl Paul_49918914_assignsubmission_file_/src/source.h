/**
 * @file source.h
 * @author Paul Pölzl (12022514)
 * @date 27.10.2021
 * 
 * @brief Provides specialized functions used in main.
 * 
 * The source module contains specialized functions needed in the main module.
 */

#ifndef SOURCE_H__      //prevent multiple inclusion
#define SOURCE_H__

#include <stdio.h>
#include <signal.h>

/** 
 * @brief Program name.
 */
extern char *myprog;

/**
 * @brief Signals the program to shut down.
 * @details Turns to 1 when SIGINT or SIGTERM are called.
 */
extern volatile sig_atomic_t quit;

/**
 * @brief This function associtates a stream with the file at path.
 * 
 * @details Includes error handling.
 * 
 * @param *path contains the path where the file should be opened.
 * @param *mode contains the I/O mode (e.G.: "w", "r", "a").
 * 
 * @return Returns a FILE pointer to the file with the given name.
 */
FILE *open(char *filename, char *options);

/**
 * @brief Every line from the input *in containing the keyword is written to the output *out.
 *
 * @details Includes error handling.
 * 
 * @param *in FILE pointer of the input input file or stream.
 * @param *out FILE pointer of the output file or stream.
 * @param *keyword The keyword used to filter the lines from input.
 * @param not_case_sensitive 0: Case sensitive; 1: Not case sensitve.
 */
void filter(FILE* in, FILE* out, char *keyword, int not_case_sensitive);

#endif /* SOURCE_H__ */