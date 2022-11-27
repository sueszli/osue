/**
 * @file readIn.h
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 06.12.2021
 *  
 * @brief Header file for functions needed for reading input in readIn.c.
 *
 */

#ifndef readIn_H
#define readIn_H


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#define BYTES 100


/**
 * @brief check if the input values have the same length and are hex numbers (with ascii table)
 * @details The function compares the input characters with appropriate ascii values to determine
 * whether the input is a correct number or not.
 * @param Two hexadecimal input numbers, argument vector
 */
void checkInputValues(char* firstLine, char* secondLine, char** argv);


/**
 * @brief read in first and second line
 * @details The functions reads in the first number until a newline is reached. Afterwards
 * the second number is read until EOF. It is assumed, that the input only consists of two
 * lines.
 * @param Two hexadecimal input numbers, argument vector
 */
void readLines(char* firstLine, char* secondLine, char** argv);

#endif