/**
 * @file comp.h
 * @author Lukas Fink 11911069
 * @date 04.11.2021
 *  
 * @brief Compression Module
 *
 * The compress module. It contains a compression function and a size function for char arrays.
 **/

#ifndef COMP_H
#define COMP_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

char * compress(char* input, int end, char *programName);
int sizeOfCharArray(char *arr);

#endif