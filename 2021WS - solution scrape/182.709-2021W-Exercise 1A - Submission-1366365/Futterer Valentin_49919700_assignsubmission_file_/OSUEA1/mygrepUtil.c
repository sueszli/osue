/**
 * @file mygrepUtil.c
 * @author Valentin Futterer 11904654
 * @date 01.11.2021
 * @brief Provides methods for mygrep.c to use.
 * @details The error and usage function are provided by this file. It also provides the core functionality for mygrep.c.
 * The mygrep method reads lines from the input and uses search_str to search the line for a keyword. If one is found,
 * it is printed to the output file.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>


/**
 * @brief Exits with error.
 * @details Prints a message to stderr and prints the error code from errno, exits with error.
 * @param prog_name Programme name to print in error message.
 * @param msg The error message to print.
 * @return Returns nothing.
*/
void handle_error(const char *prog_name, char *msg) {
    fprintf(stderr, "[%s] ERROR: %s: %s\n",prog_name, msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief Displays usage message.
 * @details Prints a message to stderr and shows the correct way to parse arguments to the program, exits with error.
 * @param prog_name Programme name to print in error message.
 * @return Returns nothing.
*/
void usage(const char *prog_name) {
    fprintf(stderr,"Usage: %s [-i] [-o outfile] keyword [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief Like strstr with case insensitive option.
 * @details Calls strstr with needle and haystack and returns the result. If case_insensitive is true,
 * it first copies the two input strings and makes them both uppercase.
 * @param haystack The line to search.
 * @param needle The needle to be searched for.
 * @param case_insensitive 1 if the search should be caseinsensitive.
 * @return Returns a pointer to the first occurence of needle in haystack, if the needle is not found NULL.
 */
static char* search_str (char *haystack, char *needle, int case_insensitive) {
    // if case_insensitive is 1
    if (case_insensitive) {
        size_t haystack_len = strlen(haystack);
        size_t needle_len = strlen(needle);
        // generate uppercase copy of haystack
        char *cpy_haystack = malloc((haystack_len + 1) * sizeof(char));
        strcpy(cpy_haystack, haystack);
        while (*cpy_haystack) {
            *cpy_haystack = toupper((unsigned char) *cpy_haystack);
            cpy_haystack++;
        }
        cpy_haystack = cpy_haystack - haystack_len;
        // generate uppercse copy of needle
        char *cpy_needle = malloc((needle_len + 1) * sizeof(char));
        strcpy(cpy_needle, needle);
        while (*cpy_needle) {
            *cpy_needle = toupper((unsigned char) *cpy_needle);
            cpy_needle++;
        }
        cpy_needle = cpy_needle - needle_len;
        char* result = strstr(cpy_haystack, cpy_needle);
        free(cpy_haystack);
        free(cpy_needle);
        return result;
    } else {
        return strstr(haystack, needle);
    }
    
}

/**
 * @brief Reads lines and calls search_str for each line.
 * @details Reads lines from input using getline(). Calls search_str for each line.
 * @param output The File to write the result to.
 * @param input The File to read the lines from.
 * @param case_insensitive 1 if the search should be case insensitive.
 * @param keyword The keyword to be searched for in the read lines.
 * @return Returns a pointer to the first occurence of needle in haystack, if the needle is not found NULL.
 */
char* mygrep(FILE *output, FILE *input, int case_insensitive, char *keyword) {
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char* str_search_return;

    while ((nread = getline(&line, &len, input)) != -1) {
        str_search_return = search_str(line, keyword, case_insensitive);
        if (str_search_return != NULL) {
            if (fprintf(output, "%s", line) < 0) {
                free(line);
                return "Writing to output file failed";
            }
        }
    }
    free(line);
    return NULL;
}
