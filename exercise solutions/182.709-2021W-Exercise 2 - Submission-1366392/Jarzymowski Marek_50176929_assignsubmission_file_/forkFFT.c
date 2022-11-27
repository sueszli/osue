/**
 * @file forkFFT.c
 * @author Marek Jarzymowski <e11721384@student.tuwien.ac.at>
 * @date 11.12.2021
 * @brief This programm calculcates the FFT recursively. We use pipes and fork to comply to assigment.
 **/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

static char *thisProgramName;

/**
 * @brief turns the input into a complex number
 * @details we read from the char* turn it to a complex number, which is written to complex*.
 * @param input which should be turned into complex number.
 * @param complex_number pointer to where the result is written to.
 * @return Returns true if successfull, otherwise false.
**/
bool inputToComplex(char* input, float complex* complexNum){
    float imaginary = 0;
    float real = 0;

    char* pointer1 = NULL;
    char* pointer2 = NULL;

    real = strtof(input, &pointer1);
    imaginary = strtof(pointer1, &pointer2);

    //strof fail => value = 0
    if(real == 0 && pointer2 == input){
        return false;
    }

    //has to end with either \0 or \n, otherwise operation was incomplete.
    if(*pointer2 != '\0' && *pointer2 != '\n'){
        return false;
    }

    //create complex number:
    *complexNum = real + imaginary * I;
    return true;
}

/**
 * @brief Program entry point. 
 * This program calculates the FFT by using the Cooley-Tukey algorithm. 
 * @details Program takes 0 arguments as an input, but reads it. If there's 1 input provided,
 * it gives back its' value. If more than 1 it forks. Each child forks as long as there's only
 * one input provided. Then it is returned to the parent that calculates the FFT. 
 * @param argc argument counter.
 * @param argv argument vector.
 * @return Can return either EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char** argv){

    //No args allowed by default
    if(argc != 1){
        fprintf(stderr, "Usage: %s\n Wrong number of arguments!\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    thisProgramName = argv[0];


    size_t capacity1 = 0;
    size_t capacity2 = 0;

    char* str1 = NULL;
    char* str2 = NULL;

    ssize_t l1 = 0;
    ssize_t l2 = 0;

    l1 = getline(&str1, &capacity1, stdin);
    l2 = getline(&str2, &capacity2, stdin);

    //Check if there's at least some input:
    if(l1 == -1){
        free(str1);
        fprintf(stderr, "Error: No input provided!\n");
        exit(EXIT_FAILURE);
    }
    
    //Only 1 input => print to stdout directly.
    if(l2 == -1){
        float complex complexNum;
        bool wasTransofmed = false;
        wasTransofmed = inputToComplex(str1,&complexNum);
        if(wasTransofmed == false){
            fprintf(stderr,"Problem transforming input to complex number!");
            free(str1);
            free(str2);
            exit(EXIT_FAILURE);
        }
        //We use creal(1) and cimag(1) to output complex number as real + imaginary part.
        fprintf(stdout, "%f %f\n", creal(complexNum), cimag(complexNum));

        free(str1);
        free(str2);
        exit(EXIT_SUCCESS);
    }

    //If we are here, it means l1 != -1 and l2 != -1, which points to >=2 input. Therefore we create FD's
    // for communication with children and start forking afterwards.


    int fd_writeTo1[2];
    int fd_readFrom1[2];
    int fd_writeTo2[2];
    int fd_readFrom2[2];

    //We create pipes.
    int fdP1 = pipe(fd_writeTo1);
    int fdP2 = pipe(fd_writeTo2);
    int fdP3 = pipe(fd_readFrom1);
    int fdP4 = pipe(fd_readFrom2);

    //Check if creation was successful.
    if(fdP1 == -1 || fdP2 == -1 || fdP3 == -1 || fdP4 == -1){
        fprintf(stderr,"Error: one of the pipes failed.\n");
        free(str1);
        free(str2);
        exit(EXIT_FAILURE);
    }
    
    pid_t pidF1 = fork();

    switch(pidF1){
        case -1:
            fprintf(stderr, "1st fork failed.\n");
            free(str1);
            free(str2);
            exit(EXIT_FAILURE);
            break;
        case 0:
            //Due to return value of fork, this means we got a child 1.
            //First we close all unnecessary pipes - as we don't need pipes from Child2 in Child1.
            close(fd_writeTo2[0]);
            close(fd_writeTo2[1]);
            close(fd_readFrom2[0]);
            close(fd_readFrom2[1]);
            // We dupe twice in order to allow: (Parent RD from Child WR to stdout) and (Child RD from stdin Parent WR to stdin). All this with pipes.
            if(dup2(fd_writeTo1[0], STDIN_FILENO) == -1 || dup2(fd_readFrom1[1], STDOUT_FILENO) == -1){
                fprintf(stderr,"Error: one of the dups failed.\n");
            }

            //Now we can close rest of the pipes, as they are already duped and exec program:
            close(fd_writeTo1[0]);
            close(fd_writeTo1[1]);
            close(fd_readFrom1[0]);
            close(fd_readFrom1[1]);
            execl(thisProgramName, thisProgramName, NULL);

            //should not proceed here, but if it does - free resources and close.
            free(str1);
            free(str2);
            exit(EXIT_FAILURE);
            break;
        default: 
            break;
    }

    // Back at the parent again (again due to switch and fork return value) - we fork 2nd time analogous to above.

    pid_t pidF2 = fork();

    switch(pidF2){
        case -1:
            fprintf(stderr, "2nd fork failed.\n");
            free(str1);
            free(str2);
            exit(EXIT_FAILURE);
            break;
        case 0:
            close(fd_writeTo1[0]);
            close(fd_writeTo1[1]);
            close(fd_readFrom1[0]);
            close(fd_readFrom1[1]);
            if(dup2(fd_writeTo2[0], STDIN_FILENO) == -1 || dup2(fd_readFrom2[1], STDOUT_FILENO) == -1){
                fprintf(stderr,"Error: one of the dups failed for 2nd child.\n");
            }
            close(fd_writeTo2[0]);
            close(fd_writeTo2[1]);
            close(fd_readFrom2[0]);
            close(fd_readFrom2[1]);
            execl(thisProgramName, thisProgramName, NULL);

            //Same as above, we should never get here.
            free(str1);
            free(str2);
            exit(EXIT_FAILURE);
            break;
        default:
            break;
    }


    // Here we finally handle parent of the both children above.
    FILE* wrChild1 = fdopen(fd_writeTo1[1], "w");
    FILE* wrChild2 = fdopen(fd_writeTo2[1], "w");

    //Before we use I/O lib for operations, we check if fdopen didn't fail for both children.
    if(wrChild1 == NULL || wrChild2 == NULL){
        fprintf(stderr, "Error: fdopen failed.\n");
        free(str1);
        free(str2);
        close(fd_writeTo1[1]);
        close(fd_readFrom1[0]); 
        close(fd_writeTo2[0]); 
        close(fd_readFrom2[1]); 

        //We have to wait untill both child terminate.
        int status = 0;
        while (true){
            if(waitpid(pidF1, &status, 0) != -1){
                break;
            }
        }
        while (true){
            if(waitpid(pidF2, &status, 0) != -1){
                break;
            }
        }
        exit(EXIT_FAILURE);
    }   


    if(fprintf(wrChild1, "%s", str1) < 0){
        fprintf(stderr, "Error while printing at child 1.\n");
        free(str1);
        free(str2);
        fclose(wrChild1);
        fclose(wrChild2);


        //We have to wait untill both child terminate.
        int status = 0;
        while (true){
            if(waitpid(pidF1, &status, 0) != -1){
                break;
            }
        }
        while (true){
            if(waitpid(pidF2, &status, 0) != -1){
                break;
            }
        }
        exit(EXIT_FAILURE);
    }

    if(fprintf(wrChild2, "%s", str2) < 0){
        fprintf(stderr, "Error while printing at child 2.\n");
        free(str1);
        free(str2);
        fclose(wrChild1);
        fclose(wrChild2);

        //We have to wait untill both child terminate.
        int status = 0;
        while (true){
            if(waitpid(pidF1, &status, 0) != -1){
                break;
            }
        }
        while (true){
            if(waitpid(pidF2, &status, 0) != -1){
                break;
            }
        }
        exit(EXIT_FAILURE);
    }



    // we start with size = 2 and will incrementally make it bigger in the loop below.
    // In general the input size must be 2^n to be valid, which will be checked in the end.
    size_t size = 2;

    while(true){
        if(getline(&str1, &capacity1,stdin) == -1){
            break;
        }
        size++;
        fprintf(wrChild1, "%s", str1);

        if(getline(&str2, &capacity2, stdin) == -1){
            break;
        }
        size++;
        fprintf(wrChild2, "%s", str2);
    }

    fflush(wrChild1);
    fflush(wrChild2);

    //If input won't be even in a child or parent, this will fire off and we will stop.
    if(size%2 != 0){
        fprintf(stderr, "Error: input not even in a parent or child. [only input of type 2^n acceptable]\n");
        fclose(wrChild1);
        fclose(wrChild2);
        free(str1);
        free(str2);
        int status = 0;
        while (true){
            if(waitpid(pidF1, &status, 0) != -1){
                break;
            }
        }
        while (true){
            if(waitpid(pidF2, &status, 0) != -1){
                break;
            }
        }
        exit(EXIT_FAILURE);
    }

    //We close all not needed pipes. R - W and W - R from 1 and 2 respectively. 
    close(fd_writeTo1[0]);
    close(fd_readFrom1[1]);
    close(fd_writeTo2[0]);
    close(fd_readFrom2[1]);

    //After reading, we need to close FP's

    fclose(wrChild1);
    fclose(wrChild2);

    //Again we wait for children to terminate.
    int waitStatus1;
    int waitStatus2;
    while (true){
        if(waitpid(pidF1, &waitStatus1, 0) != -1){
            break;
        }
    }
    while (true){
        if(waitpid(pidF2, &waitStatus2, 0) != -1){
            break;
        }
    }

    //We ensure correct exit of children.
    //This ensures that we don't make unnecessary calculations and outputs if for example input isn't 2^n type.
    if(WEXITSTATUS(waitStatus1) != EXIT_SUCCESS || WEXITSTATUS(waitStatus2) != EXIT_SUCCESS){
        fprintf(stderr, "Error: one of the children had erroneus exit.\n");
        close(fd_readFrom1[0]);
        close(fd_readFrom2[0]);
        free(str1);
        free(str2);
        exit(EXIT_FAILURE);
    }

    // After children exit, we can read the results and use "butterfly" formula on them.
    FILE* readFromChild1 = fdopen(fd_readFrom1[0],"r");
    FILE* readFromChild2 = fdopen(fd_readFrom2[0],"r");

    if(readFromChild1 == NULL || readFromChild2 == NULL){
        fprintf(stderr, "Error: reading from children failed.\n");
        close(fd_readFrom1[0]);
        close(fd_readFrom2[0]);
        free(str1);
        free(str2);
        fclose(readFromChild1);
        fclose(readFromChild2);
        exit(EXIT_FAILURE);
    }

    //for calcs later.
    int x = 0;
    float PI = 3.141592654;        
    float complex re;
    float complex ro;
    //2nd array of values that will be output after 1st.
    float complex half2[size/2];
    
    // Here we calc and store resulst for both arrays and output the first half.
    while(true){
        int checkL1 = getline(&str1, &capacity1, readFromChild1);
        int checkL2 = getline(&str2, &capacity2, readFromChild2);

        //If we can't read anymore, we break.
        if(checkL1 == -1 || checkL2 == -1){
            break;
        }

        if(strcmp(str1, "\n") == 0){
            break;
        }

        if(strcmp(str2, "\n") == 0){
            break;
        }

        //We convert lines to complex numbers for calcs.
        bool convCheckRO = inputToComplex(str2, &ro);
        bool convCheckRE = inputToComplex(str1, &re);

        //Check if conversion failed.
        if(convCheckRE == false || convCheckRO == false){
            fclose(readFromChild1);
            fclose(readFromChild2);
            free(str1);
            free(str2);
            fprintf(stderr,"Storing in complex numbers failed, exiting now.\n");
            exit(EXIT_FAILURE);
        }


        //Now we calculate the results using "butterfly" formula.
        float complex output1 = re+(cos((-(2*PI)/size)*x)+I*sin((-(2*PI)/size)*x))*ro;
        float complex output2 = re-(cos((-(2*PI)/size)*x)+I*sin((-(2*PI)/size)*x))*ro;

        //Fill the 2nd half with 2nd part of output.
        half2[x] = output2;
        x++;
        //Output the first part.
        fprintf(stdout, "%f %f\n", creal(output1), cimag(output1));
    }

    //Here we output the 2nd part.
    int o2Size = 0;
    while(o2Size < size/2){
        fprintf(stdout, "%f %f\n", creal(half2[o2Size]), cimag(half2[o2Size]));
        o2Size++;
    }
    
    //After outputting, we close anything that hasn't been closed yet, free resources, and exit.
    free(str1);
    free(str2);
    fclose(readFromChild1);
    fclose(readFromChild2);
    exit(EXIT_SUCCESS);
}