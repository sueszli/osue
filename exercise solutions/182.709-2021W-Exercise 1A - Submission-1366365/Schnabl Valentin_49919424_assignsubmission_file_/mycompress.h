/**
* @file mycompress.h
* @author Valentin Schnabl, 11848108
* @date 02.11.2021
* @brief this header includes all libraries and function headers. Also it defines the global variable PROGRAM_NAME
**/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#define PROGRAM_NAME "mycompress"

/**
* @brief this funtction holds the main functionality of the program. It reads in every character in a file and counts them. If the next character is equal to the last, count is beeing raised. If the next character
* is different, it prints the character and the counter into the output file. The loops body is repeated once after the loop for the last character. Also, there are statistics. The total amount of characters, the
* actual printed characters and the compression rate (written/total) is printed in stderr. 
* @param in is the input file. It can be either a temp file from stdin or a normal file.
* @param out is the output file. It can be either a normal file or stdout.
**/
void myCompress(FILE* in, FILE* out);