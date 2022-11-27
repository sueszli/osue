/**
 * @file mydiff.c
 * @author Michael Zischg <Matriculation Number: 12024010>
 * @date 02.11.2021 
 * 
 * @brief implementation of mydiff Algorithm
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "mydiff.h"

void compareFiles(FILE *ptr1, FILE *ptr2, const int caseSensitive, FILE *outstream) {
    int pos = 0, line = 1, count = 0;
    char ch1, ch2;

    //avoiding to use strcmp, because it would be less intuitive, to compare different strings: instead of every character individually to keep counter with the right values
    do {
        ch1 = fgetc(ptr1);
        ch2 = fgetc(ptr2);
        pos++;

        if(ch1 == '\n' || ch2 == '\n') {
            while(ch1 != '\n') {
                ch1 = fgetc(ptr1);
                if(ch1 == EOF) break;
            }
            while(ch2 != '\n') {
                ch2 = fgetc(ptr2);
                if(ch2 == EOF) break;
            }
            if(ch1 == EOF || ch2 == EOF) break;
            if(count != 0) {
            	fprintf(outstream,"Line: %i, characters: %i\n", line, count);
	        }
            line++;
            pos = 0;
            count = 0;
        }

        if(caseSensitive == 1 && ch1 != ch2) count++;
        else if(tolower(ch1) != tolower(ch2)) count++;
    } while (ch1 != EOF && ch2 != EOF);
}

