/**     
*@file mycompress.c
*@author Fidel Cem Sarikaya <e11941488@student.tuwien.ac.at>
*@date 07.11.2021
*
*@brief Implementation of the mycompress module.
*
*The mycompress module that consists of mycompress() function is defined.  
**/

#include "mycompress.h"
#include <string.h>
#include <stdio.h>

/**
 * Main program function that fulfills the run-length compression.
 * @brief This function takes a character array and as it reads it index-by-index, the run-length coded compressed form of it gets written to the character array 'out'.
 * @details After initialization of 'out', a for-loop reads 'in' character by character and 'rep'´counts the number of repeatative characters, which is set back to 1 when 
 * a new character is read. Loop starts with 'out' that already has the first character from 'in' and either counts the 'rep' or insterts it in 'out' by using 'sprintf()'. 
 * After such an insertion, next index in 'out' gets the adjacent different character in 'in'. There are 2 if-cases that represent 2 possible states while reading the last 
 * character of 'in'. During either of them, loop is terminated by inserting '/0' at the end of 'out'.
 * @param in Constant character array. 'ind' indicates the next index in 'out' to receive a character from 'in'.
 * @param out Character array whose allocated memory is larger than twice the size of parameter 'in'
*/
void *compressor(const char *in, char *out) {
    int const LEN = strlen(in);

    out[0] = in[0];
    out[1] = '\0';
    int rep = 1;

    int ind = 0;
    for (int i = 1; i < LEN; i++) {
        if (in[i] == (in[i-1])) {
            rep++;
            if (i == (LEN - 1)) {
                ind = ind + sprintf(out + strlen(out), "%i", rep) + 1;
                out[ind] = '\0';
            }
        }
        else {
            ind = ind + sprintf(out + strlen(out), "%i", rep) + 1;
            out[ind] = in[i];
            out[ind+1] = '\0';
            rep = 1;
            if (i == (LEN - 1)) {
                out[ind+1] = (rep +'0');
                out[ind+2] = '\0';
            }
        }
    }
    return 0;
}
