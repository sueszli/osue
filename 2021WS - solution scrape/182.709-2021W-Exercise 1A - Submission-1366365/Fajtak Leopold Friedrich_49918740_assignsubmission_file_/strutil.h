/**
 * @file strutil.h
 * @author Leopold Fajtak (e01525033@student.tuwien.ac.at)
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef STRUTIL_H
#define STRUTIL_H

#include <stdlib.h>
#include <stdio.h>

/**
 * @brief counts the number of occurrences of c in str
 *
 * @param str!=NULL
 * @param c
 * @return number of occurrences of c in str. On error, the function returns a
 * negative number
 */
int strchrocc(char* str, char c);

#endif
