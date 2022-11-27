/*
*   @file   cpair.c
*
*   @author Jakob Frank (11837319)
*
*   @details cpair takes in a free ammount of points from stdin,
*            (usage: xCoordinate yCoordinate\n) and then returns the shortest distance
*            between all the points and prints the solution to stdout. The program uses
*            recursive Algorithm by forking itself into child processes.
*
*   @date   10/12/2021
*/



#include "cpair.h"



//**********************************************************************************************


/*
*   @brief  Simple Math function - compares two distances and returns the smaller of the two.
*
*   @param  a   a double value
*   @param  b   a double value to compare a to
*   @param  *diff   if the value a is smaller than b, then d == 1. This is used to check comparisons more efficiently
*/
double my_min(double a, double b, int * diff);   


/*
*   @brief  reads in an input from stdin and factors it inside an array to further use within the program.
*           For that it reallocates memory dynamically depending on the size of the input.
*
*   @param  **input a pointer the starting adress of the array
*   @param  *memSize    a reference to the address, where the current size of memory is stored.
*/
int createInput(point ** input, size_t * memSize);


/*
*   @brief removes the "/n" character at the end of a line (because getline() is used)
*
*   @param *lineptr a pointer to the start of the String to be checked
*   @param  c   the size of the line stated in *lineptr
*/
void removeNewLine(char * lineptr, ssize_t c);


/*
*   @brief Prints the point to an output file
*
*   @param  input   the point to print to the output file
*   @param  *out    the output file to print the point to
*/
void printPoint(point input, FILE * out);


/*
*   @brief  Reads the result from the child process
*
*   @param  *output references the address where the read result should be stored
*   @param  *read   references the output file of the child process to read from
*   @param  *left   used for error handling to free allocated memory in case of error
*   @param  *right  used for error handling to free allocated memory in case of error
*/
void readChild(pair_t * output, FILE * read, point * left, point * right);


/*
*   @brief  Takes in an array of points, and calculates the mean of all xCoordinates of points within said array
*
*   @param  *input  The point array to operate
*   @param  inputSize   States the ammount of points withing *input
*/
double calcMean(point * input, size_t inputSize);


/*
*   @brief  Takes in an input array and splits it according to the mean
*
*   @details    The function goes through all values of the input array and copies the points within to either the 
*               left (xCoordinate <= mean) or right (xCoordinate > mean) point array. While doing so, the function also
*               updates the size of the output arrays. The function stops when all points of the input array are copied
*               to either the left or right subarray.
*
*   @param  *left   a subarray where all points.xCoordinate <= mean
*   @param  *right  a subarray where all points.xCoordinate > mean
*   @param  *input  the input array to copy the points from
*   @param  mean    the average value of the xCoordinates of all points within *input
*   @param  size    total size of the input array
*   @param  *leftSize   a pointer to the current value of the Size of the left array
*   @param  *rightSize  a pointer to the current value of the Size of the right array
*/
void splitArray(point * left, point * right, point * input, double mean, size_t size, size_t * leftSize, size_t * rightSize);


/*
*   @brief  Calculates the distance between two points
*
*   @param  pointA  a Point with an x and y coordinate  
*   @param  pointB  a Point with an x and y coordinate
*/  
double calcDistance(point pointA, point pointB);


/*
*   @brief Writes the Point pair with the shortest distance between to the best pair ptr.
*
*   @details    Recieves the best point pairs from the two childprocesses (leftC, rightC) and calculates its own best solution by 
*               getting the shortest distance between all points that the childprocesses used. It then compares the results depending
*               on the distance between the pairs and assigns the pair "best" the value with the shortest distance between two points.
*
*   @param  leftC   the best result of the left child process
*   @param  rightC  the best result of the right child process
*   @param  *left   a reference to the starting address of the array of points, that the left child process used
*   @param  *right  a reference to the starting address of the array of points, that the right child process used
*   @param  *best   the address, where the best result found in the function will be saved
*   @param  leftS   the size of the *left array
*   @param  rightS  the size of the *right array
*/
void getShortestDistance(pair_t leftC, pair_t rightC, point * left, point * right, pair_t * best, size_t leftS, size_t rightS);


/*
*   @brief  Frees allocated Memory, and writes an error message with the errno to stderr
*
*   @param  *input  the allocated memory to be freed
*   @param  *errMsg a custom error message depending on the use-case of errorExit()
*   @param  err The number of the last ocurring error (errno)
*/
void errorExit(point * input, const char * errMsg, int err);


/*
*   @brief A function to simplify the open file process. The return value is used to check if the file was opened successfully (0 == Success)
*
*   @param  **file  a pointer to the file address to be opened
*   @param  *pipe   references the adress to the pipe that should be opened
*   @param  rwIndex a number to decide if the file should be "read" or "write"
*/
int openFile(FILE ** file, int * pipe, int rwIndex);


/*
*   @brief  Closes both instances of a pipe
*
*   @param *pipe    Represents a pointer to the first element of a pipe.
*/
void closePipe(int * pipe);


//**********************************************************************************************


const char * programName;


int main(int argc, char ** argv)
{ 
    if (argc != 1)
    {
        //wrong usage -> to many args
        fprintf(stderr, "Program Name: %s\nInvalid Ammount of Args!\n", programName);
        exit(EXIT_FAILURE);
    }
    else
    {
        programName = argv[0];
    }
    
    //Parsing input
    //*********************************************************************************************
    size_t memSize = 2;  //starting size of memory...
    point * input = malloc(memSize*sizeof(point));
    size_t inputSize = createInput(&input, &memSize); //initialize input
    
    //Checking input
    //*********************************************************************************************
    switch (inputSize)
    {
    case 2:
        for (size_t i = 0; i < inputSize; ++i)
        {
            printPoint(input[i], stdout);
        }

    case 1:
        free(input);
        exit(EXIT_SUCCESS);
        break;

    case 0:
        fprintf(stderr, "Program Name: %s\nNo input -- illegal Argument\n", programName);
        free(input);
        exit(EXIT_FAILURE);
        break;
    
    default:
        break;
    }
    

    //create pipes
    //*********************************************************************************************

    int write1[2];
    int read1[2];
    int write2[2];
    int read2[2];

    if(pipe(write1) == -1)
    {
        errorExit(input, "Pipe 'write1' didn't open properly", errno);    
    }

    if(pipe(read1) == -1)
    {
        errorExit(input, "Pipe 'read1' didn't open properly", errno);
    }

    if(pipe(write2) == -1)
    {
        errorExit(input, "Pipe 'write2' didn't open properly", errno);
    }

    if(pipe(read2) == -1)
    {
        errorExit(input, "Pipe 'read2' didn't open properly", errno);
    }


    pid_t pid1;
    pid_t pid2;


    switch (pid1 = fork())
    {
    case -1:
        errorExit(input, "Cannot fork process!\n", errno);
        break;

    case 0:

        //start first child (left)

        closePipe(write2);
        closePipe(read2);
        close(read1[0]);
        close(write1[1]);

        if (dup2(read1[1], STDOUT_FILENO) == -1)
        {
            errorExit(input, "Duplicate didn't work properly!", errno);
        }
        
        if (dup2(write1[0], STDIN_FILENO) == -1)
        {
            errorExit(input, "Duplicate didn't work properly!", errno);
        }

        close(read1[1]);
        close(write1[0]);

        execlp(programName, programName, NULL);
        errorExit(input, "Couldn't execute program", errno);

        break;
    
    default:
        switch (pid2 = fork())
        {
        case -1:
            errorExit(input, "Cannot fork process!\n", errno);
            break;

        case 0:
            //start 2nd child task (right)

            closePipe(write1);
            closePipe(read1);
            close(read2[0]);
            close(write2[1]);

            if (dup2(read2[1], STDOUT_FILENO) == -1)
            {
                errorExit(input, "Duplicate didn't work properly!", errno);
            }
            if (dup2(write2[0], STDIN_FILENO) == -1)
            {
                errorExit(input, "Duplicate didn't work properly!", errno);
            }
            
            close(read2[1]);
            close(write2[0]);

            execlp(programName, programName, NULL);
            errorExit(input, "Couldn't execute program", errno);
            
            break;
        
        default:
            
            break;
        }
        break;
    }

    //create Filestreams

    FILE * leftRead;
    FILE * leftWrite;
    FILE * rightRead;
    FILE * rightWrite;

    int errsum = 0;
    errsum += openFile(&leftRead, read1, 0);
    errsum += openFile(&leftWrite, write1, 1);
    errsum += openFile(&rightRead, read2, 0);
    errsum += openFile(&rightWrite, write2, 1);

    if (errsum != 0)
    {
        close(read1[0]);
        close(write1[1]);
        close(read2[0]);
        close(write2[1]);
        errorExit(input, "File didn't open properly\n", errno);
    }
    

    double xMean = calcMean(input, inputSize);
    point * left = malloc(memSize*sizeof(point));
    if(left == NULL)
    {
        errorExit(input, "Error allocating memory!",errno);
    }

    point * right = malloc(memSize*sizeof(point));
    if (right == NULL)
    {
        free(left);
        errorExit(input, "Error allocating memory!",errno);
    }

    size_t leftSize = 0;
    size_t rightSize = 0;

    splitArray(left, right, input, xMean, inputSize, &leftSize, &rightSize);
    free(input);

    for (size_t i = 0; i < leftSize; i++)
    {
        printPoint(left[i], leftWrite);
    }
    
    for (size_t i = 0; i < rightSize; i++)
    {
        printPoint(right[i], rightWrite);
    }

    fclose(leftWrite);
    fclose(rightWrite);

    int status;
    while(waitpid(pid1, &status, 0) == -1)
    {
        if (errno == EINTR)
        {
            continue;
        }
        fclose(leftRead);
        fclose(rightRead);
        exit(EXIT_FAILURE);
    }
    
    if (WEXITSTATUS(status) == EXIT_FAILURE)
    {
        fprintf(stderr, "Program Name: %s\nError in left child process\n", programName);
        fclose(leftRead);
        fclose(rightRead);
        exit(EXIT_FAILURE);
    }
    
    while(waitpid(pid2, &status, 0) == -1)
    {
        if (errno == EINTR)
        {
            continue;
        }
        fclose(leftRead);
        fclose(rightRead);
        exit(EXIT_FAILURE);
    }

    if (WEXITSTATUS(status) == EXIT_FAILURE)
    {
        fprintf(stderr, "Program Name: %s\nError in right child process\n", programName);
        fclose(leftRead);
        fclose(rightRead);
        exit(EXIT_FAILURE);
    }

    pair_t leftChild;
    pair_t rightChild;
    pair_t best;

    readChild(&leftChild, leftRead, left, right);
    readChild(&rightChild, rightRead, left, right);

    getShortestDistance(leftChild, rightChild, left, right, &best, leftSize, rightSize);
    printPoint(best.p1, stdout);
    printPoint(best.p2, stdout);

    fclose(leftRead);
    fclose(rightRead);
    free(left);
    free(right);

    return EXIT_SUCCESS;
}



int createInput(point ** input, size_t * memSize)
{

    char * lineptr = NULL;
    int size = 0;
    size_t c = 0;
    ssize_t lineLen;
    

    while ((lineLen = getline(&lineptr, &c, stdin)) != -1)
    {
        //split lineptr to create two double values
        double x;
        double y;

        removeNewLine(lineptr, lineLen);
        sscanf(lineptr, "%lf %lf", &x, &y);
        
        (*input)[size].xCoordinate = x;
        (*input)[size].yCoordinate = y;

        //update size of array

        ++ size;
        if (size == *memSize)
        { 
            *memSize = (*memSize * 2);
            *input = realloc(*input, *memSize * sizeof(point));
        }
        
    }

    free(lineptr);
    if (errno != 0)
    {
        free(*input);
        //fprintf(stderr,"\n\n %d \n\n", size);
        exit(EXIT_FAILURE);
    }
    
    return size;
}



void removeNewLine(char * lineptr, ssize_t c)
{
    if (lineptr[c - 1] == '\n')
    {
        lineptr[c - 1] = '\0';
    }
    
}



void printPoint(point input, FILE * out)
{
    fprintf(out, "%lf / %lf\n", input.xCoordinate, input.yCoordinate);
}



void readChild(pair_t * output, FILE * read, point * left, point * right)
{
    char * lineptr = NULL;
    size_t c;
    ssize_t linelen;

    if ((linelen = getline(&lineptr, &c, read)) != -1)
    {
        sscanf(lineptr, "%lf / %lf\n", &(output->p1.xCoordinate), &(output->p1.yCoordinate));
    }
    else
    {   
        free(left);
        free(right);
        free(lineptr);
        fprintf(stderr, "Program Name: %s\nCouldn't read from child\n", programName);
        exit(EXIT_FAILURE);
    }
    
    if ((linelen = getline(&lineptr, &c, read)) != -1)
    {
        sscanf(lineptr, "%lf / %lf\n", &(output->p2.xCoordinate), &(output->p2.yCoordinate));
    }
    else
    {   
        free(left);
        free(right);
        free(lineptr);
        fprintf(stderr, "Program Name: %s\nCouldn't read from child\n", programName);
        exit(EXIT_FAILURE);
    }
    
    free(lineptr);
    output->dist = calcDistance(output->p1, output->p2);
}



double calcMean(point * input, size_t inputSize)
{   
    double sum = 0;

    
    for (size_t i = 0; i < inputSize; ++i)
    {
        sum += input[i].xCoordinate;
    }
    

    return sum / inputSize;
}



void splitArray(point * left, point * right, point * input, double mean, size_t size, size_t * leftSize, size_t * rightSize)
{

    for (size_t i = 0; i < size; i++)
    {
        if (input[i].xCoordinate <= mean)
        {
            left[*leftSize] = input[i];
            ++ (*leftSize);
        }
        else
        {
            right[*rightSize] = input[i];
            ++ (*rightSize);
        }
    }  
}



double calcDistance(pt_t pointA, pt_t pointB)
{   
    double xDist = pointA.xCoordinate - pointB.xCoordinate;
    double yDist = pointA.yCoordinate - pointB.yCoordinate;

    return sqrt(pow(xDist, 2) + pow(yDist, 2));
}



double my_min(double a, double b, int * diff)
{
    if (a < b)
    {
        *diff = 1;
    }
    
    return a < b ? a : b; 
}



void getShortestDistance(pair_t leftC, pair_t rightC, point * left, point * right, pair_t * best, size_t leftS, size_t rightS)
{   

    double dist1 = leftC.dist;
    double dist2 = rightC.dist;
    double dist3 = DBL_MAX;
    int diff = 0;
    point a3;
    point b3;

    for (size_t i = 0; i < leftS; ++i)
    {
        for (size_t j = 0; j < rightS; ++j)
        {
            dist3 = my_min(calcDistance(left[i], right[j]), dist3, &diff);

            if (diff == 1)
            {
                diff = 0;
                a3 = left[i];
                b3 = right[j];
            }  
        }  
    }
    double min = dist1;
    *best = leftC;

    if (min > dist2)
    {
        min = dist2;
        *best = rightC;
    }
    
    if (min > dist3)
    {
        best -> dist = dist3;
        best -> p1 = a3;
        best -> p2 = b3;
    }
}



void errorExit(point * input, const char * errMsg, int err)
{
    free(input);
    fprintf(stderr, "Program Name: %s\nError %d\n%s", programName, errno, errMsg);
    exit(EXIT_FAILURE);
}



int openFile(FILE ** file, int * pipe, int rwIndex)
{
    char index;
    switch (rwIndex)
    {
    case 0:
        index = 'r';
        break;
    
    default:
        index = 'w';
        break;
    }
    if ((*file = fdopen(pipe[rwIndex], &index)) == NULL)
    {
        return 1;
    }
    return 0;
}



void closePipe(int * pipe)
{
    for (size_t i = 0; i < 2; ++i)
    {
        close(pipe[i]);
    } 
}