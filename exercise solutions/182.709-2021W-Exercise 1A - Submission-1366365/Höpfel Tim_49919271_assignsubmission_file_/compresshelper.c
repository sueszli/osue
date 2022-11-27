/**
 * @file compresshelper.c
 * @author Tim Höpfel Matr.Nr.: 01207099 <e01207099@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Provides the compress-function used for all compressing in mycompress.
 *
 * This program compresses text with a simple algorithm. The input is compressed by substituting subsequent identical characters by only one occurence of the
 * character followed by the number of characters. For example, if you encounter the sequence aaa, then it is replaced by a3.
 **/

#include "compresshelper.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/**
 * Compress a line to output.
 * @brief This function takes a text, compresses it to an outputstring.
 * @details The program returns
 * @param text This string is read.
 * @param output This string is written to. It contains the compressed text.
 * @param inputsize This int is written to. It continas the size of the parameter text.
 * @return Returns the size of the compressed text. Returns 0 if the parameters are invalid.
 */
int compress(const char *text, char *output, int *inputsize ) {
    //validate input
    if(!text || !output ) {
        return 0;
    }
    //determin size of space needed
    // double the space of the text is needed to have enough for all cases
    int outputsize = sizeof(text)*2+1;
    char *p;
    p = malloc(outputsize);
    if(p == NULL) {
        exit(EXIT_FAILURE);
    }
    int p_counter = 0; /**< This value is beeing returned. */

    int tempCounter = 1; /**< This value counts the number of ident chars in a row. */
    const char* current_char;
    int text_lenght = strlen(text);

    *inputsize = text_lenght;
    //point to the start of the text, iterates through text
    current_char = text;
    bool first = true;
    for(current_char = text; *current_char; current_char++) {
        if (first) {
            first = false;
            //copy first charakter to p (=result)
            strncpy(p, current_char, 1);
        }else if(*p == *current_char) {
            //increase counter of same characters
            tempCounter++;
        }
        // digit after char is limited to 9 to increase readability
        if(*p != *current_char || tempCounter >= 10) {
            //move p to next (empty) slot
            p++; p_counter++;
            //copy char-counter into it e.g.: 2 ; write 9 if tempCounter is 10. 1 will be added
            if(tempCounter >= 10) {
                tempCounter = 9;
            }
            sprintf(p, "%d", tempCounter);
            //*p = (char)tempCounter;
            //move p to next empty place
            p++; p_counter++;
            //write new character into it, p now points at the charakter
            strncpy(p, current_char, 1);
            //adjust temp-vars to new character
            if(tempCounter >= 9) {
                tempCounter = 1;
            } else {
            tempCounter = 1;
            }
        }
    }
    p -= p_counter;
    strncpy(output, p, outputsize);

    free(p);

    return p_counter;
}
