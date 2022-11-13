/**
 * @file mult.c
 * @author Marcel Boros <e11937377@student.tuwien.ac.at>
 * @date 06.12.2021
 *  
 * @brief Implementation file for functions declared in mult.h
 *
 */


#include "mult.h"



//closing pipes + error handling - arguments have to be pipes or pipe ends
void closePipes(int type, char** argv, int counter, ...) {
    va_list ap;
    va_start(ap, counter);
    int pipe_rd, pipe_wr;
    int pipe_end;
    
    for(int i=0; i<counter; ++i) {
        
        
        if(type == 0) {
            //type=pipe
            pipe_rd = va_arg(ap, int[2])[0];
            pipe_wr = va_arg(ap, int[2])[1];
        
            if(close(pipe_rd) == -1) {
                fprintf(stderr, "Error in %s -> close failed: %s \n",argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        
            if(close(pipe_wr) == -1) {
                fprintf(stderr, "Error in %s -> close failed: %s \n",argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        } 
        
        else {
            //type=pipe_end
            pipe_end = va_arg(ap, int);
            if(close(pipe_end) == -1) {
                fprintf(stderr, "Error in %s -> close failed: %s \n",argv[0], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }
    
    va_end(ap);
}



//pipe opening/error handling
void open_handle_pipes(int* c11, int* c12, int* c21, int* c22, int* c31, int* c32, int* c41, int* c42, char** argv,
                       char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2) {
    
    //check pipe creation for errors
    if(pipe(c11) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c12) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,1,c11);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c21) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,2,c11,c12);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c22) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,3,c11,c12,c21);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c31) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,4,c11,c12,c21,c22);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c32) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,5,c11,c12,c21,c22,c31);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c41) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,6,c11,c12,c21,c22,c31,c32);
        exit(EXIT_FAILURE);
    }
    
    if(pipe(c42) == -1) {
        fprintf(stderr, "Error in %s -> pipe failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv,7,c11,c12,c21,c22,c31,c32,c41);
        exit(EXIT_FAILURE);
    }
}



//multiplication function
void multiply(char* firstLine, char* secondLine, char** argv, char** solutionArray) {
    
    //here both numbers has to have equal size
    int n = strlen(firstLine);

    
    //if both lines only have one digit multiply immediately, print and return
    if((strlen(firstLine) == 1) && (strlen(secondLine) == 1)) {
        int result = hexToNumber(firstLine,0) * hexToNumber(secondLine,0);
        
        //print result in hex-format
        printf("%x\n", result);
        free(firstLine);
        free(secondLine);
        exit(EXIT_SUCCESS);
    }
    
    //terminate if the number of digits is not even
    if((strlen(firstLine) % 2 != 0) || (strlen(secondLine) % 2 != 0)) {
        fprintf(stderr, "Error in %s -> number of digits is not even:\nFirst Line: %s(%zu)\nSecond Line: %s(%zu)\n",argv[0], firstLine, strlen(firstLine)
        , secondLine, strlen(secondLine));
        free(firstLine);
        free(secondLine);
        exit(EXIT_FAILURE);
    }
    
    
    //split both integers in half and allocate memory
    char* firstHalf_1, *secondHalf_1, *firstHalf_2, *secondHalf_2;
    
    if((firstHalf_1 = malloc(sizeof(char) * (strlen(firstLine)/2))) == NULL) {
        fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        free(secondLine);
        exit(EXIT_FAILURE);
    }
    
    
     if((secondHalf_1 = malloc(sizeof(char) * (strlen(firstLine)/2))) == NULL) {
        fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        free(secondLine);
        free(firstHalf_1);
        exit(EXIT_FAILURE);
    }
    
    
    if((firstHalf_2 = malloc(sizeof(char) * (strlen(secondLine)/2))) == NULL) {
        fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        free(secondLine);
        free(firstHalf_1);
        free(secondHalf_1);
        exit(EXIT_FAILURE);
    }
    
    
    if((secondHalf_2 = malloc(sizeof(char) * (strlen(secondLine)/2))) == NULL) {
        fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        free(firstLine);
        free(secondLine);
        free(firstHalf_1);
        free(secondHalf_1);
        free(firstHalf_2);
        exit(EXIT_FAILURE);
    }
    
    
    //divide lines at middle into two parts
    divideLines(firstLine, firstHalf_1, secondHalf_1, secondLine, firstHalf_2, secondHalf_2);
    
    
    //parallelize
    parallelize(firstLine, firstHalf_1, secondHalf_1, secondLine, firstHalf_2, secondHalf_2, argv, solutionArray);
    
    //print solution in hex-format
    printf("%x\n", calculateSolution(solutionArray,n));
       
    
    free(firstHalf_1);
    free(secondHalf_1);
    free(firstHalf_2);
    free(secondHalf_2);
    
}



//calculate final solution from solutionArray by formula from the assignment
unsigned int calculateSolution(char** solutionArray, int n) {
    long int res[4];
    
    //interpret values as hex numbers
    for(int i=0; i<4; ++i) {
        //only valid values are stored in the solution
        res[i] = strtol(solutionArray[i], NULL, 16);
    }
    
    //calculate result as demanded in the assignment
    unsigned int solution = res[0] * pow(16,n) + res[1] * pow(16, n/2) + res[2] * pow(16, n/2) + res[3];
    return solution;
}



//release memory
void freeMemory(char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2) {
    free(fL);
    free(fH_1);
    free(sH_1);
    free(sL);
    free(fH_2);
    free(sH_2);
}



//parallelize process to improve computation
void parallelize(char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2, char** argv, char** solutionArray) {
    
    //create two pipes for each of the 4 children
    int pipefd_child1_1[2], pipefd_child1_2[2];  
    int pipefd_child2_1[2], pipefd_child2_2[2];  
    int pipefd_child3_1[2], pipefd_child3_2[2];  
    int pipefd_child4_1[2], pipefd_child4_2[2];  
    
    
    //pid values for process handling after fork
    pid_t pid_child1, pid_child2, pid_child3, pid_child4;
    
    
    //open pipes
    open_handle_pipes(pipefd_child1_1,pipefd_child1_2,pipefd_child2_1,pipefd_child2_2,pipefd_child3_1,pipefd_child3_2,
    pipefd_child4_1,pipefd_child4_2, argv, fL, fH_1, sH_1, sL, fH_2, sH_2);
    
    
    
    //create input for child processes
    char* nL = "\n"; //newline symbol as string
    
    //A_h,B_h -> inp1
    char inp1[strlen(fH_1) + strlen(fH_2) + 1];
    strcpy(inp1, fH_1);
    strcat(inp1, nL);
    strcat(inp1, fH_2);
    strcat(inp1, nL);
    
    //A_h,B_l -> inp2
    char inp2[strlen(fH_1) + strlen(sH_2) + 1];
    strcpy(inp2, fH_1);
    strcat(inp2, nL);
    strcat(inp2, sH_2);
    strcat(inp2, nL);
    
    //A_l,B_h -> inp3
    char inp3[strlen(sH_1) + strlen(fH_2) + 1];
    strcpy(inp3, sH_1);
    strcat(inp3, nL);
    strcat(inp3, fH_2);
    strcat(inp3, nL);
    
    //A_l,B_l -> inp4
    char inp4[strlen(sH_1) + strlen(sH_2) + 1];
    strcpy(inp4, sH_1);
    strcat(inp4, nL);
    strcat(inp4, sH_2);
    strcat(inp4, nL);
    
    
    
    //Write created input into the 4 write ends of the pipes
    write(pipefd_child1_2[1], &inp1, strlen(inp1));
    closePipes(1, argv, 1, pipefd_child1_2[1]);
    
    write(pipefd_child2_2[1], &inp2, strlen(inp2));
    closePipes(1, argv, 1, pipefd_child2_2[1]);
    
    write(pipefd_child3_2[1], &inp3, strlen(inp3));
    closePipes(1, argv, 1, pipefd_child3_2[1]);
    
    write(pipefd_child4_2[1], &inp4, strlen(inp4));
    closePipes(1, argv, 1, pipefd_child4_2[1]);
    
    
    
    //parallelize process...
    //check child creation for errors
    if((pid_child1 = fork()) == -1) {
        fprintf(stderr, "Error in %s -> fork failed: %s \n",argv[0], strerror(errno));
        freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
        closePipes(0,argv, 2,pipefd_child1_1, pipefd_child1_2);
        exit(EXIT_FAILURE);
    }
    
    
    if((pid_child2 = fork()) == -1) {
            fprintf(stderr, "Error in %s -> fork failed: %s \n",argv[0], strerror(errno));
            freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
            closePipes(0,argv, 4,pipefd_child1_1, pipefd_child1_2, pipefd_child2_1, pipefd_child2_2);
            exit(EXIT_FAILURE);
    }
   
   
    if((pid_child3 = fork()) == -1) {
            fprintf(stderr, "Error in %s -> fork failed: %s \n",argv[0], strerror(errno));
            freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
            closePipes(0,argv, 6,pipefd_child1_1, pipefd_child1_2, pipefd_child2_1, pipefd_child2_2, pipefd_child3_1, pipefd_child3_2);
            exit(EXIT_FAILURE);
    }
   
    
    if((pid_child4 = fork()) == -1) {
            fprintf(stderr, "Error in %s -> fork failed: %s \n",argv[0], strerror(errno));
            freeMemory(fL, fH_1, sH_1, sL, fH_2, sH_2);
            closePipes(0,argv, 8,pipefd_child1_1, pipefd_child1_2, pipefd_child2_1, pipefd_child2_2, pipefd_child3_1, pipefd_child3_2,
                   pipefd_child4_1, pipefd_child4_2);
            exit(EXIT_FAILURE);
    }
   
    

    //redirect stdin and stdout for children into pipes    
    if(pid_child1 == 0) {
        
        dup2(pipefd_child1_2[0], STDIN_FILENO);
        dup2(pipefd_child1_1[1], STDOUT_FILENO);
        
        //replace process image - execute
        if(execlp(argv[0], argv[0], NULL) == -1) {
            fprintf(stderr, "Error in %s -> execlp failed: %s \n",argv[0], strerror(errno));
        }
        
    }
    
    
    if(pid_child2 == 0) {
        
        dup2(pipefd_child2_2[0], STDIN_FILENO);
        dup2(pipefd_child2_1[1], STDOUT_FILENO);

        //replace process image - execute
        if(execlp(argv[0], argv[0], NULL) == -1) {
            fprintf(stderr, "Error in %s -> execlp failed: %s \n",argv[0], strerror(errno));
        }
    }
    
    
    if(pid_child3 == 0) {
        
        dup2(pipefd_child3_2[0], STDIN_FILENO);
        dup2(pipefd_child3_1[1], STDOUT_FILENO);

        //replace process image - execute
        if(execlp(argv[0], argv[0], NULL) == -1) {
            fprintf(stderr, "Error in %s -> execlp failed: %s \n",argv[0], strerror(errno));
        }
    }
    
    
    if(pid_child4 == 0) {
        
        dup2(pipefd_child4_2[0], STDIN_FILENO);
        dup2(pipefd_child4_1[1], STDOUT_FILENO);
        
        //replace process image - execute
        if(execlp(argv[0], argv[0], NULL) == -1) {
            fprintf(stderr, "Error in %s -> execlp failed: %s \n",argv[0], strerror(errno));
        }
    }
    
    
    
    //wait for all children to terminate
    int exitChd1, exitChd2, exitChd3, exitChd4; //exit codes of children
    if((exitChd1 = waitpid(pid_child1, NULL,0)) == -1) {
        fprintf(stderr, "Error in %s -> waitpid failed: %s \n",argv[0], strerror(errno));
    }
    if((exitChd2 = waitpid(pid_child2, NULL,0)) == -1) {
        fprintf(stderr, "Error in %s -> waitpid failed: %s \n",argv[0], strerror(errno));
    }
    if((exitChd3 = waitpid(pid_child3, NULL,0)) == -1) {
        fprintf(stderr, "Error in %s -> waitpid failed: %s \n",argv[0], strerror(errno));
    }
    if((exitChd4 = waitpid(pid_child4, NULL,0)) == -1) {
        fprintf(stderr, "Error in %s -> waitpid failed: %s \n",argv[0], strerror(errno));
    }
    
    
    
    //read solutions from childrens outputs in pipe
    char c[4];
    int memory[4];
    memset(memory, 2, sizeof(int) * 4);
    memset(c, 0, sizeof(char) * 4);
   
   
    for(int i=0; i<4; ++i) {
        if((solutionArray[i] = malloc(10*sizeof(char))) == NULL) {
            fprintf(stderr, "Error in %s -> malloc failed: %s \n",argv[0], strerror(errno));
        }
        memset(solutionArray[i], 0, 10);
    }
    
    
    closePipes(1, argv, 1, pipefd_child1_1[1]);
    while(read(pipefd_child1_1[0], &c[0], 1) > 0) {
        char str[2];
        str[1] = '\0';
        str[0] = c[0];
        strcat(solutionArray[0], str);
        if(strlen(solutionArray[0]) == 10*(memory[0]-1)) {
            if((solutionArray[0] = realloc(solutionArray[0], 10*sizeof(char)*memory[0]++)) == NULL) {
                fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
            }
        }
    }
    
    closePipes(1, argv, 1, pipefd_child2_1[1]);
    while(read(pipefd_child2_1[0],&c[1], 1) > 0) {
        char str[2];
        str[1] = '\0';
        str[0] = c[1];
        strcat(solutionArray[1], str);
        if(strlen(solutionArray[1]) == 10*(memory[1]-1)) {
            if((solutionArray[1] = realloc(solutionArray[1], 10*sizeof(char)*memory[1]++)) == NULL) {
                fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
            }
        }
    }
    
    closePipes(1, argv, 1, pipefd_child3_1[1]);
    while(read(pipefd_child3_1[0],&c[2], 1) > 0) {
        char str[2];
        str[1] = '\0';
        str[0] = c[2];
        strcat(solutionArray[2], str);
        if(strlen(solutionArray[2]) == 10*(memory[2]-1)) {
            if((solutionArray[2] = realloc(solutionArray[2], 10*sizeof(char)*memory[2]++)) == NULL) {
                fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
            }
        }
    }
    
    closePipes(1, argv, 1, pipefd_child4_1[1]);
    while(read(pipefd_child4_1[0],&c[3], 1) > 0) {
        char str[2];
        str[1] = '\0';
        str[0] = c[3];
        strcat(solutionArray[3], str);
        if(strlen(solutionArray[3]) == 10*(memory[3]-1)) {
            if((solutionArray[3] = realloc(solutionArray[3], 10*sizeof(char)*memory[3]++)) == NULL) {
                fprintf(stderr, "Error in %s -> realloc failed: %s \n",argv[0], strerror(errno));
            }
        }
    }
    
    
    //close open pipe ends
    closePipes(1, argv, 8, pipefd_child1_1[0], pipefd_child2_1[0], pipefd_child3_1[0], pipefd_child4_1[0],
               pipefd_child1_2[0], pipefd_child2_2[0], pipefd_child3_2[0], pipefd_child4_2[0]);
     
}



//separate hex numbers into two parts.
void divideLines(char* fL, char* fH_1, char* sH_1, char* sL, char* fH_2, char* sH_2) {
    
    //divide first line
    int idx = 0;
    for(int i=0; i<strlen(fL); ++i) {
        if(i<(strlen(fL)/2)) {
            fH_1[i] = fL[i];
        } else {
            sH_1[idx++] = fL[i];
        }
    }
    
    
    //divide second line
    idx = 0;
    for(int i=0; i<strlen(sL); ++i) {
        if(i<(strlen(sL)/2)) {
            fH_2[i] = sL[i];
        } else {
            sH_2[idx++] = sL[i];
        }
    }
}



//convert the hexadecimal number to the corresponding integer 
int hexToNumber(char* hexNumber, int exponent) {
    
    int ascii_val, int_val = 0;
    
    for(int i=strlen(hexNumber)-1; i>=0; --i) {
        ascii_val = (int)hexNumber[i];
        
        //if number in range [0,9]
        if((ascii_val >= 48) && (ascii_val <= 57)) {
            //conversion to corresponding integer value by reducing with the ascii value of '0' (=48)
            //multiply with a power of 16, since converting into hexadecimal requires this.
            int_val += (ascii_val-48) * (int)pow(16,exponent++);
        
        //if number in range [A,F]
        } else {
            //By reduction of 55 here the correct integer value is recieved
            int_val += (ascii_val-55) * (int)pow(16,exponent++);
        }
    }
    
    return int_val;
}

