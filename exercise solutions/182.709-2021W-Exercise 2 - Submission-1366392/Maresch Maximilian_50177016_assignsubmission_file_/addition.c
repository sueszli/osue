/**
 * @author Maximilian Maresch
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "addition.h"

int hexCharToInt(char character) {
    if (character >= 'A' && character <= 'F')
        return character - 'A' + 10;
    if (character >= 'a' && character <= 'f')
        return character - 'a' + 10;
    if (character >= '0' && character <= '9')
        return character - '0';
    return -1;
}

char* add(char* x, char* y) {

    size_t max;
    if (strlen(x) > strlen(y)) {
        max = strlen(x);
    } else {
        max = strlen(y);
    }

    int indX = strlen(x) - 1;
    int indY = strlen(y) - 1;

    int indCurrentResult = 0;

    int resultLength = 2 * max;
    char* result = malloc(sizeof(char) * (resultLength + 1));

    if (result == NULL) {
        return NULL;
    }

    for (int i = 0; i < resultLength; i++)  {
        result[i] = '0';
    }

    result[resultLength] = '\0';

    int carry = 0;
    for (int i = 0; i < max; i++) {
        char cX = '0';
        char cY = '0';

        if (indX - i >= 0) {
            cX = x[indX - i];
        }

        if (indY - i >= 0) {
            cY = y[indY - i];
        }

        int iX = hexCharToInt(cX);    
        int iY = hexCharToInt(cY);

        if (iX == -1 || iY == -1) {
            return NULL;
        }

        int iResult = iX + iY + carry;    

        if (iResult > 15) {
            carry = iResult / 16;
            while (iResult > 15) {
                iResult = iResult - 16;
            }
        } else {
            carry = 0;
        }

        char cResult;
        sprintf(&cResult, "%x", iResult);

        result[resultLength - 1 - indCurrentResult] = cResult;
        indCurrentResult++;
    }

    if (carry != 0) {
        char cCarry;
        sprintf(&cCarry, "%x", carry);

        result[resultLength - 1 - indCurrentResult] = cCarry;
    }

    int i = 0;
    while (result[i] == '0') {
        i++;
    }

    char* resultTrimmed = malloc(sizeof(char) * (resultLength - i + 1));

    if (resultTrimmed == NULL) {
        free(result);
        return NULL;
    }

    for (int j = 0; j < resultLength - i + 1; j++) {
        resultTrimmed[j] = result[j+i];
    }

    resultTrimmed[resultLength - i] = '\0';

    free(result);

    return resultTrimmed;
}

char* withZeros(char* str, int numZeros) {
    char* strWithZeros = malloc(sizeof(char) * (strlen(str) + numZeros + 1));

    if (strWithZeros == NULL) {
        return NULL;
    }

    for (int i = 0; i < strlen(str) - 1 + numZeros; i++) { // - 1 to remove the \n
        if (i < strlen(str) - 1) { // - 1 to remove the \n
            strWithZeros[i] = str[i];
        } else {
            strWithZeros[i] = '0';
        }
    }
    strWithZeros[strlen(str) + numZeros] = '\0';

    return strWithZeros;
}