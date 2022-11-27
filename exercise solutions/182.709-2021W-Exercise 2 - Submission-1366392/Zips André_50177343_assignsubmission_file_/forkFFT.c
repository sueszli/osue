/**
 * @file forkFFT.c
 * @author Andre Zips 11811363 
 * @brief Computes Fourier Transform of inputs recursivly
 * @date 2021-12-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

#define PI 3.141592654

int lineCount = 0;
int podd[2], peven[2];
int rodd[2], reven[2];
int resOfSecondPart[2];

struct ComplexNumber {
    float real;
    float imaginary;
};

struct ComplexNumber parseString(char* string){
    struct ComplexNumber result;
    char *r;
    result.real = strtof(string, &r);
    result.imaginary = strtof(&r[1], &r);

    if(!(strcmp(r, "*i\n") != 0 || (strcmp(r, "\n") != 0))) {
        fprintf(stderr, "Could not parse float! \n");
            exit(EXIT_FAILURE);
    }

    return result;
}

/**
 * @brief print a complex number.
 * 
 * @param number the complex number to be printed
 */
void printComplexNumber(struct ComplexNumber number){
    printf("%f %f*i\n", number.real, number.imaginary);
}

/**
 * @brief the main programm of the furier transformation with fork and piplines
 * 
 * @param argc count of input arguments
 * @param argv array of input arguments with length of argc
 * @return Exit code 
 */
int main(int argc, char *argv[]) {
    pipe(podd);
    pipe(rodd);
    pipe(peven);
    pipe(reven);
    FILE *poddFile = fdopen(podd[1], "w");
    FILE *pevenFile = fdopen(peven[1], "w");

    char oldc, c;
    while(1){
        oldc = c;
        c = fgetc(stdin);
        if(feof(stdin)){
            if(oldc != '\n') lineCount++;
            break;
        }
        FILE *f;
        if(lineCount%2 == 0){
            f = poddFile;
        }else{
            f = pevenFile;
        }
        
        if(c == '\n') lineCount++;
        
        fputc(c, f);
    }

    fclose(poddFile);
    fclose(pevenFile);

    char buffer[1024];
    if(lineCount == 1){
        FILE *f = fdopen (podd[0], "r");
        fgets(buffer, sizeof(buffer), f);
        struct ComplexNumber cn = parseString(buffer);
        printComplexNumber(cn);
        fclose(f);
        exit(EXIT_SUCCESS);
    }
    if(lineCount%2 == 1){
        fprintf(stderr, "Uneaven line count\n");
        exit(EXIT_FAILURE);
    }

    pid_t pevenId = fork();
    switch(pevenId){
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0:
            close(peven[1]);
            dup2(peven[0], STDIN_FILENO);
            close(peven[0]);
            close(reven[0]);
            dup2(reven[1], STDOUT_FILENO);
            close(reven[1]);

            execlp("./forkFFT", argv[0], NULL);
            //On Error
            fprintf(stderr, "Execution of child process peven failed\n");
            exit(EXIT_FAILURE);
            break;
        default: break;
    }

    pid_t poddId = fork();
    switch(poddId){
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0:
            close(podd[1]);
            dup2(podd[0], STDIN_FILENO);
            close(podd[0]);
            close(rodd[0]);
            dup2(rodd[1], STDOUT_FILENO);
            close(rodd[1]);

            execlp("./forkFFT", argv[0], NULL);
            //On Error
            fprintf(stderr, "Execution of child process podd failed\n");
            exit(EXIT_FAILURE);
            break;
        default: break;
    }

    close(peven[0]);
    close(peven[1]);
    close(podd[0]);
    close(podd[1]);
    close(reven[1]);
    close(rodd[1]);

    int oddStatus, evenStatus;
    waitpid(pevenId, &evenStatus, 0);
    waitpid(poddId, &oddStatus, 0);

    FILE* resEvenFile = fdopen(reven[0], "r");
    FILE* resOddFile = fdopen(rodd[0], "r");
    char buffer2[1024];
    pipe(resOfSecondPart);
    FILE *secondHalfStdOut = fdopen(resOfSecondPart[1], "w");
    for(int k = 0; k < lineCount/2; k++){
        if(fgets(buffer2, sizeof(buffer2), resOddFile) == NULL || fgets(buffer, sizeof(buffer), resEvenFile) == NULL) {
            fprintf(stderr, "buffer size not matching \n");
            exit(EXIT_FAILURE);
        }
        
        struct ComplexNumber roddNumber = parseString(buffer);
        struct ComplexNumber revenNumber = parseString(buffer2);
        float x = (-((2*PI)/lineCount)*k);
        struct ComplexNumber xComp;
        xComp.imaginary = sin(x);
        xComp.real = cos(x);

        struct ComplexNumber multiplicationStep;
        multiplicationStep.real = xComp.real*roddNumber.real - xComp.imaginary*roddNumber.imaginary;
        multiplicationStep.imaginary = xComp.imaginary*roddNumber.real + xComp.real*roddNumber.imaginary; 

        struct ComplexNumber firstHalfNumber;
        firstHalfNumber.real = revenNumber.real + multiplicationStep.real;
        firstHalfNumber.imaginary = revenNumber.imaginary + multiplicationStep.imaginary;

        struct ComplexNumber secondHalfNumber;
        secondHalfNumber.real = revenNumber.real - multiplicationStep.real;
        secondHalfNumber.imaginary = revenNumber.imaginary - multiplicationStep.imaginary;

        printComplexNumber(firstHalfNumber);
        fprintf(secondHalfStdOut, "%f %f*i\n", secondHalfNumber.real, secondHalfNumber.imaginary);
    }

    fclose(resEvenFile);
    fclose(resOddFile);
    fclose(secondHalfStdOut);
    close(reven[0]);
    close(rodd[0]);
    close(resOfSecondPart[1]);

    secondHalfStdOut = fdopen(resOfSecondPart[0], "r");
    while(1){
        c = fgetc(secondHalfStdOut);
        if(feof(secondHalfStdOut)){
            break;
        }
        fputc(c, stdout);
    }
    fclose(secondHalfStdOut);
    close(resOfSecondPart[0]);
    if(evenStatus != 0 || oddStatus != 0){
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
