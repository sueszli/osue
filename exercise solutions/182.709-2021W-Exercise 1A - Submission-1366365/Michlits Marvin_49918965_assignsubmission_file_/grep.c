/**
 * @file grep.c
 * @author Marvin Michlits 11731205
 * @date 04.11.2021
 * @brief grep function module
 * 
 * grep.c implements some functions which are used in main.c to implement a variation of grep.
 * The implemented functions include a function to open an output file, one to write to that output file and one to write to stdout.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "grep.h"

/**
 * openOutput function
 * @brief This function opens an output file.
 * @details The file which is opened or created is the file which the function writeToOutput is going to write to later on.
 * because of that w+ access is needed. (read needed in order to open it after the program terminates)
 * @param filename the name which the outputfile should have (specified in argv)
 * @return Returns output which is a FILE pointer which points to the output file 
 */
FILE *openOutput(char *filename, char *option){
    FILE *output;
    if ((output = fopen(filename, option)) == NULL){
        fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", "grep.c", strerror(errno)); /*grep.c instead of argv[0] because I don't know how to access argv[0] here*/
        exit(EXIT_FAILURE);
    }
    return output;
}

/**
 * writeToOutput function
 * @brief This function writes to an output file.
 * @details The function checks if grepString is contained in line with the predefined function strstr. If it is contained the unchanged version
 * of the line is written to the output file using fputs and a pointer to that file is returned. If not a pointer to the unchanged output file is returned.
 * @param originalLine This is the unchanged line which is originally read from either an input file or stdin. It needs to be a parameter because 
 * in case of -i (not case sensitive) the line parameter is changed to capital letters only. In order to write the original version of 
 * the line to the output originalLine is used.  
 * @param line The line which is to be checked (if it contains grepString). This line is read from either an input file or stdin.
 * @param grepString The specified (in argv) string which is searched for in the input file or stdin. 
 * @param output A pointer to the output file this function should write to 
 * @return Returns output which is a FILE pointer which points to the output file which was potentially just written to.
 */
FILE *writeToOutput(char *originalLine, char *line, char *grepString, FILE *output) {
    if (output){
        if (strstr (line, grepString) != NULL){
            if(fwrite(originalLine, sizeof(char), strlen(originalLine), output) != strlen(originalLine)){
                fprintf(stderr, "[%s] ERROR: fwrite failed: %s\n", "grep.c", strerror(errno)); /*grep.c instead of argv[0] because I don't know how to access argv[0] here*/
                exit(EXIT_FAILURE);
            }
        }
    }
    else {
        fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", "grep.c", strerror(errno)); /*grep.c instead of argv[0] because I don't know how to access argv[0] here*/
        exit(EXIT_FAILURE);
    }
    return output;
}

/**
 * writeToStd function
 * @brief This function prints to stdout.
 * @details The function checks if grepString is contained in line with the predefined function strstr. If it is contained the unchanged version
 * of the line is written to stdout using fprintf and 0 is returned. If not nothing is printed and 0 is returned.
 * @param originalLine This is the unchanged line which is originally read from either an input file or stdin. It needs to be a parameter because 
 * in case of -i (not case sensitive) the line parameter is changed to capital letters only. In order to print the original version of 
 * the line to stdout originalLine is used.  
 * @param line The line which is to be checked (if it contains grepString). This line is read from either an input file or stdin.
 * @param grepString The specified (in argv) string which is searched for in the input file or stdin. 
 * @return Returns 0 if sucessfull. Returns EXIT_FAILURE if not.
 */
int writeToStd(char *originalLine, char *line, char *grepString) {
    char *originalLineMaybe;
        if ((originalLineMaybe = strstr (line, grepString)) != NULL){
            if (originalLineMaybe == originalLine) {
                fprintf(stderr, "[%s] ERROR: empty grepString - please enter non-empty string: %s\n", "grep.c", strerror(errno)); /* grep.c instead of argv[0] because I don't know how to access argv[0] here */
                exit(EXIT_FAILURE);
            }
            printf("%s", originalLine);
        }
    return 0;
}