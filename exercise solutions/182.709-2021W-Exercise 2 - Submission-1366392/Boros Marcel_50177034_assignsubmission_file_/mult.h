/**
 * @file mult.h
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 06.12.2021
 *  
 * @brief Header file for functions needed for multiplication in mult.c.
 *
 */



#ifndef mult_H
#define mult_H

#include <stdarg.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>


/**
 * @brief convert a hexadecimal number to a number in decimal system
 * @details The function doesn't check on correct input values. It converts the hexadecimal
 * number with the help of the ascii values and the Math.pow-function.
 * @param Hexadecimal input and the starting exponent.
 */
int hexToNumber(char* hexNumber, int exponent);


/**
 * @brief multiply given hexadecimal values
 * @details The functions multiplies the two hex-values given in firstLine and secondLine with the help
 * of other functions and prints the result to stdout. If further recursion is needed in order to calculate
 * the result the partial results will be written to solutionArray and printed to stdout after a separate
 * calculation.
 * @param First and second hex-number, argument vector, partial solutions in an array
 */
void multiply(char* firstLine, char* secondLine, char** argv, char** solutionArray);


/**
 * @brief divide lines in the middle into two parts
 * @details The functions divides the two hexadecimal numbers each into two halves with the help
 * of loops.
 * @param Two hex values (fL, sL) and first/second half of each
 */
void divideLines(char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2);


/**
 * @brief release memory function
 * @details The function simply uses free to release memory of the given parameters.
 * @param Two hex values (fL, sL) and first/second half of each
 */
void freeMemory(char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2);


/**
 * @brief parallelize (pipes, fork, etc.)
 * @details The function creates 8 pipes and 4 children (so 2 pipes for each child) in order to
 * parallelize the computation. Stdin and stdout of each child will be redirected into the pipes.
 * After the calculation the solution is read from the pipes and stored into the solutionArray.
 * @param Two hex values (fL, sL) and first/second half of each, argument vector, partial solutions in an array
 */
void parallelize(char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2, char** argv, char** solutionArray);


/**
 * @brief close pipes with variable parameters
 * @details The function closes pipes with variable parameter numbers. It doesnt check whether the given
 * parameters are pipe-ends or "pipe-arrays", so it is assumed that the the arguments
 * can be closed. Furthermore a counter is given as a parameter in order to know the end of a loop and
 * a type to differentiate between pipe-end and pipe-array.
 * @param type to determine pipe or pipe-end, argument vector, a counter variable for loop ending and one or more pipes.
 */
void closePipes(int type, char** argv, int counter, ...);


/**
 * @brief pipe opening/error handling
 * @details The function opens the pipes from c11 to c42 with possible error handling
 * @param 8 pipes from c11 to c42, argument vector and the two hex values (fL, sL) with first/second half of each
 */
void open_handle_pipes(int* c11, int* c12, int* c21, int* c22, int* c31, int* c32, int* c41, int* c42, char** argv,
                       char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2);


/**
 * @brief calculate solution
 * @details The function calculates the solutoin of the 4 given parts in solutionArray. Firstly those values 
 * are interpreted as hex numbers with strtol and afterwards the final solution is calculated with
 * the formula given in the assignment.
 * @param partial solutions in an array, size of the full hexadecimal number
 */
unsigned int calculateSolution(char** solutionArray, int n);

#endif