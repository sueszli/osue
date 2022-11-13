/**
 * @author Maximilian Maresch
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "addition.h"
#include "child.h"
#include "util.h"

/**
 * @brief Prints the usage message for this program and exits the process with failure
 * @detail Prints the usage message to stderr which shows how to correctly use this program and exits the process with failure
 * @param argv - argv from main
 */ 
void usage(char *argv[]) {
    fprintf(stderr,"[%s] Usage: intmul \n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * @brief Exits the process with failure and prints a message
 * @detail This function prints msg to stderr and exits the process with failure
 * @param msg - the message to print to stderr before exiting
 * @param argv - argv from main
 */ 
void exitEarly(char* msg, char* argv[]) {
    fprintf(stderr,"[%s] %s \n", argv[0], msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) { 

    if (argc != 1) {
        usage(argv);
    }

    char *A = NULL;
    char *B = NULL;

    ssize_t lineSizeA = 0;
    ssize_t lineSizeB = 0;

    size_t lenA = 0;
    size_t lenB = 0;
    lineSizeA = getline(&A, &lenA, stdin) - 1; // remove delimitor character
    lineSizeB = getline(&B, &lenB, stdin) - 1; // remove delimitor character

    if (A[strlen(A)-1] == '\n') {
        A[strlen(A)-1] = '\0';   
    }

    if (B[strlen(B)-1] == '\n') {
        B[strlen(B)-1] = '\0';
    }

    if (A[0] == '-' || B[0] == '-') {
        freeAll2(A, B);
        exitEarly("Negative numbers were found - Provide positive numbers only", argv);
    }

    if (lineSizeA != lineSizeB) {
        freeAll2(A, B);
        exitEarly("Line sizes were not equal", argv);
    } else if (lineSizeA == 1) {
        int Adec = hexStrToInt(A);
        int Bdec = hexStrToInt(B);

        if (Adec == -1 || Bdec == -1) {
            freeAll2(A, B);
            exitEarly("Failed to convert the received numbers", argv);
        }

        printf("%x\n", Adec*Bdec);
        fflush(stdout);

        free(A);
        free(B);

        exit(EXIT_SUCCESS);
    } else if (lineSizeA % 2 != 0) {
        freeAll2(A, B);
        exitEarly("Not divisible by 2", argv);
    }

    int forkLineSize = lineSizeA / 2;
    char forkA1[forkLineSize + 1];
    char forkA2[forkLineSize + 1];

    for (int i = 0; i < lineSizeA; i++) {
        if (i < forkLineSize) {
            forkA1[i] = A[i];
        } else {
            forkA2[i-forkLineSize] = A[i];
        }
    }

    char forkB1[forkLineSize + 1];
    char forkB2[forkLineSize + 1];
    for (int i = 0; i < lineSizeB; i++) {
        if (i < forkLineSize) {
            forkB1[i] = B[i];
        } else {
            forkB2[i-forkLineSize] = B[i];
        }
    }

    forkA1[forkLineSize] = '\0';
    forkA2[forkLineSize] = '\0';
    forkB1[forkLineSize] = '\0';
    forkB2[forkLineSize] = '\0';

    free(A);
    free(B);

    int pipefd1Child1Mult11[2];
    int pipefd2Child1Mult11[2];

    int pipefd1Child2Mult12[2];
    int pipefd2Child2Mult12[2];

    int pipefd1Child3Mult21[2];
    int pipefd2Child3Mult21[2];

    int pipefd1Child4Mult22[2];
    int pipefd2Child4Mult22[2];

    pipe(pipefd1Child1Mult11);
    pipe(pipefd2Child1Mult11);

    pipe(pipefd1Child2Mult12);
    pipe(pipefd2Child2Mult12);

    pipe(pipefd1Child3Mult21);
    pipe(pipefd2Child3Mult21);

    pipe(pipefd1Child4Mult22);
    pipe(pipefd2Child4Mult22);

    pid_t child1 = setupChild(pipefd1Child1Mult11, pipefd2Child1Mult11, forkA1, forkB1, argv);
    pid_t child2 = setupChild(pipefd1Child2Mult12, pipefd2Child2Mult12, forkA1, forkB2, argv);
    pid_t child3 = setupChild(pipefd1Child3Mult21, pipefd2Child3Mult21, forkA2, forkB1, argv);
    pid_t child4 = setupChild(pipefd1Child4Mult22, pipefd2Child4Mult22, forkA2, forkB2, argv);

    if (child1 == -1 || child2 == -1 || child3 == -1 || child4 == -1) {
        exitEarly("Failed to setup children", argv);
    }

    int statusChild1;
    int statusChild2;
    int statusChild3;
    int statusChild4;

    waitpid(child1, &statusChild1, 0);
    waitpid(child2, &statusChild2, 0);
    waitpid(child3, &statusChild3, 0);
    waitpid(child4, &statusChild4, 0);

    if (WEXITSTATUS(statusChild1) != EXIT_SUCCESS || 
        WEXITSTATUS(statusChild2) != EXIT_SUCCESS || 
        WEXITSTATUS(statusChild3) != EXIT_SUCCESS || 
        WEXITSTATUS(statusChild4) != EXIT_SUCCESS) {
        exitEarly("Children did not exit successfully", argv);
    }

    char* resultChild1 = readResultFromChild(pipefd2Child1Mult11, argv);
    char* resultChild2 = readResultFromChild(pipefd2Child2Mult12, argv);
    char* resultChild3 = readResultFromChild(pipefd2Child3Mult21, argv);
    char* resultChild4 = readResultFromChild(pipefd2Child4Mult22, argv);

    if (resultChild1 == NULL || resultChild2 == NULL || resultChild3 == NULL || resultChild4 == NULL) {
        freeAll4(resultChild1, resultChild2, resultChild3, resultChild4);
        exitEarly("Failed to read result from children", argv);
    }

    char* resultChild1WithZeros = withZeros(resultChild1, lineSizeA);
    char* resultChild2WithZeros = withZeros(resultChild2, forkLineSize);
    char* resultChild3WithZeros = withZeros(resultChild3, forkLineSize);
    resultChild4[strlen(resultChild4) - 1] = '\0';

    if (resultChild1WithZeros == NULL || resultChild2WithZeros == NULL || resultChild3WithZeros == NULL) {
        freeAll4(resultChild1, resultChild2, resultChild3, resultChild4);
        freeAll3(resultChild1WithZeros, resultChild2WithZeros, resultChild3WithZeros);
        exitEarly("Malloc failed while adding zeros to the results of the children", argv);
    }

    char* resultChild23 = add(resultChild2WithZeros, resultChild3WithZeros);

    if (resultChild23 == NULL) {
        freeAll4(resultChild1, resultChild2, resultChild3, resultChild4);
        freeAll3(resultChild1WithZeros, resultChild2WithZeros, resultChild3WithZeros);
        exitEarly("Failed to add the received numbers", argv);
    }

    char* resultChild123 = add(resultChild1WithZeros, resultChild23);

    if (resultChild123 == NULL) {
        free(resultChild23);
        freeAll4(resultChild1, resultChild2, resultChild3, resultChild4);
        freeAll3(resultChild1WithZeros, resultChild2WithZeros, resultChild3WithZeros);
        exitEarly("Failed to add the received numbers", argv);
    }

    char* result = add(resultChild123, resultChild4);

    if (result == NULL) {
        freeAll2(resultChild123, resultChild23);
        freeAll2(A, B);
        freeAll4(resultChild1, resultChild2, resultChild3, resultChild4);
        freeAll3(resultChild1WithZeros, resultChild2WithZeros, resultChild3WithZeros);
        exitEarly("Failed to add the received numbers", argv);
    }

    printf("%s\n", result);
    fflush(stdout);

    freeAll4(resultChild1, resultChild2, resultChild3, resultChild4);
    freeAll3(resultChild1WithZeros, resultChild2WithZeros, resultChild3WithZeros);
    freeAll3(result, resultChild123, resultChild23);

    exit(EXIT_SUCCESS);

    return 0;
}