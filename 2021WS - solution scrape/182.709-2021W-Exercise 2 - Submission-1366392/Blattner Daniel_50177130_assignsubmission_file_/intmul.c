/**
 * @file intmul.c
 * @author Daniel Blattner <e12020646@students.tuwien.ac.at>
 * @date 24.11.2021
 *
 * @brief Calculate the multiplication of two hex numbers with same even lenght.
 *
 * The calculation is done by splitting each number in half and creating child 
 * processes to calculate the smaller part solution. The child process sends the
 * solution to its parent. The parent adds them togehter and the solution is printed
 * to stdout. 
**/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <sys/wait.h>

/** Name of the programm as defined in argv[0] */
static char *progName;

/**
 * @struct Mult
 * @brief This struct stores two numbers and their lenght.
 */
typedef struct{
    /** The lenght of both numbers */
    ssize_t nSize;
    /** The first number */
    char *fInt;
    /** The second number */
    char *sInt;
}Mult;

/**
 * @struct PartRes
 * @brief This struct has a child solution and its size.
 */
typedef struct{
    /** The lenght of the result */
    ssize_t size;
    /** The result */
    char *res;
}PartRes;

/**
 * @brief This function prints an error message to stderr and exit the
 * programm with an EXIT_FAILURE.
 * 
 * @param msg The error message which is printed.
 */
static void printErrorMsgExit(char *msg)
{
    assert(msg != NULL);

    //Print error message and exit
    fprintf(stderr, "[%s] ERROR: %s: %s\n", progName, msg, strerror(errno));
    fflush(stderr);
    exit(EXIT_FAILURE);
}

/**
 * @brief This function free all resources in numbers. Then it print an
 * error message and exit the programm with EXIT_FAILURE.
 * 
 * @param msg The error message which is printed.
 * @param numbers The resources which are free. 
 */
static void exitAndFree(char *msg, Mult numbers)
{
    assert(msg != NULL);

    //Free resources
    free(numbers.fInt);
    free(numbers.sInt);

    //Print error message and exit
    printErrorMsgExit(msg);
}

/**
 * @brief This function parse the upper half of a string to long.
 * If the string is shorter than the given halfLen, the result will be 0.
 * 
 * @param res The string which is parsed.
 * @param halfLen The half of the maximum lenght. 
 * @return The result of the parsed string.
 */
static long upperPart(PartRes res, size_t halfLen)
{
    assert(res.res != NULL);

    //Allocate buffer
    char *halfPart = (char*)malloc((halfLen+2)*sizeof(char)), *endptr;
    if(halfPart == NULL) return 0;
    halfPart = memset(halfPart, 0, halfLen+2);

    //Find the upper half of the string
    if(res.size > halfLen){
        memcpy(halfPart, res.res, halfLen);
        halfPart[halfLen+1] = '\0';
    }
    else halfPart[0] = '\0';

    //Parse the upper half to long
    long num = strtol(halfPart, &endptr, 16);
    free(halfPart);

    return num;
}

/**
 * @brief This function parse the lower half of a string to long.
 * 
 * @param res The string which is parsed.
 * @param halfLen The half of the maximum lenght. 
 * @return The result of the parsed string.
 */
static long lowerPart(PartRes res, size_t halfLen)
{
    assert(res.res != NULL);

    //Allocate buffer
    char *halfPart = (char *)malloc((halfLen+2)*sizeof(char)), *endptr;
    if(halfPart == NULL) return 0;
    halfPart = memset(halfPart, 0, halfLen+2);

    //Find the lower half of the string
    int offset = (res.size > halfLen) ? halfLen : 0;
    memcpy(halfPart, (res.res)+offset, halfLen);
    halfPart[halfLen+1] = '\0';

    //Parse the lower half to long
    long num= strtol(halfPart, &endptr, 16);
    free(halfPart);

    return num;
}

/**
 * @brief This functions read the results from the children. Than its adds
 * the part results together and finally prints the result to stdout.
 * 
 * @param numbers The numbers which are to be multiplied
 * @param resultFd The pipe where the children write their results.
 */
static void calculateChildrenResult(Mult numbers, int *resultFd)
{
    assert(resultFd != NULL);

    //Read results from children
    PartRes childRes[4];
    for(int i=0; i<4; i++){
        childRes[i].res = (char*)malloc((numbers.nSize+3)*sizeof(char));
        if(childRes[i].res == NULL){
            for(int j=0; j<4; j++) {
                close(resultFd[j]);
                free(childRes[j].res);
            }
            exitAndFree("Could not allocate memory for result", numbers);
        }
        childRes[i].size = read(resultFd[i], childRes[i].res, numbers.nSize+2);
        if(childRes[i].size == -1){
            for(int j=0; j<4; j++) {
                close(resultFd[j]);
                free(childRes[j].res);
            }
            exitAndFree("Could not read from pipe", numbers);
        }
        close(resultFd[i]);
        childRes[i].res[strcspn(childRes[i].res,"\n")] = '\0';
        childRes[i].size--;
    }

    //Calculate part results togehter
    size_t halfLen = numbers.nSize/2;
    long result[4];
    result[0] = lowerPart(childRes[0], halfLen);
    result[1] = upperPart(childRes[0], halfLen);
    result[1] += lowerPart(childRes[1], halfLen);
    result[1] += lowerPart(childRes[2], halfLen);
    result[2] = result[1] >> (4*halfLen);
    result[1] &= ~((~0L) << (4*halfLen));
    result[2] += upperPart(childRes[1], halfLen);
    result[2] += upperPart(childRes[2], halfLen);
    result[2] += lowerPart(childRes[3], halfLen);
    result[3] = result[2] >> (4*halfLen);
    result[2] &= ~((~0L) << (4*halfLen));
    result[3] += upperPart(childRes[3], halfLen);

    //Print result to stdout  
    for(int i=3; i>=0; i--){
        char *partStr = (char *)malloc((halfLen+1)*sizeof(char));
        for(int j=halfLen; j>0; j--){
            char hexDigit[2];
            snprintf(hexDigit, 2, "%lx", (result[i] & 15));
            result[i] = result[i] >> 4;
            partStr[j-1] = hexDigit[0];
        }
        partStr[halfLen] = '\0';
        fprintf(stdout, "%s", partStr);
        free(partStr);
    }
    fprintf(stdout, "\n");
    fflush(stdout);

    //Free all resources
    for(int i=0; i<4; i++) free(childRes[i].res);
}

/**
 * @brief This function create 4 children and for each child two pipes.
 * The child redirect stdin and stdout to the respective pipe end and 
 * exec the same programm. The parent write though the pipe to the child 
 * a half of each number to calculate the part results.  
 * 
 * @param numbers The numbers which are to be multiplied.
 * @param resultFd The pipe where the children write their results.
 */
static void makeChildren(Mult numbers, int *resultFd)
{
    assert(resultFd != NULL);

    //Create buffer for integer half
    int halfLen = numbers.nSize/2;
    char *fh = (char*)malloc((numbers.nSize+4)*sizeof(char));
    if(fh == NULL){
        exitAndFree("Could not allocate memory", numbers);
    }
    char *sh = (char*)malloc((halfLen+2)*sizeof(char));
    if(sh == NULL){
        free(fh);
        exitAndFree("Could not allocate memory", numbers);
    }

    //Create pipes and fork
    int multPipe[4][2] = {{0,0},{0,0},{0,0},{0,0}};
    int resultPipe[4][2] = {{0,0},{0,0},{0,0},{0,0}};
    for(int i=0; i<4; i++){

        //Open pipes for children
        if(pipe(multPipe[i]) == -1 || pipe(resultPipe[i]) == -1){
            free(fh);
            free(sh);
            exitAndFree("Could not open pipe", numbers);
        }

        switch (fork()) {
        case -1: //No fork possible

            close(multPipe[i][0]);
            close(multPipe[i][1]);
            close(resultPipe[i][0]);
            close(resultPipe[i][1]);
            free(fh);
            free(sh);
            exitAndFree("Could not fork", numbers);

        case 0: //Child process

            //Redirect stdin of child
            close(multPipe[i][1]);
            dup2(multPipe[i][0],STDIN_FILENO);
            close(multPipe[i][0]);

            //Redirect stdout of child
            close(resultPipe[i][0]);
            dup2(resultPipe[i][1],STDOUT_FILENO);
            close(resultPipe[i][1]);

            //Load child programm
            execl("./intmul", progName, NULL);
            free(fh);
            free(sh);
            exitAndFree("Could not exec", numbers);

        default: //Parent process

            close(multPipe[i][0]);
            close(resultPipe[i][1]);

            //Desice which halfs are send to child
            int offset = (i < 2) ? halfLen : 0;
            memcpy(fh, (numbers.fInt)+offset, halfLen);
            fh[halfLen] = '\n';
            fh[halfLen+1] = '\0';
            offset = (i%2 == 0) ? halfLen : 0;
            memcpy(sh, (numbers.sInt)+offset, halfLen);
            sh[halfLen] = '\n';
            sh[halfLen+1] = '\0';

            //Write chosen halfs to child
            write(multPipe[i][1], strcat(fh,sh), numbers.nSize+3);
            close(multPipe[i][1]);
            resultFd[i] = resultPipe[i][0];
            break;
        }
    }
    free(fh);
    free(sh);
}

/**
 * @brief The function parse two string to a long and multiply the 
 * numbers. The reuslt is printed to stdout. 
 * 
 * @param numbers The numbers which should be multiplied.
 */
static void multiplySingle(Mult numbers)
{
    char *endptr; 

    //Parse first integer
    errno = 0;
    long fInt = strtol(numbers.fInt,&endptr, 16);
    if(endptr == numbers.fInt || *endptr != '\0' || errno != 0){
        exitAndFree("First integer could not be parsed correctly", numbers);
    }

    //Parse second integer
    errno = 0;
    long sInt = strtol(numbers.sInt,&endptr, 16);
    if(endptr == numbers.sInt || *endptr != '\0' || errno != 0){
        exitAndFree("Second integer could not be parsed correctly", numbers);
    }

    //Calculate result and print to stdout
    long result = fInt*sInt;
    fprintf(stdout, "%lx\n", result);
    fflush(stdout);
}

/**
 * @brief This function calculate the multiplication of two hex numbers.
 * If the numbers are bigger than one digit, the function create 
 * child processes to calculate part results and adds them together.
 * Therefore the calculation is faster with bigger hex numbers.
 * 
 * @param numbers The numbers which are to be multiplied. 
 */
static void calculateResult(Mult numbers)
{
    //Multiply both numbers if they are single digit
    if(numbers.nSize == 1){
        multiplySingle(numbers);
        return;
    }

    //Split numbers and create children
    int resultFd[4];
    makeChildren(numbers, resultFd);

    //Wait for all children
    bool abortProgram = false;
    for(int i=0; i<4; i++){
        int status;
        while(wait(&status) == -1){
            if(errno == EINTR) continue;
            exitAndFree("Cannot wait, I am sorry little one", numbers);
        }
        if(WEXITSTATUS(status) != EXIT_SUCCESS) abortProgram = true;
    }
    //Abort programm if a child did not exit successful
    if(abortProgram) exitAndFree("A child aborted", numbers);

    //Calculate and print result
    calculateChildrenResult(numbers, resultFd);
}

/**
 * @brief This function reads two lines from stdin. If the lenght
 * of each line is not the same or even (expect lenght of one), the
 * function print an error message and exits the program.
 * 
 * @return The read lines from the stdin 
 */
static Mult getInteger()
{
    Mult input = {.nSize = 0, .fInt = NULL, .sInt = NULL};
    size_t len = 0;

    //Read first integer
    while((input.nSize = getline(&(input.fInt), &len, stdin)) == -1){
        if (errno == EINTR) continue;
        exitAndFree("Could not read from stdin", input);
    }
    input.fInt[strcspn(input.fInt, "\n")] = '\0';
    input.nSize--;

    //Read second integer
    ssize_t sSize = 0;
    len = 0;
    while((sSize = getline(&(input.sInt), &len, stdin)) == -1){
        if (errno == EINTR) continue;
        exitAndFree("Could not read from stdin", input);
    }
    if(strcspn(input.sInt, "\n") != sSize){
        input.sInt[strcspn(input.sInt, "\n")] = '\0';
        sSize--;
    }
    
    //Checks if there is too much input or the integers are invalid
    if(input.nSize != sSize){
        exitAndFree("Invalid input: the two integers have to be equal lenght", input);
    }
    if(input.nSize%2 == 1 && input.nSize != 1){
        exitAndFree("Invalid input: the two integers have to be even lenght", input);
    }
    if(input.nSize == 0){
        exitAndFree("Invalid input: ... you didn't even put a single integer in. What is wrong with you?", input);
    }
    return input;
}

/**
 * @brief The programm starts here. 
 * No arguments are expected. First of all the program read two numbers from
 * stdin. Then it calculate the multiplication of the numbers and prints the
 * result to stdout.
 * 
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[])
{
    progName = argv[0];

    //Expect no arguments
    if(argc > 1){
        fprintf(stderr,"Usage: %s \n", progName);
        fflush(stderr);
        return EXIT_FAILURE;
    }

    //Get integers from stdin
    Mult numbers = getInteger();

    //Calculate result
    calculateResult(numbers);

    //Free resources
    free(numbers.fInt);
    free(numbers.sInt);

    return EXIT_SUCCESS;
}