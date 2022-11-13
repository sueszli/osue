/**
 * @file forksort.c
 * @author Michael Zischg <Matriculation Number: 12024010>
 * @date 2021-12-07
 * 
 * @brief forksort takes Lines as input and sorts them alphabetically
 * @details Each line is read separately and given to child process. Process is repeated until only one line es left. Lines are 
 * then returned to parent where they are linearly sorted. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>
#include <string.h>

int READ = 0; /**< Constant for reading in pipe. */
int WRITE = 1; /**< Constant for writing in pipe. */
char *prg_name; /**< The program name. */
pid_t actPid; /**< The program pid. */
size_t zero = 0; /**< Constant for a zero value, used in getline. */

int tree = 0; /**< Constant for output tree. */

/**
 * @brief a child consists of input and output pipes, an input and outpur file and its pid
 */
typedef struct {
    pid_t pid;
    int input[2];
    FILE *inputFile;
    int output[2];
    FILE *outputFile;
} child; 

/**
 * @brief Usage Message
 * @details Returns how program should be called (./forksort)
 * @param void
 * @return void
 */
static void usage() { 
    fprintf(stderr,"[%s]: ./%s",prg_name,prg_name);
}

/**
 * @brief Initalizes a child process
 * @details for a given child pointer, pipes its input and output and forks to a child and parent process. 
 * Corresponding pipes are closed, input is redirected to STDIN_FILENO and output to STDOUT_FILENO.
 * @param childPtr pointer to a child
 * @return void
 */
void initChild(child *childPtr) {
    if(pipe(childPtr->input) == -1) {
        fprintf(stderr,"[%s:%ld] Error opening pipe\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(pipe(childPtr->output) == -1) {
        fprintf(stderr,"[%s:%ld] Error opening pipe\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }

    childPtr->pid = fork();
    if(childPtr->pid < 0) {
        fprintf(stderr,"[%s:%ld] Error creating fork\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(childPtr->pid == 0) { //begin child 
        if(close(childPtr->input[WRITE]) == -1) {
            fprintf(stderr,"[%s:%ld] Error closing READ for input of child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }
        if(dup2(childPtr->input[READ], STDIN_FILENO) == -1) {
            fprintf(stderr,"[%s:%ld] Error redirecting stdin to child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }
        if(close(childPtr->input[READ]) == -1) {
            fprintf(stderr,"[%s:%ld] Error closing READ for input of child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }

        if(close(childPtr->output[READ]) == -1) {
            fprintf(stderr,"[%s:%ld] Error closing READ for input of child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }
        if(dup2(childPtr->output[WRITE], STDOUT_FILENO) == -1) {
            fprintf(stderr,"[%s:%ld] Error redirecting stdout to child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }
        if(close(childPtr->output[WRITE]) == -1) {
            fprintf(stderr,"[%s:%ld] Error closing WRITE for output of child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }

        if(execlp(prg_name,prg_name,NULL) == -1) {
            fprintf(stderr,"[%s:%ld] Error calling program for child\n",prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }
        //End of child
    }

    //parent part - close remaining pipes
    if(close(childPtr->input[READ]) == -1) {
        fprintf(stderr,"[%s:%ld] Error closing READ for input in parent\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(close(childPtr->output[WRITE]) == -1) {
        fprintf(stderr,"[%s:%ld] Error closing WRITE for output in parent\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Reads all lines from both children and merges them alphabetically
 * @details uses getline on the outputfiles of the children and prints them alphabetically to stdout
 * @param child1 pointer to first child
 * @param child2 pointer to second child
 * @return void
 */
void readCompareChildren(child *child1, child *child2) {
    char *line1 = NULL,
         *line2 = NULL;

    size_t size1 = getline(&line1, &zero, child1->outputFile),
           size2 = getline(&line2, &zero, child2->outputFile);

    if(tree == 0) {
        while (size1 != -1 && size2 != -1 && strcmp(line1, "\n") != 0 && strcmp(line2, "\n") != 0) {
            if(strcmp(line1, line2) < 0) {
                fprintf(stdout, "%s", line1);
                size1 = getline(&line1, &zero, child1->outputFile);
            } else {
                fprintf(stdout, "%s", line2);
                size2 = getline(&line2, &zero, child2->outputFile);
            }
        } 
        
        while(size1 != -1 && strcmp(line1, "\n") != 0) {
            fprintf(stdout, "%s", line1);
            size1 = getline(&line1, &zero, child1->outputFile);
        }
        while(size2 != -1 && strcmp(line2, "\n") != 0) {
            fprintf(stdout, "%s", line2);
            size2 = getline(&line2, &zero, child2->outputFile);
        }
    } else {
        int sizeTotal = sizeof(char);
        char *output = malloc(sizeTotal);
        if(output == NULL) {
            fprintf(stderr,"[%s:%ld] Error allocating memory", prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }
        line1[size1-1] = ' ';
        sizeTotal += size1 + size2;
        output = realloc(output, sizeTotal);
        if(output == NULL) {
            fprintf(stderr,"[%s:%ld] Error reallocating memory", prg_name, (long) actPid);
            exit(EXIT_FAILURE);
        }

        strcat(output,line1);
        strcat(output,line2);

        while((size1 = getline(&line1, &zero, child1->outputFile)) != -1) {
            size2 = getline(&line2, &zero, child2->outputFile);

            line1[size1-1] = ' ';
            sizeTotal += size1 + size2;
            output = realloc(output, sizeTotal);
            if(output == NULL) {
                fprintf(stderr,"[%s:%ld] Error reallocating memory", prg_name, (long) actPid);
                exit(EXIT_FAILURE);
            }

            strcat(output,line1);
            strcat(output,line2);
        }

        char *pad = malloc(strlen(output)/8);
        for (int i = 0; i < strlen(output)/8; i++) {
            strcat(pad," "); 
        }

        fprintf(stdout, "%sMAX(%s)%s\n",pad,output,pad);
    }

    free(line1);
    free(line2);
}

/**
 * @brief Waits for children processes
 * @details with waitpid checks accordingly if the children terminated correctly
 * @param child pointer to child Process
 * @return void
 */
void waitChildren(child *child) {
    int status;
    while(waitpid(child->pid, &status, 0) < 0) {
        if (errno == EINTR) continue;

        fprintf(stderr, "[%s:%ld] Waiting for child failed\n", prg_name, (long) actPid);
        fprintf(stderr, "%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }  
    if(status < 0) {
        fprintf(stderr, "[%s:%ld] Exiting of child failed\n", prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief main branch of Program
 * @details Sets program name and pid, checks if stdin consists of one line, if so returns it to stdout. 
 * If not two children are opened and lines are alternatingly written to both. Then lines are again read from the 
 * output files of the children, merged together alphabetically, children and program are terminated.
 * @param argc argument counter
 * @param argv pointer to argument array
 * @return sucess
 */
int main(int argc, char *argv[]) {
    child child1;
    child child2;
    prg_name = argv[0];
    actPid = getpid();

    if(argc != 1) {
        usage();
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    char temp;
    ssize_t i = getline(&line, &zero, stdin);
    if(i == -1 || (temp = fgetc(stdin)) == EOF) { //only one line
        if(tree == 0) fprintf(stdout,"%s\n",line);
        else fprintf(stdout," %s   ",line);
        free(line);
        exit(EXIT_SUCCESS);
    }
    ungetc(temp, stdin);
    if(line[i-1] == '\n') line[--i] = '\0';

    initChild(&child1);
    initChild(&child2);

    child1.inputFile = fdopen(child1.input[WRITE],"w");
    child2.inputFile = fdopen(child2.input[WRITE],"w");

    if(child1.inputFile == NULL) {
        fprintf(stderr, "[%s:%ld] Error opening input file for first child.\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(child2.inputFile == NULL) {
        fprintf(stderr, "[%s:%ld] Error opening input file for second child.\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }

    short toggle = 0;

    if(line[i-1] == '\n') line[--i] = '\0';
    fprintf((&child1)->inputFile,"%s\n",line); //write first line

    while((i = getline(&line, &zero, stdin)) != -1) {
        if(line[i-1] == '\n') line[--i] = '\0';
        fprintf(toggle ? (&child1)->inputFile : (&child2)->inputFile,"%s\n",line);
        toggle = !toggle;
    }

    if(fclose(child1.inputFile) == EOF) {
        fprintf(stderr,"[%s:%ld] Error closing input File for first Child\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(fclose(child2.inputFile) == EOF) {
        fprintf(stderr,"[%s:%ld] Error closing input File for second Child\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    free(line);

    child1.outputFile = fdopen(child1.output[READ],"r");
    child2.outputFile = fdopen(child2.output[READ],"r");

    if(child1.inputFile == NULL) {
        fprintf(stderr, "[%s:%ld] Error opening output file for first child.\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(child2.inputFile == NULL) {
        fprintf(stderr, "[%s:%ld] Error opening output file for second child.\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }

    readCompareChildren(&child1, &child2);

    if(fclose(child1.outputFile) == EOF) {
        fprintf(stderr,"[%s:%ld] Error closing output File for first Child\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }
    if(fclose(child2.outputFile) == EOF) {
        fprintf(stderr,"[%s:%ld] Error closing output File for second Child\n",prg_name, (long) actPid);
        exit(EXIT_FAILURE);
    }

    waitChildren(&child1);
    waitChildren(&child2);
    exit(EXIT_SUCCESS);
}