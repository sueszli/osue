/**
 * @author Maximilian Maresch
 */ 

#include <stdlib.h>
#include "util.h"

int hexStrToInt(char *str) {
    char *endptr;
    int decimal = (int)strtol(str, &endptr, 16);

    if (*endptr != '\0' && *endptr != '\n') {
        return -1;
    }

    return decimal;
}

void freeAll2(char* str1, char* str2) {
    free(str1);
    free(str2);
}

void freeAll3(char* str1, char* str2, char* str3) {
    free(str1);
    free(str2);
    free(str3);
}

void freeAll4(char* str1, char* str2, char* str3, char* str4) {
    free(str1);
    free(str2);
    free(str3);
    free(str4);
}