/**
 * myexpand.h
 * 
 * @author Luis Henrique Paoletti 11929823
 * @brief Header file for myexpand.c file
 * @date 12.11.2021
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/**
 * @details This function should print formated_message out into stderr, with a specification of
 * the program, to which the message applies to. At the end there's a call to the exit function of stdlib.h
 * @param formated_message A message, which should be formated according to printf of stdlib.h
 * @param program The program, which the message applies to
 * @return void
 */
void usage(const char *const formated_message, const char *const program);

/**
 * @details This function should print formated_message out into stderr, with a specification of
 * the program, to which the message applies to. At the end there's a call to the exit function of stdlib.h
 * @note There must be an extra %s at the end of formated_message, which takes in the value of
 * strerror(errno)
 * @param formated_message A message, which should be formated according to fprintf of stdlib.h
 * @param program The program, which the message applies to
 * @return void
 */
void error(const char *const formated_message, const char *const program);

/**
 * @details This function uses string to return a reformated char*, where each '\t' character is replaced
 * with ' ' characters until the next multiple of tab_size is reached
 * @param tab_size Size of the tabulators to be used
 * @param string A constant string pointer used as input
 * @note string will be altered as following: if there's at the end of the string a '\n' character,
 * it will be replaced by '\0'
 * @return char*
 */
char *tabulate(int tab_size, char *const string);

/**
 * @details This function reads each line of file_in as a single string and calls tabulate with it as input.
 * Each char* returned by a tabulate call gets appended with a '\n' character.
 * If there's a file_out (i.e. not NULL), then it is overwritten by the results of the tabulate calls;
 * else the results are printed into stdout
 * @param tab_size Size of the tabulators to be used
 * @param program A constant string pointer used as input
 * @param file_in Input file
 * @param file_out Output file (can be NULL to print to stdout)
 * @return void
 */
void tabulateFiles(int tab_size, const char *program, FILE *file_in, FILE *file_out);