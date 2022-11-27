/**
 * @file mydiff.h
 * @author Michael Zischg <Matriculation Number: 12024010>
 * @date 02.11.2021 
 * 
 * @brief provides compareFiles function.
 **/


#include <stdio.h>

#ifndef MYDIFF_H__  /* ' prevent multiple inclusion'*/
#define MYDIFF_H__

/**
 * @brief Function that returns all lines that differ in the files 
 * together with the corresponding amount of different characters.
 * @details Both files are compared following the smaller principle:
 * characters that are past the length of characters of the line of 
 * the other file are not compared. Same goes for lines that are not 
 * present in the other file. If parameter caseSensitive is equal to 
 * 1, then characters are directly compared. Else the characters are 
 * compared without taking account of capital letters. 
 * @param output Char Buffer to which output is written to.
 * @param ptr1 Pointer to the first file.
 * @param ptr2 Pointer to the second file. 
 * @param caseSensitive 1 = comparison differs for example 'a' and 'A'; else = it does not
 */
void compareFiles(FILE *ptr1, FILE *ptr2, const int caseSensitive, FILE *outstream);

#endif