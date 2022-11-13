/**
 * @author Maximilian Maresch
 */ 

#include <stdbool.h>
#include <ctype.h>
#include "palindrom_algorithm.h"

char* ispalindrom(char* input, int size, int opt_i, int opt_s) {
    int start = 0;
    int end = size - 2;
    bool ispalindrom = true;

    char inputProcessed[end];

    if (opt_i > 0) {
        for (int i = 0; i <= end; i++) {
            inputProcessed[i] = toupper(input[i]);
        }
    } else {
        for (int i = 0; i <= end; i++) {
            inputProcessed[i] = input[i];
        }
    }

    while (end >= start) {
        if (opt_s > 0) {
            if (inputProcessed[start] == ' ') {
                start++;
                continue;
            } 

            if (inputProcessed[end] == ' ') {
                end--;
                continue;
            } 
        }

        if (inputProcessed[end] == inputProcessed[start]) {
            start++;
            end--;
        } else {
            ispalindrom = false;
            break;
        }
    }

    if (ispalindrom) {
        return " is a palindrom\n";
    } else {
        return " is not a palindrom\n";
    }
}