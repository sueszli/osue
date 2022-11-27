/**
 * @file forkFFT.c
 * @author Kieffer Joé <e11814254@student.tuwien.ac.at>
 * @date 11.12.2021
 * 
 * @brief Main program module
 * 
 * This program implements the Cooley-Turkey Fast Fourier Transform algorithm with forking and recursively calling itself.
 * This program reads lines fork itself if more than 2 are read and splits the values read in even and odd lines and gives
 * the even lines and the odd lines to a new instance of itself, then when the values comes back from the children a calculation
 * is made and the lines are outputted.
 */

#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PI (3.141592654)

char *progName;

/**
 * @brief Method to close pipe end
 * 
 * @details This method gets a pipe and the value of which end to close and closes
 * the end of the pipe which is whished by end. If an error occurs a suitable message is printed.
 * 
 * @param end the ned to close
 * @param pipefd the pipe where one end should be closed
 */
void closePipeEnd(int end, int pipefd[]){
    assert(end == 0 || end == 1);
    int success = close(pipefd[end]);
    if(success == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't close pipe end! close failed: %s\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    } 
}

/**
 * @brief Method to close a filestream
 * 
 * @details This method closes a given filestream. If an error occurs a suitable message is printed.
 * 
 * @param file a file pointer to the filestream to close
 */
void closeFile(FILE *file){
    int success = fclose(file);
    if(success == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't close filestream! fclose failed: %s\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Method to open a filestream
 * 
 * @details This Method opens a filestream with the given filedescriptor and the rights of access (r or w) and returns a pointer to
 * the stream. If an error occurs a suitable message is printed.
 * 
 * @param fd the filedescriptor to open
 * @param access the accessrights needed of the filestream
 * @return Returns a filepointer to the opened filestream
 */
FILE *openFilePointer(int fd, char *access){
    assert(strcmp(access, "r") == 0 || strcmp(access, "w") == 0);
    FILE *file = fdopen(fd, access);
    if(file == NULL){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't open filedescriptior! fdopen failed: %s\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return file;
}

/**
 * @brief This method reads from the childprocesses, calculates the Cooley-Tukey Fast Fourier Transform and prints the results
 * 
 * @details This method gets to pipes to read from, one from child with the even lines one from the one with the odd ones. First the method opens filestreams with the pipes.
 *  Than the method goes in a loop and remains in it as long as the children send lines. The loop always reads one line from each children at ones, then transform the
 * strings to floats (2 per line imaginary and real part). Then the calcultion is made, 2 values are generated, one is printed instantly and the
 * second one is for after every child has send all the lines. If finished the functions to close the filestreams are called.
 * 
 * @param pipeToParentE pipe where the first child writes to
 * @param pipeToParentO pipe where the second child writes to
 * @param nHalf number of lines each child should have
 */
void readFromChildren(int pipeToParentE[], int pipeToParentO[], int nHalf){
    FILE *fromEvenChild = openFilePointer(pipeToParentE[0], "r");
    FILE *fromOddChild = openFilePointer(pipeToParentO[0], "r");

    float secondHalf[nHalf][2];
    char *line1 = NULL;
    char *line2 = NULL;
    size_t len1 = 0;
    size_t len2 = 0;
    ssize_t nread1 = 0;
    ssize_t nread2 = 0;
    int k = 0;
    int n = 2*nHalf;
    int stop = 0;
    while(stop==0){
        nread1 = getline(&line1, &len1, fromEvenChild);
        nread2 = getline(&line2, &len2, fromOddChild);
        if(nread1 <= 0 && nread2 <= 0){
            stop = 1;
        }else if(nread1 > 0 && nread2 > 0){      
            char *imaginary;
            
            float rEReal = strtof(line1, &imaginary);
            float rEImaginary = strtof(imaginary, NULL);
            
            float rOReal = strtof(line2, &imaginary); 
            float rOImaginary = strtof(imaginary, NULL);

            float a = cos(-((2*PI)/n)*k);
            float b = sin(-((2*PI)/n)*k);

            float real = (a * rOReal) - (b * rOImaginary);
            float ima = (a * rOImaginary) + (b* rOReal); 

            float resultReal = rEReal + real;
            float resultIma = rEImaginary + ima;

            fprintf(stdout, "%f %f*i\n", resultReal, resultIma);
            secondHalf[k][0] = rEReal - real;
            secondHalf[k++][1] = rEImaginary - ima;
        }else{
            fprintf(stderr, "[%s:%i] ERROR: Could not read 2 to the power of n numbers with n >= 0\n", progName, __LINE__);
            exit(EXIT_FAILURE);
        }
    }
    if(k != nHalf){
        fprintf(stderr, "[%s:%i] ERROR: Could not calculate all numbers\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
    for(int i=0; i < nHalf; i++){
        fprintf(stdout, "%f %f*i\n", secondHalf[i][0], secondHalf[i][1]);
    }
    closeFile(fromEvenChild);
    closeFile(fromOddChild);
}

/**
 * @brief Method which writes the recived lines to the children
 * 
 * @details This method gets 2 pipes to write to 2 children and 2 lines which are already read at the beginning to determine if
 * the process needed to get fork. First this method opens the pipes to write to the children with the needed accessrights. Than
 * the first 2 lines are written to the children one to each. After that the method remains in a loop as long as new inputs are
 * recived they are written alternating to the children. After no more input is recived the method checks if both children got
 * the same amount of values, if so the filestreams are closed else an error message gets displayed.
 * 
 * @param pipeToChildE pipe to the child for the even lines
 * @param pipeToChildO pipe to the child for the odd lines
 * @param line1 first read line
 * @param line2 second read line
 * @return returns the number of lines written to one child
 */
int writeToChildren(int pipeToChildE[], int pipeToChildO[], char *line1, char *line2){
    FILE *toEvenChild = openFilePointer(pipeToChildE[1], "w");
    FILE *toOddChild = openFilePointer(pipeToChildO[1], "w");

    fprintf(toEvenChild, "%s", line1);
    int e = 1;
    fprintf(toOddChild, "%s", line2);
    int o = 1;
    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, stdin) != -1){
        if(e > o){
            fprintf(toOddChild, "%s", line);
            o++;
        }else{
            fprintf(toEvenChild, "%s", line);
            e++;
        }
    }
    if(e != o){
        fprintf(stderr, "[%s:%i] ERROR: Please enter 2 to the power of n numbers with n>=0!\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
    closeFile(toEvenChild);
    closeFile(toOddChild);
    return e;
}

/**
 * @brief Method to wait on child processes
 * 
 * @details This method gets the process id of the childprocesses and waits that the children terminates.
 * If both terminate successfully this method ends this program successfully else this method ends this
 * program with an error.
 * 
 * @param child1 the process id of the first child
 * @param child2 the process id of the second child
 */
void waitOnChildren(pid_t child1, pid_t child2){
    int status1;
    int status2;
    pid_t pid;
    while((pid = waitpid(child1, &status1, 0)!= child1)){
        if(pid == -1){
            continue;
        }
        if(errno == EINTR){
            continue;
        }
        fprintf(stderr, "[%s:%i] ERROR, Cannot wait\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
    while((pid = waitpid(child2, &status2, 0)!= child2)){
        if(pid == -1){
            continue;
        }
        if(errno == EINTR){
            continue;
        }
        fprintf(stderr, "[%s:%i] ERROR, Cannot wait\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
    if(WEXITSTATUS(status1) == EXIT_SUCCESS && WEXITSTATUS(status2) == EXIT_SUCCESS){
        exit(EXIT_SUCCESS);
    }else{
        fprintf(stderr, "[%s:%i] ERROR: At least one child hasn't terminate properly!\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This method closes the unneeded pipe ends for the children
 * 
 * @details This method calls the function to close the write ends of the pipes to the children
 * and the read ends of the pipes to the parents. This method should only be called by child processes.
 * 
 * @param pipeToParentE pipe to the parent for the first child
 * @param pipeToChildE pipe to write to the first child
 * @param pipeToParentO pipe to the parent for the second child
 * @param pipeToChildO pipe to write to the second child
 */
void closePipesForChild(int pipeToParentE[], int pipeToChildE[], int pipeToParentO[], int pipeToChildO[]){
    closePipeEnd(1, pipeToChildE);
    closePipeEnd(0, pipeToParentE);
    closePipeEnd(1, pipeToChildO);
    closePipeEnd(0, pipeToParentO);
}

/**
 * @brief This method forks the process into 3 processes (1 parent + 2 child)
 * 
 * @details This method forks the process first in 2 processes and then let the parent process call this method again and so forks the 2 child.
 * After forking this method calls the function to close unneeded pipeends. Then in the child processes this method redirects stdin and stdout
 * and then calls this program to execute from the beginning. In the parent process the function is called to write to the children and after
 * thar to read from them. If finished reading the function is called to wait on the child proccesses. This function also returns the process id
 * given by fork.
 * 
 * 
 * @param forkNr variable to know how ofter the program is called and to stop endless recursion 
 * @param pipeToParentE pipe for the first child to write to the parent process
 * @param pipeToChildE pipe to write to the first child
 * @param pipeToParentO pipe for the second child to write to the parent process
 * @param pipeToChildO pipe to write to the second child
 * @param line1 first line read from the process
 * @param line2 second line read from the process
 * @return pid_t the process id returned by fork
 */
pid_t forkMethod(int forkNr, int pipeToParentE[], int pipeToChildE[], int pipeToParentO[], int pipeToChildO[], char *line1, char *line2){
    if(forkNr > 1){
        assert(0);
        return -1;
    }
    pid_t childPid = fork();
    switch (childPid) {
        case -1:
            fprintf(stderr, "[%s:%i] ERROR: Cannot fork!\n", progName, __LINE__);
            exit(EXIT_FAILURE);
        case 0:
            //child
            closePipesForChild(pipeToParentE, pipeToChildE, pipeToParentO, pipeToChildO);
            if(forkNr == 0){
                dup2(pipeToChildE[0], STDIN_FILENO);
                dup2(pipeToParentE[1], STDOUT_FILENO);
            }else{
                assert(forkNr == 1);
                dup2(pipeToChildO[0], STDIN_FILENO);
                dup2(pipeToParentO[1], STDOUT_FILENO);
                
            }
            execlp("./forkFFT", "./forkFFT", NULL);
            return childPid;
        default:
            //parent
            if(forkNr == 0){
                pid_t otherChildPid = forkMethod(1, pipeToParentE, pipeToChildE, pipeToParentO, pipeToChildO, line1, line2);
                closePipeEnd(0, pipeToChildE);
                closePipeEnd(1, pipeToParentE);
                closePipeEnd(0, pipeToChildO);
                closePipeEnd(1, pipeToParentO);    
                int nHalf = writeToChildren(pipeToChildE, pipeToChildO, line1, line2);
                readFromChildren(pipeToParentE, pipeToParentO, nHalf);
                waitOnChildren(childPid, otherChildPid);
            }
            return childPid;
    }
    return childPid;
}

/**
 * @brief This method creates a pipe
 * 
 * @details This method creates a pipe out of the given int array. If an error occurs a suitable errormessage is printed out.
 * 
 * @param pipefd the array where the pipe should be created
 */
void openPipe(int pipefd[]){
    int success = pipe(pipefd);
    if(success == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't create pipe! pipe failed: %s\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    } 
}

/**
 * @brief This method creates a and opens the needed pipes of this program
 * 
 * @details This method gets the 2 lines already read by the process. This method creates
 * new arrays and calls the function to make pipes out of them. After that this method
 * calls the function to fork this process.
 * 
 * @param line1 first line read by the process
 * @param line2 second line read by the process
 */
void createAndOpenPipes(char *line1, char *line2){
    int pipeToChildE[2];
    int pipeToParentE[2];

    int pipeToChildO[2];
    int pipeToParentO[2];
    
    openPipe(pipeToChildE);
    openPipe(pipeToParentE);
    openPipe(pipeToChildO);
    openPipe(pipeToParentO);
    forkMethod(0, pipeToParentE, pipeToChildE, pipeToParentO, pipeToChildO, line1, line2);
}

/**
 * @brief Main method
 * 
 * @details The program starts here. This method first tries to read 2 lines. If no line is
 * read a error message is displayes, if only one line is entered this method prints the line.
 * If 2 lines are read this method calls the function to create and open pipes.
 * 
 * @param argc the argument counter
 * @param argv the given arguments
 * @return EXIT_SUCCESS or EXIT_FAILURE depending how the program ends.
 */
int main(int argc, char *argv[]){
    progName = argv[0];
    char *line1 = NULL;
    char *line2 = NULL;
    size_t len1 = 0;
    size_t len2 = 0;
    ssize_t nread1 = getline(&line1, &len1, stdin);
    ssize_t nread2 = getline(&line2, &len2, stdin);
    if(nread1 <= 0){
        fprintf(stderr, "[%s:%i] ERROR: Empty line detected!\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }else if(nread2 <=0){
        fprintf(stdout, "%s" ,line1);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }else{
        createAndOpenPipes(line1, line2);
    }
    exit(EXIT_SUCCESS);
}
