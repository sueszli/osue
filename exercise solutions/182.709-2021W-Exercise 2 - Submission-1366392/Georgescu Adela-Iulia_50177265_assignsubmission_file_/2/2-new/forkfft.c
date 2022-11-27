/**
* @author Adela-Iulia Georgescu, 11810894
* @date 08.12.2022
* @brief computes the Fork Fourier Transform of its input values recursively
* @details given a list of numbers, the program divides them into 2 parts: even indexes 
*          and uneven indices. If there is just one nr given, the number itself is printed
*          the child processes get the numbers from the parent by redirecting stdin and 
*          write to the parent by redirecting stdout
**/


#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <errno.h>

#define PI 3.141592654

//a is the real part and b is the imaginary part of the complex number
typedef struct complexNr{
    double a;
    double b;
} complexNr;

char* myProgram;

//parses the complex number and transforms what is given in stdin in char to double
void parseComplexNr(complexNr *c, char* line);

//adds 2 complex numbers
void addition(complexNr *first, complexNr *second, complexNr *result);

//substracts 2 complex numbers
void substraction(complexNr *first, complexNr *second, complexNr *result);

//multiplies 2 complex numbers
void multiplication(complexNr *first, complexNr *second, complexNr *result);

//prints a complex number
void printComplexNr(complexNr *c, FILE* outputFile);

//prints an error and finishes with EXIT_FAILURE
void error(char *message);

//calculates a part of the fourier transform
void calculate(complexNr *complexArray1, complexNr *complexArray2, int count, complexNr *result);

//returns the fourier constant  as required by calculate
void getFourierConstant(complexNr *result, int k, int n);

//execution of the first child
void execFirstChild(char* cwd, int* childToParent, int* parentToChild);

//execution of the second child
void execSecondChild(char* cwd, int* firstChildToParent, int* parentToFirstChild, int* secondChildToParent, int* parentToSecondChild);

int main(int argc, char *argv[]){

    //initialize the program name with the first element
    myProgram = argv[0];

    //reads the first line or the first 2 lines
    char *firstLine = NULL;
    char *secondLine = NULL;
    size_t len;
    int sth = 0;

    sth = getline(&firstLine, &len, stdin);
    if(sth < 0){
        error("getline for first line failed");
    }
    errno = 0;
    sth = getline(&secondLine, &len, stdin);
    if(sth <0 && errno != 0){
        error("getline for second line failed");
    }
    //if there was just a number given, errno should still be 0, but sth should be negative and we have to write the first number to stdout
    if(sth < 0){
        complexNr nr = {0, 0};
        parseComplexNr(&nr, firstLine);
        printComplexNr(&nr, stdout);
        exit(EXIT_SUCCESS);
    }

    //create pipes 
    int parentToFirstChild[2];
    int firstChildToParent[2];
    int parentToSecondChild[2];
    int secondChildToParent[2];
    pipe(parentToFirstChild);
    pipe(firstChildToParent);
    pipe(parentToSecondChild);
    pipe(secondChildToParent);

    //fork first child
    pid_t firstPid = fork();
    if(firstPid < 0){
        error("fork for first child failed");
    }

    char cwd[10000];
    getcwd(cwd, 10000);
    strcat(cwd, "/forkfft");

    //let first child execute the program
    if(firstPid == 0) execFirstChild(cwd, firstChildToParent, parentToFirstChild);

    // close the pipes
    close(parentToFirstChild[0]);
    close(firstChildToParent[1]);

    //fork second child
    pid_t secondPid = fork();
    if(secondPid < 0){
        error("fork for second child failed");
    }

    //let second child execute the program
    if(secondPid == 0) execSecondChild(cwd, firstChildToParent, parentToFirstChild, secondChildToParent, parentToSecondChild);

    // close the pipes
    close(parentToSecondChild[0]);
    close(secondChildToParent[1]);

    //creates files for the pipes
    FILE* writeFirstChild = fdopen(parentToFirstChild[1], "w");
    FILE* writeSecondChild = fdopen(parentToSecondChild[1], "w");
    FILE* readFirstChild = fdopen(firstChildToParent[0], "r");
    FILE* readSecondChild = fdopen(secondChildToParent[0], "r");

    //writes the first 2 lines to the children
    fprintf(writeFirstChild, "%s", firstLine);
    fflush(writeFirstChild);
    fprintf(writeSecondChild, "%s", secondLine);
    fflush(writeSecondChild);

    //read lines from stdin and give them to children
    int count = 1;
    do{
        count++;
        firstLine = NULL;
        errno = 0;
        sth = getline(&firstLine, &len, stdin);
        if(sth == -1 && errno != 0){
            error("getline failed");
        }
        if(count % 2 == 0){
            fprintf(writeFirstChild, "%s", firstLine);
            fflush(writeFirstChild);
        }
        else{
            fprintf(writeSecondChild, "%s", firstLine);
            fflush(writeSecondChild);
        }
    }while(sth >= 0);

    //if the nr of lines is uneven, then kill the children and exit
    if(count % 2 != 0){
        kill(firstPid, SIGKILL);
        kill(secondPid, SIGKILL);
        error("number of lines is uneven");
    }else{
        close(parentToFirstChild[1]);
        close(parentToSecondChild[1]);
    }

    //wait for the children
    int firstStatus = 0;
    int secondStatus = 0;
    sth = waitpid(firstPid, &firstStatus, 0);
    if(sth < 0){
        error("first waitpid failed");
    }
    sth = waitpid(secondPid, &secondStatus, 0);
    if(sth < 0){
        error("second waitpid failed");
    }
    if(WEXITSTATUS(firstStatus) == EXIT_FAILURE || WEXITSTATUS(secondStatus) == EXIT_FAILURE){
        error("at least one child process failed");
    }

    //read what has been calculated and print it
    complexNr complexArrayAll[count];
    complexNr complexArray1[count/2];
    complexNr complexArray2[count/2];

    firstLine = NULL;
    len = 0;
    for(int i = 0; i < count/2; i++){
        sth = getline(&firstLine, &len, readFirstChild);
        if(sth < 0){
            error("getline failed");
        }
        parseComplexNr(&complexArray1[i], firstLine);
        firstLine = NULL;
        sth = getline(&firstLine, &len, readSecondChild);
        if(sth < 0){
            error("getline failed");
        }
        parseComplexNr(&complexArray2[i], firstLine);
        firstLine = NULL;
    }

    //calculate
    calculate(complexArray1, complexArray2, count, complexArrayAll);
    for(int i = 0; i < count; i++){
        printComplexNr(&complexArrayAll[i], stdout);
    }

}

void error(char* message){
    fprintf(stderr, "%s: %s\n", myProgram, message);
    exit(EXIT_FAILURE);
}

void addition(complexNr *first, complexNr *second, complexNr *result){
    result->a = first->a + second->a;
    result->b = first->b + second->b;
}

void substraction(complexNr *first, complexNr *second, complexNr *result){
    result->a = first->a - second->a;
    result->b = first->b - second->b;
}

void multiplication(complexNr *first, complexNr *second, complexNr *result){
    result->a = (first->a * second->a) - (first->b * second->b);
    result->b = (first->a * second->b) + (first->b * second->a);
}
void parseComplexNr(complexNr *c, char* line){

    //parse first nr
    char* firstRest;
    c->a = strtod(line, &firstRest);
    if(strcmp(firstRest, line)==0 && (c->a)==0){
        error("strtod failed for first number");
    }

    //parse second nr
    char* secondRest;
    c->b = strtod(firstRest, &secondRest);
    if(strcmp(firstRest, secondRest)==0 && 
    (c->b)==0 &&strcmp(secondRest, "")!=0 && strcmp(secondRest,"\n")!=0 && 
    strcmp(secondRest,"*i")!=0 &&strcmp(secondRest,"*i\n")!=0){
        error("strtod failed for second number");
    }
}

void getFourierConstant(complexNr *result, int k, int n){
	double inner = -(2*PI*k)/n;
	result->a = cos(inner);
	result->b = sin(inner);
}

void calculate(complexNr *complexArray1, complexNr *complexArray2, int count, complexNr *result){
    complexNr *iResult1 = calloc(1, sizeof(complexNr));
    complexNr *iResult2 = calloc(1, sizeof(complexNr));

    for(int i = 0; i < count; i++){
        getFourierConstant(iResult1, i % (count/2), count);
        multiplication(iResult1, &complexArray2[i%(count/2)], iResult2);
        if(i<(count/2)){
            addition(&complexArray1[i%(count/2)], iResult2, &result[i]);
        }
        else{
            substraction(&complexArray1[i%(count/2)], iResult2, &result[i]);
        }
    }
    free(iResult1);
    free(iResult2);
}

void printComplexNr(complexNr *c, FILE* outputFile){
    fprintf(outputFile, "%f %f*i\n", c->a, c->b);
    fflush(outputFile);
}

void execFirstChild(char* cwd, int* childToParent, int* parentToChild){
  close(parentToChild[1]);
  close(childToParent[0]);
  dup2(parentToChild[0], STDIN_FILENO);
  dup2(childToParent[1], STDOUT_FILENO);
  close(parentToChild[0]);
  close(childToParent[1]);
  int check = execl(cwd,"./forkfft", NULL);
  if(check<0){
    error("execl failed");
  }
}

void execSecondChild(char* cwd, int* firstChildToParent, int* parentToFirstChild, int* secondChildToParent, int* parentToSecondChild){
  close(parentToFirstChild[1]);
  close(firstChildToParent[0]);
  close(parentToSecondChild[1]);
  close(secondChildToParent[0]);
  dup2(parentToSecondChild[0], STDIN_FILENO);
  dup2(secondChildToParent[1], STDOUT_FILENO);
  close(parentToSecondChild[0]);
  close(secondChildToParent[1]);
  int check = execl(cwd,"./forkfft", NULL);
  if(check<0){
    error("execl failed");
  }
}
