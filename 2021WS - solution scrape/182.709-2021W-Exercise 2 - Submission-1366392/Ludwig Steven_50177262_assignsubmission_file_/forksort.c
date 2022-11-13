
/**
 * @author Steven Ludwig 11914281
 * @brief A basic line sorting program
 * @details 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

const char* fs = "forksort.c";

/**
 * @brief Close Pipes
 * @param pipe The Pipe we want to close
 */
static void plumber(int pipe[2]) {
    for(int i = 0; i < 2; i++) {
        if(pipe[i] != -1) {
            close(pipe[i]);
            if(errno == -1) fprintf(stderr, "Could not close pipe: %s in %s\n", strerror(errno),fs);
        }
    }
    for(int i = 0; i < 2; i++) {
        pipe[i] = -1;
    }
}

/**
 * @brief Write to pipe diretly
 * @param pipe The pipe we want to write to.
 * @param line inputLine we want to write to pipe.
 */
static void pipeLineGiver(int pipe[2], const char * line) {
    int length = strlen(line);
    if(length > 0) {
        dprintf(pipe[1], "%s", line);
        if(line[length-1] != '\n')
            dprintf(pipe[1], "\n");
    }
}

/**
 * @brief forksort
 *
 * @details Breaks up e.g. input file in Blocks of 2 passes them to a childprocess and sorts them internally. This is passed
 * to the parent which sorts two children who just have sorted their input. 
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns anything but -1 if successfull
 */
int main(int argc, char* argv[]) {

    //maybe get rid of
    static const char *fileName;
    fileName = argv[0];

    if(argc != 1) {
        fprintf(stderr, "Usage: %s\n", fileName);
        return EXIT_FAILURE;
    }

    size_t firstSize = 0;
    char* firstBuffer = NULL;

    ssize_t firstLine;
    if( (firstLine = getline(&firstBuffer, &firstSize, stdin)) == -1) {
        if(errno == 0) {
            free(firstBuffer);
            fprintf(stderr, "Empty stdin\n");
        } else
            fprintf(stderr, "Error while reading first line from stdin: %d: %s in %s\n", errno, strerror(errno),fileName);
        return EXIT_FAILURE;
    }

    size_t secondSize = 0;
    char* secondBuffer = NULL;

    ssize_t secondLine;
    if((secondLine = getline(&secondBuffer, &secondSize, stdin)) == -1) {
        if(errno == 0) {
            fprintf(stdout, "%s", firstBuffer);
            free(firstBuffer);
            free(secondBuffer);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Could not read Line: %d: %s in %s\n", errno, strerror(errno),fileName);
            free(firstBuffer);
            return EXIT_FAILURE;
        }
    }


    int leftChildInPipe[2] = {-1, -1};
    int leftChildOutPipe[2] = {-1, -1};
    int rightChildInPipe[2] = {-1, -1};
    int rightChildOutPipe[2] = {-1, -1};

    if(pipe(leftChildInPipe) == -1 || pipe(leftChildOutPipe) == -1 || pipe(rightChildInPipe) == -1  || pipe(rightChildOutPipe) == -1) {
        fprintf(stderr, "Could not create Pipe: %s in %s\n",  strerror(errno),fileName);
        free(firstBuffer);
        free(secondBuffer);
        plumber(leftChildInPipe);
        plumber(leftChildOutPipe);
        plumber(rightChildInPipe);
        plumber(rightChildOutPipe);
        return EXIT_FAILURE;
    }

    pid_t rightChild = -1;
    pid_t leftChild = fork();

    if(leftChild != 0) {
        if(leftChild != -1)
            rightChild = fork();
        
        if(leftChild == -1 || rightChild == -1) {
            fprintf(stderr, "Could not Fork a child, GoodBye: %d: %s in %s\n", errno, strerror(errno),fileName);
            plumber(leftChildInPipe);
            plumber(leftChildOutPipe);
            plumber(rightChildInPipe);
            plumber(rightChildOutPipe);
            free(firstBuffer);
            free(secondBuffer);
            return EXIT_FAILURE;
        }
    }

    if(leftChild == 0 || rightChild == 0) {

        bool left = false;
        if(leftChild == 0) left = true;
        
        if(left){
            plumber(rightChildInPipe);
        }else {
            plumber(leftChildInPipe);
        }

        if(left){
            plumber(rightChildOutPipe);
        }else {
            plumber(leftChildOutPipe);
        }

        int* inpipe;

        if(left){
            inpipe = leftChildInPipe;
        }else {
            inpipe = rightChildInPipe;
        }


        int* outpipe;

        if(left){
            outpipe = leftChildOutPipe;
        }else {
            outpipe = rightChildOutPipe;
        }
        

        bool running = true;

        if(close(outpipe[0]) == -1) {
            fprintf(stderr, "Error closing pipe output: %sin %s\n", strerror(errno),fileName);
            running = false;
        }

        outpipe[0] = -1;
        // Defensive Programming, close all pipe ends even if we don't use them.
        if(close(inpipe[1]) == -1) {
            fprintf(stderr, "Error closing pipe input: %s in %s\n", strerror(errno),fileName);
            running = false;
        }
        inpipe[1] = -1;

        
        if(dup2(inpipe[0], STDIN_FILENO) == -1) {
            fprintf(stderr, "Error setting input to stdin: %s in %s\n", strerror(errno),fileName);
            running = false;
        }
        if(dup2(outpipe[1], STDOUT_FILENO) == -1) {
            fprintf(stderr, "Error setting output to stdout: %s in %s\n", strerror(errno),fileName);
            running = false;
        }

        free(firstBuffer);
        free(secondBuffer);

        if(running){
            execlp(fileName, argv[0], NULL);}
        else
            return EXIT_FAILURE;
    }

    bool running = true;
    if(close(rightChildInPipe[0]) == -1) {
        fprintf(stderr, "Error closing right child input pipe: %s\n in %s", strerror(errno),fileName);
        running = false;
    }
    rightChildInPipe[0] = -1;

    if(close(rightChildOutPipe[1]) == -1) {
        fprintf(stderr, "Error closing right child output pipe: %s\n in %s", strerror(errno),fileName);
        running = false;
    }
    rightChildOutPipe[1] = -1;

    if(close(leftChildInPipe[0]) == -1) {
        fprintf(stderr, "Error closing left child input pipe: %s\n in %s", strerror(errno),fileName);
        running = false;
    }
    leftChildInPipe[0] = -1;

    if(close(leftChildOutPipe[1]) == -1) {
        fprintf(stderr, "Error closing left child output pipe: %s\n in %s", strerror(errno),fileName);
        running = false;
    }
    leftChildOutPipe[1] = -1;
    
    pipeLineGiver(leftChildInPipe, firstBuffer);
    pipeLineGiver(rightChildInPipe, secondBuffer);
    
    bool left = true;
    while(running) {
        ssize_t length = getline(&firstBuffer, &firstSize, stdin);
        if(length == -1) {
            if(errno == 0) {
                break;
            } else {
                fprintf(stderr, "Error while reading line from stdin: %d: %s in %s \n", errno, strerror(errno),fileName);
                running = false;
                break;
            }
        }

        if(left==0){
            pipeLineGiver(leftChildInPipe, firstBuffer);
        }else{
            pipeLineGiver(rightChildInPipe, firstBuffer);
        }
        
        left = !left;
    }

    if(running) {

        if(close(leftChildInPipe[1]) == -1) {
            fprintf(stderr, "Could not close left input pipe: %s in %s \n", strerror(errno),fileName);
            running = false;
        }
        leftChildInPipe[1] = -1;
        if(close(rightChildInPipe[1]) == -1) {
            fprintf(stderr, "Could not close right input pipe: %s in %s \n", strerror(errno),fileName);
            running = false;
        }
        rightChildInPipe[1] = -1;
    }

    FILE* leftChildOut = NULL;
    FILE* rightChildOut = NULL;
    if(running) {
        leftChildOut = fdopen(leftChildOutPipe[0], "r");
        rightChildOut = fdopen(rightChildOutPipe[0], "r");

        if(leftChildOut == NULL) {
            fprintf(stderr, "Cannot open output Filedescriptor: %d: %s in %s\n", errno, strerror(errno),fileName);
            running = false;
        }        
        leftChildOutPipe[0] = -1;

        if(rightChildOut == NULL) {
            fprintf(stderr, "Failed to open child output for reading: %d: %s in %s\n", errno, strerror(errno),fileName);
            running = false;
        }
        rightChildOutPipe[0] = -1;
    }

    bool hasLeft = true;
    bool hasRight = true;
    if(running) {
        firstLine = getline(&firstBuffer, &firstSize, leftChildOut);
        if(firstLine == -1) {
            if(errno == 0) {
                hasLeft = false;
            } else {
                fprintf(stderr, "Problem reading from frist line: %d: %s\n", errno, strerror(errno));
                running = false;
            }
        }

        secondLine = getline(&secondBuffer, &secondSize, rightChildOut);
        if(secondLine == -1) {
            if(errno == 0) {
                hasRight = false;
            } else {
                fprintf(stderr, "Problem reading from second line: %d: %s in %s\n", errno, strerror(errno),fileName);
                running = false;
            }
        }
    }

    while(running && (hasLeft || hasRight)) {

        bool isleft;

        if(hasLeft && hasRight) {

            isleft = strcmp(firstBuffer, secondBuffer) < 0;

        } else if(hasLeft) {

            isleft = true;

        } else if(hasRight) {

            isleft = false;

        } else {
            running = false;
            break;
        }


        fprintf(stdout, "%s", isleft ? firstBuffer : secondBuffer);


        ssize_t nextLine = getline(isleft ? &firstBuffer : &secondBuffer, isleft ? &firstSize : &secondSize, isleft ? leftChildOut : rightChildOut);
        if(nextLine == -1) {
            if(errno == 0) {
                if(isleft)
                    hasLeft = false;
                else
                    hasRight = false;
            } else {
                running = false;
                fprintf(stderr, "%s Could not read Line in %s\n", strerror(errno),fileName);
                break;
            }
        }

        if(isleft)
            firstLine = nextLine;
        else
            secondLine = nextLine;
    }

    if(leftChildOut != NULL) {
        fclose(leftChildOut);
        leftChildOut = NULL;
    }

    if(rightChildOut != NULL) {
        fclose(rightChildOut);
        rightChildOut = NULL;
    }

    bool isLeftRunning = false;
    bool isRighttRunning = false;
    while(running && (!isLeftRunning || !isRighttRunning)) {
        int status;
        pid_t childWait = wait(&status);
        if(childWait == -1) {
            if(errno == EINTR)
                continue;
            fprintf(stderr, "Something happend while waiting for children to finish: %d: %s in %s\n", errno, strerror(errno),fileName);
            running = false;
            break;
        }

        if(childWait == leftChild)
            isLeftRunning = true;
        else if(childWait == rightChild)
            isRighttRunning = true;
        else {
            running = false;
            break;
        }

        if(WEXITSTATUS(status) != EXIT_SUCCESS || !WIFEXITED(status)) {
            fprintf(stderr, "Some Child was killed, i am sorry.\n");
            running = false;
            break;
        }
    }

    plumber(leftChildInPipe);
    plumber(rightChildInPipe);
    plumber(leftChildOutPipe);
    plumber(rightChildOutPipe);
    
    free(firstBuffer);
    free(secondBuffer);

    if(running){
        return EXIT_SUCCESS;
    }else{
        return EXIT_FAILURE;
    }
}

