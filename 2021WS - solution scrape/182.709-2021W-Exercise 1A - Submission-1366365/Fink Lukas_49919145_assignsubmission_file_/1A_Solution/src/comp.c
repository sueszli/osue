/**
 * @file comp.c
 * @author Lukas Fink 11911069
 * @date 04.11.2021
 *
 * @brief Compression module implementation.
 * 
 * @details This program is responsible for compressing data and for evaluating the length of an char array.
 * 
 **/

#include "comp.h"

/**
 * compress function
 * @brief This function compresses a char-array into a new char-array and returns it.
 * @details Compresses e.g. aaabbc\\n to a3b2c1\\n1. If not successful, it exits with EXIT_FAILURE.
 * @param input the input char array.
 * @param end the index of the last element '\0'.
 * @param programName the name of the program calling (for error-messages)
 * @return Returns a new compressed char array.
 **/
char * compress(char* input, int end, char *programName) {
    
    char current;
    int nIn = 0;
    int nOut = 0;
    int size = end;
    char *outputBuf = malloc(sizeof(char) * size);
    if (outputBuf == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        return NULL;
    }

    while (nIn < end) {
        current = input[nIn];
        if (nOut >= size-2) {
            size += 256;
            outputBuf = realloc(outputBuf, size * sizeof(char));
            if (outputBuf == NULL) {
                fprintf(stderr, "%s: out of memory!\n", programName);
                free(outputBuf);
                return NULL;
            }
        }
        nIn++;
        int amount = 1;
        char local;
        while ((local = input[nIn]) == current) {
            amount++;
            nIn++;
        }
        outputBuf[nOut] = current;
        nOut++;

        int nSize = 256;
        char* number = malloc(sizeof(char) * nSize);
        if (number == NULL) {
            fprintf(stderr, "%s: out of memory!\n", programName);
            free(outputBuf);
            free(number);
            return NULL;
        }
        int success;
        while ((success = sprintf(number, "%d", amount)) < 0) {
            nSize += 256;
            number = realloc(number,sizeof(char) * nSize);
            if (number == NULL) {
                fprintf(stderr, "%s: out of memory!\n", programName);
                free(outputBuf);
                free(number);
                return NULL;
            }
        }
        
        int n = 0;
        while (number[n] != '\0') {
            if (nOut >= size-2) {
                size += 256;
                outputBuf = realloc(outputBuf, size * sizeof(char));
                if (outputBuf == NULL) {
                    fprintf(stderr, "%s: out of memory!\n", programName);
                    free(outputBuf);
                    free(number);
                    return NULL;
                }
            }
            outputBuf[nOut] = number[n];
            nOut++;
            n++;
        }
        free(number);

    }
    outputBuf[nOut] = '\0';

    return outputBuf;
}

/**
 * sizeOfCharArray function
 * @brief This function returns the size of an char array.
 * @details It counts the number of chars until (but not including) the '\0' char.
 * @param arr the input char array.
 * @return Returns the length of the array up to (but not including) the '\0' char.
 **/
int sizeOfCharArray(char *arr) {
    if (arr == NULL) {
        return 0;
    }
    int size = 0;
    int n = 0;
    while (arr[n] != '\0') {
        size++;
        n++;
    }
    return size;
}