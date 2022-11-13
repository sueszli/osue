/**
 * @file forksort.c
 * @author Christoph Neubauer 12023172
 * @date 2021-11-21
 */
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdbool.h>

char *myProg;

typedef struct
{
    char **lines;
    int countLines;
    int maxLineSize;

} Input;

/**
 * @brief prints the error Message and exits with Failure
 * 
 * @param errMsg Error Message that is printed
 */
static void printErrAndExit(char* errMsg)
{
    if(errno == 0){
        fprintf(stderr, "%s: %s\n", myProg,errMsg);
    }
    else{
    fprintf(stderr, "%s: %s: %s", myProg,errMsg,strerror(errno));
    }
    exit(EXIT_FAILURE);
}

/**
 * @brief prints the given error message
 * 
 * @param errMsg Error message that is printed
 */
static void printErr(char* errMsg)
{
    if(errno == 0){
        fprintf(stderr, "%s: %s\n", myProg,errMsg);
    }
    else{
    fprintf(stderr, "%s: %s: %s\n", myProg,errMsg,strerror(errno));
    }
}

/**
 * @brief frees all memory stored in the struct 'input'
 * 
 * @param input struct where all input data is stored
 */
static void freeMem(Input *input)
{
    for(int i= 0;i< input->countLines;i++){
         free(input->lines[i]);
    }
    free(input->lines);
}

/**
 * @brief all strings stored in input are streched to the same length, the maximum length of a string in the array. 
 *        First the '\n is cut away, then all strings are filled with the needed number of spaces 
 * @param input struct where all input data is stored
 */
static void parseStrings(Input *input)
{
    for(int i = 0; i< input->countLines; i++){
        input->lines[i][strcspn(input->lines[i], "\n")] = 0;
        int len = strlen(input->lines[i]);
        int spaces = input->maxLineSize - 1 - len;
        while(spaces>0){
            strcat(input->lines[i]," ");
            spaces--;
        }
        strcat(input->lines[i],"\n");
    }
}

/**
 * @brief reads all lines from stdin and saves them to the 'input' struct. stops reading if it reaches
 * 
 * @param input struct where the read lines are stored in
 */
static void readInput(Input *input){

    int capacity = 10;
    char *currLine = NULL;
    size_t lineSize = 0; // is automatically re-allocated by getline
    size_t chars;

    input->lines = malloc(capacity*sizeof(char*));

    if(input->lines == NULL){
        freeMem(input);
        printErrAndExit("lines-array memory couldn't be reallocated");
    }

    // read as long as there is no EOF
    while((chars=getline(&currLine,&lineSize,stdin)) != -1)
    {
        if(chars <2){ // this line only has '\n' or nothing
            continue;
        }
        if(input->maxLineSize < strlen(currLine)){
            input->maxLineSize = strlen(currLine);
        }
        input->lines[input->countLines] = currLine;
        lineSize = 0;
        currLine = NULL;
        (input->countLines)++;
        // more than 'capacity' lines have to be read
        if(input->countLines == capacity){
            int newCap = capacity +10;
            char **newLines = realloc(input->lines,newCap * sizeof(char*));
            if(newLines == NULL){
                // free all memory
                free(currLine);
                freeMem(input);
                printErrAndExit("lines-array memory couldn't be reallocated");
            }
            input->lines = newLines;
            capacity = newCap;
        }
    }
    free(currLine);
    
    parseStrings(input);

}

/**
 * @brief frees the memory of the two given strings
 * 
 * @param lineOne string to be freed 
 * @param lineTwo string to be freed
 */
static void closeMerge(char* lineOne,char* lineTwo)
{
    free(lineOne);
    free(lineTwo);
}

/**
 * @brief writes the given line to stdout and then reads a line from the given pipe
 * 
 * @param input struct where the maximum line size for the read is stored
 * @param readPipe pipe where data should be read from
 * @param line string that is first written to stdout, then reads a new line from the pipe
 * @return int returns -1 if an error occurred, returns 0 otherwise
 */
static int writeReadFromPipe(Input *input, int readPipe, char *line)
{

    if(write(STDOUT_FILENO,line,strlen(line)) <0){
        printErr("couldnt write to parent");
        return -1;
    }
    // read the next line from first child
    if((read(readPipe,line,input->maxLineSize)) <0){
        printErr("getline from pipe failed");
        return -1;
    }
    return 0;
}

/**
 * @brief reads 'input->countLines' lines from the two children, merges the read lines and writtes them lexicographically to stdout
 * 
 * @param input struct that stores the number of lines to read and the line size of all strings
 * @param readFromFirstChild pipe to read from first child
 * @param readFromSecondChild pipe to read from second child
 * @return int returns -1 if an error occurred, returns 0 otherwise
 */
static int merge(Input *input,int readFromFirstChild, int readFromSecondChild)
{
    int leftMax = input->countLines/2;
    int rightMax = input->countLines - leftMax;
    char* lineOne , *lineTwo ;

    lineOne =malloc((input->maxLineSize) * sizeof(char));
    lineTwo =malloc((input->maxLineSize) * sizeof(char));

    if(lineOne==NULL || lineTwo == NULL){
        printErr("couldn't alloc memory for reading from child");
        return -1;
    }

    // read first line from first child
    if((read(readFromFirstChild,lineOne,input->maxLineSize)) <0){
        closeMerge(lineOne,lineTwo);
        printErr("getline 1 from pipefile failed");
        return -1;
    }
    int left =1;

    // read first line from second child
    if((read(readFromSecondChild,lineTwo,input->maxLineSize)) <0){
        closeMerge(lineOne,lineTwo);
        printErr("getline 2 from pipefile failed");
        return -1;
    }
    int right =1;
    

    while(left<=leftMax && right <=rightMax)
    {    // take the line from first child
        if((strcmp(lineOne,lineTwo))<0){
            if(writeReadFromPipe(input,readFromFirstChild,lineOne)<0){
                closeMerge(lineOne,lineTwo);
                return -1;
            }
            left++;
        }
        else{
            // take the line from second child
            if(writeReadFromPipe(input,readFromSecondChild,lineTwo)<0){
                closeMerge(lineOne,lineTwo);
                return -1;
            }
            right++;
        }
    }

    // already read all from second child but still some from first child missing
    while(left<=leftMax){
        if(writeReadFromPipe(input,readFromFirstChild,lineOne)<0){
            closeMerge(lineOne,lineTwo);
            return -1;
        }
        left++;
    }
    // already read all from first child but still some from second child missing
    while(right<=rightMax){
        if(writeReadFromPipe(input,readFromSecondChild,lineTwo)<0){
            closeMerge(lineOne,lineTwo);
            return -1;
        }
        right++;
    }

    closeMerge(lineOne,lineTwo);
    close(readFromFirstChild);
    close(readFromSecondChild);
    return 0;

}

/**
 * @brief after splitting up into parent and children, this function does the parent part. It writes data to the children and
 *        then merges the provided data lexicographically and waits for all children to terminate
 * 
 * @param input struct that stores all data that is sent to children
 * @param writeToFirstChild pipe to write to the first child
 * @param readFromFirstChild pipe to read from the first child
 * @param writeToSecChild  pipe to write to the second child
 * @param readFromSecondChild pipe to read from the second child
 * @return int returns -1 if an error occurred, returns 0 otherwise
 */
static int continueAsParent(Input *input,int writeToFirstChild, int readFromFirstChild, int writeToSecChild, int readFromSecondChild)
{

    // write first half to second child
    for(int i= 0; i<((input->countLines)/2);i++){
        if(write(writeToFirstChild,input->lines[i],strlen(input->lines[i])) <0){
            printErr("couldnt write to first child");
            return -1;
        }
    }
    if(close(writeToFirstChild)<0){
        printErr("Close failed");
        return -1;
    }

    // write second half to second child
    for(int i= ((input->countLines)/2); i<input->countLines;i++){
        if(write(writeToSecChild,input->lines[i],strlen(input->lines[i])) <0){
            printErr("couldnt write to second child");
            return -1;
        }
    }
    if(close(writeToSecChild)<0){
        printErr("Close failed");
        return -1;
    }

    fflush(stdout);

    if(merge(input,readFromFirstChild,readFromSecondChild) <0){
        printErr("Merge failed");
        return -1;
    }

    int status;
    // wait for all kids to finish
    while(wait(&status) != -1){

        if(errno == EINTR) continue;

        // if any child had an error and exited
        if(WEXITSTATUS(status) != EXIT_SUCCESS){
            printErr("Child had an error");
            return -1;
        }
    }
    return 0;
}

/**
 * @brief fork the program, close unused pipe ends and exec the child
 * 
 * @param input struct that stores all data, only needed for freeing memory in the child process before exec
 * @param childReadPipe pipe where the child reads from
 * @param childWritePipe  pipe where the child writes to
 * @return int returns -1 if an error occurred, returns 0 otherwise 
 */
static int forkChild(Input *input, int *childReadPipe,int *childWritePipe)
{
    fflush(stdout);
    fflush(stderr);
    pid_t childID;
    childID = fork();
    switch(childID){
        case -1:
            printErr("Fork 1 failed");
            return-1;
            
        case 0: //child
            
            if((close(childReadPipe[1]) != 0) || (close(childWritePipe[0]) != 0)){
                printErr("Close failed");
                return -1;
            }

            
            // child reads from stdin and writes to stdout -> over pipes
            if((dup2(childWritePipe[1], STDOUT_FILENO) == -1) || (dup2(childReadPipe[0], STDIN_FILENO) == -1)){
                printErr("dup2 failed");
                return -1;
            }
            
            close(childWritePipe[1]);
            close(childWritePipe[0]);
            freeMem(input);
            execlp(myProg,myProg,NULL);
            // after exec nothing should work here
            printErr("Exec failed");
            return -1;
        default: // parent
            // close unused pipe ends before forking another time
            if((close(childReadPipe[0]) != 0) || (close(childWritePipe[1]) != 0)){
                printErr("Close failed");
                return -1;
            }
            break;
    }  
    return 0;
}

/**
 * @brief reads lines from stdin, creates children if needed and mergesorts the read lines
 * 
 * @param argc number of input arguments
 * @param argv given arguments
 * @return int return EXIT_SUCCESS if everything works, EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
    myProg = argv[0];

    if(argc > 1){
        fprintf(stdout,"usage: %s",myProg);
        exit(EXIT_FAILURE);
    }
    
    Input input= { .countLines=0, .maxLineSize=0};

    readInput(&input);

    // this is a error case and should never happen
    if(input.countLines == 0){
        freeMem(&input);
        printErrAndExit("anzahl = 0");
        return EXIT_FAILURE;
    }

    // only one line -> return it
    if(input.countLines == 1){
        if(write(STDOUT_FILENO,input.lines[0],input.maxLineSize)<0){
            freeMem(&input);
            printErrAndExit("couldn't write to parent");
        }
        return EXIT_SUCCESS;
    }
    // pipe[0] is read end, pipe[1] is write end
    int firstChildReadPipe[2];  //this pipe is only for parent to child transfer
    int firstChildWritePipe[2]; //this pipe is only for child to parent transfer
    if((pipe(firstChildReadPipe) != 0) || (pipe(firstChildWritePipe) != 0)){
        freeMem(&input);
        printErrAndExit("pipe 1 failed");
    }
    
    // create first child
    if(forkChild(&input,firstChildReadPipe,firstChildWritePipe)<0){
        freeMem(&input);
        exit(EXIT_FAILURE);
    }
    
    int secondChildReadPipe[2]; //this pipe is only for parent to child transfer
    int secondChildWritePipe[2]; //this pipe is only for child to parent transfer

    if((pipe(secondChildReadPipe) != 0) || (pipe(secondChildWritePipe) != 0)){
        freeMem(&input);
        printErrAndExit("pipe 2 failed");
    }

    // create second child
    if(forkChild(&input,secondChildReadPipe,secondChildWritePipe)<0){
        freeMem(&input);
        exit(EXIT_FAILURE);
    }

    if(continueAsParent(&input,firstChildReadPipe[1],firstChildWritePipe[0],secondChildReadPipe[1],secondChildWritePipe[0]) <0){
        freeMem(&input);
        exit(EXIT_FAILURE);
    }

    freeMem(&input);
    return EXIT_SUCCESS;
}