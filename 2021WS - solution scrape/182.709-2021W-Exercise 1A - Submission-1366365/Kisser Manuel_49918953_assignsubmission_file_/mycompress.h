/**
 * @file mycompress.h
 * @author Manuel Kisser (12024009)
 * @brief mycompress compresses a text source (stdin/file) by counting character occurence.
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef MYCOMPRESS_H
#define MYCOMPRESS_H

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
int printMultiple(FILE *printTarget, int count, char *str);
int biggestInt(int numbers[], int size);
int get_int_len(int value);

#endif