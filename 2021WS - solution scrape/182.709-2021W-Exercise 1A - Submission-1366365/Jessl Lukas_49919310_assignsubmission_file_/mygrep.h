/*
Module mygrep.h
Author: Lukas Jessl M-Nr: 01604985
created: 03.11.2021


*/

#ifndef MYGREP_H
#define MYGREP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>


//Buffersize, can be changed
//small buffersize, so we don't allocate unneeded storage
#define BUFFERSIZE 4

void readfromFiles (char* path, char* keyword, bool cs, char* output, char* file);
void readfromCommandoLine(char* path,char* keyword, bool cs, char* output);
bool iskey (char* input, char* keyword, bool cs);

#endif


