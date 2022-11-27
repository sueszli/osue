/**
 *@author Gregor Käfer 01326186
 *@brief Write a C-program ispalindrom, which reads and check if a line of text a palindrom or not.
 *@date ‎Saturday, ‎13 November ‎2021
 */
 

 //standard library
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

/* buffer size */
const int BUFFER = 512;

/** reverse a text **/
void reverseText(char *text);

/** function that check if a text is a palindrom or not **/
void isPalindrom(char *text, int iFlag, int sFlag);

/** function to read text from stdin and print it to stdout or a file **/
void readFromStdin(int iFlag, int sFlag, int oFlag, char *outputFile);

/** function to read a file with the given file name and print it to stdout or a file **/
void readFromFile(int iFlag, int sFlag, int oFlag, char* outputFile, char* inputFile, int begin);

/** error message **/
void usage(void);