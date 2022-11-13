/**
 * @file forkFFT.c
 * @author 12024755, Florin-Elis Buju <e12024755@student.tuwien.ac.at> 
 * @date 04.12.2021
 * @brief Betriebssysteme 2 Fork Fourier Transform
 * @details After start the program receives an input float complex array with 1 or 2^n entries
 * and then calculates its FFT
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <time.h>  
#include <complex.h>
#include <math.h>

#include <fcntl.h> 
#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>

#define PI (3.141592654)
static char *program; /** name of program */

static float complex singleInput = 0; /** Complex float which is used if there is only one input float*/
static float complex* Re = NULL; /** The result of the even part Pe */
static float complex* Ro = NULL; /** The result of the even part Po */
static float complex* R = NULL; /** The result R of the Fourier Transform of the entire array */

static FILE * w1 = NULL; /** File pointer to write to child1 */
static FILE * w2 = NULL; /** File pointer to write to child2 */

static FILE * r1 = NULL; /** File pointer to read from child1 */
static FILE * r2 = NULL; /** File pointer to read from child2 */

typedef struct 
{
    pid_t pid; /** Distinguish between child and parent and other childs */
    int status; /** Termination status of the process */
    int pipewrite[2]; /** Pipe to write to child process */
    int piperead[2]; /**  Pipe to read from child process */
} child; /** struct of an child */

static void myErrorout(char *failedto); 

/**
 * @brief Cleanup function
 * @details Is cleaning up all float complex arrays, who have reserved memory with malloce
 * @param none
 * @return no return value (void)  
 **/
static void cleanUp()
{
    if(Re != NULL)
    {
        free(Re);
    }
    if(Ro != NULL)
    {
        free(Ro);
    }
    if(R != NULL)
    {
        free(R);
    }
}

/**
 * @brief Error function of the program
 * @details Will output the passed error message and will exit the programm with EXIT_FAILURE
 * @param failedto is the ErrorMessage which should be print
 * @return no return value (void) 
 **/
static void myErrorout(char *failedto)
{
    if(strcmp(strerror(errno),"Success")==0)
    {
        fprintf(stderr, "[%s] ERROR: %s\n", program, failedto);
    }
    else
    {
        fprintf(stderr, "[%s] ERROR: %s: %s\n", program, failedto, strerror(errno));
    }

    cleanUp();
    exit(EXIT_FAILURE);
}

/**
 * @brief Convert a String to Complex Float
 * @details The complex float number is extracted from 
 * @param String buffer as input, and number as pointer for output
 * @return no return value (void)
 **/
static void convertToFloat(char* buffer, float complex* number)
{
    // double strtod(const char *nptr, char **endptr);
    char* endptr1 = NULL;
    char* endptr2 = NULL;

    *number = strtof(buffer,&endptr1);
    if((isspace(*endptr1) == 0) && (iscntrl(*endptr1) == 0) && (*endptr1 != '\n') && (*endptr2 != '*'))
    {
        myErrorout("Input was no floating point value.");
    }
    *number = (*number) + (strtof(endptr1,&endptr2) * I);
    if((isspace(*endptr2) == 0) && (iscntrl(*endptr2) == 0) && (*endptr2 != '\n') && (*endptr2 != '*'))
    {
        myErrorout("Input was no floating point value.");
    }
}


/**
 * @brief Outputs a complex float number
 * @details The given complex float number is printed to stdout 
 * @param String buffer as input, and number as pointer for output
 * @return no return value (void)
 **/
static void outputComplex(float complex number)
{
   fprintf(stdout,"%f %f*i\n",crealf(number),cimagf(number));
}

/**
 * @brief Closes pipes of an child
 * @details The important and used pipes of an child are beeing cloesd
 * @param a child is given as input parameter
 * @return no return value (void)
 **/
static void closePipes(child* child)
{
    if(close(child -> piperead[0]) == -1)
    {
        myErrorout("Failed to close Pipe.");
    }
    if(close(child -> pipewrite[1])== -1)
    {
        myErrorout("Failed to close Pipe.");
    }
}

/**
 * @brief Setting up a new child prozess
 * @details Forking a new child prozess and executing it afterwards
 * Setting up pipes to control the communication between parent and child
 * @param A child Struct is given as input, to describe its parameters and use them somewhere else
 * @return no return value (void)
 **/
static void SetChild(child* child)
{   
    if(pipe(child -> piperead) == -1)
    {
        myErrorout("Failed to setup Read-Pipe for child.");
    }
    if(pipe(child -> pipewrite) == -1)
    {
        myErrorout("Failed to setup Write-Pipe for child.");
    }
    int pid  = fork();
    child -> pid = pid;
    switch (pid) 
    {
        case -1:
            myErrorout("Cannot fork!");
        case 0:
            // child tasks
            // Redirection of stdin and stdout with dup2 and pipes 
            if(dup2(child -> pipewrite[0],STDIN_FILENO) == (-1))
            {
                myErrorout("Failed to ");
            }
            if(dup2(child -> piperead[1],STDOUT_FILENO) == (-1))
            {
                myErrorout("Length of the input array is not even");
            }
            if(close(child -> piperead[1])== -1)
            {
                myErrorout("Failed to close Pipe.");
            }
            if(close(child -> pipewrite[0])== -1)
            {
                myErrorout("Failed to close Pipe.");
            }
            closePipes(child);
            //Now execute new Child Prozess
            if(execlp(program, program, NULL) == -1)
            {
                myErrorout("Failed to execute child prozess.");
            }
            //Should not reach this line
            assert(0);
        break;
        default:
            // parent tasks
            if(close(child -> piperead[1])== -1)
            {
                myErrorout("Failed to close Pipe.");
            }
            if(close(child -> pipewrite[0])== -1)
            {
                myErrorout("Failed to close Pipe.");
            }
            // Do nothing as partent and continue
        break;
    }
}    

/**
 * @brief Computes the FFT of an floating point value array
 * @details If necessary, children's processes are created to calculate the given FortFFT,
 * comunicating with childes is established through pipes
 **/
int main(int argc, char *argv[]) 
{
    program = argv[0];
    //CHeck whether no input when called 
    if(argc != 1)
    {
       myErrorout(" Usage: ./forkFFT\nEnter floating point values after program start.");
    }
    
    size_t length1 = 0;
    size_t length2 = 0;
    char* buffer1 = NULL;
    char* buffer2 = NULL;
    // Read the first line from stdin
    if(getline(&buffer1, &length1, stdin) == EOF) 
    {
        // If no input was given, output error with usage
        myErrorout(" Usage: ./forkFFT\nEnter floating point values after program start.");
    }
    if(getline(&buffer2, &length2, stdin) == EOF) 
    {
        // If the array consists of only 1 number, write that number to stdout and exit with exit status EXIT_SUCCESS.
        convertToFloat(buffer1,&singleInput);
        outputComplex(singleInput);
        free(buffer1);
        free(buffer2);
        exit(EXIT_SUCCESS);
    }

    // Create child 1 prozesses 
    child child1;
    // Set up child 1 prozess and its pipes, and execute it
    SetChild(&child1);
    // Open File write pointer and write to child 1
    if((w1=fdopen(child1.pipewrite[1],"w")) == NULL)
    {
        free(buffer1);
        free(buffer2);
         myErrorout("Failed to open Objekt to write.");        
    }
    if(fputs(buffer1,w1) == -1)
    {
        free(buffer1);
        free(buffer2);
        fclose(w1);
        myErrorout("Failed to to write to child.");
    }
    
    // Create child 2 prozesses 
    child child2;
    // Set up child 2 prozess and its pipes, and execute it
    SetChild(&child2);
    // Open File write pointer and write to child 2
    if((w2=fdopen(child2.pipewrite[1],"w")) == NULL)
    {
        free(buffer1);
        free(buffer2);
        fclose(w1);
        fclose(w2);
        myErrorout("Failed to open Objekt to write.");
    }
    if(fputs(buffer2,w2) == -1)
    {
        free(buffer1);
        free(buffer2);
        myErrorout("Failed to to write to child.");
    }
    free(buffer1);
    free(buffer2);

    int count = 1;
    char* buffer5 = NULL;
    char* buffer6 = NULL;
    size_t length5 = 0;
    size_t length6 = 0;
    // Read all left entries from stdin and write it to the childs 
    while(true)
    {
        if(getline(&buffer5, &length5, stdin) == EOF) 
        {
            // Input Array is even and program-input correct
            break;
        }
        // Write buffer5 to child 1
        if(fputs(buffer5,w1) == -1)
        {
            free(buffer5);
            fclose(w1);
            fclose(w2);
            myErrorout("Failed to to write to child.");
        }
        // In this line must be a floating point, otherwise the number of input entries would be odd 
        if(getline(&buffer6, &length6, stdin) == EOF) 
        {
            fclose(w1);
            fclose(w2);
            myErrorout("Length of the input array is not even");
        }
        // Write buffer6 to child 2
        if(fputs(buffer6,w2) == -1)
        {
            free(buffer6);
            fclose(w1);
            fclose(w2);
            myErrorout("Failed to to write to child.");
        }
        count++;
    }
    // Only free buffers if they were used
    if(count != 1)
    {
        free(buffer5);
        free(buffer6);
    }
    
    fclose(w1);
    fclose(w2);

    
    pid_t pid1,pid2;
    // Wait for child 1 two finish
    while ((pid1 = waitpid(child1.pid, &child1.status, 0)) == -1)
    {
        if (pid1 != -1)
        {
            // other child
            continue;
        } 
        if (errno == EINTR) continue;
        // interrupted
        myErrorout("Could not wait for child to finish.");
    }
    // Check if child 1 was executed successfully 
    if (WEXITSTATUS(child1.status) != EXIT_SUCCESS) 
    {
        myErrorout("Child1 Prozess failed.");
    }

    // Wait for child 2 two finish
    while ((pid2 = waitpid(child2.pid, &child2.status, 0)) == -1)
    {
        if (pid2 != -1)
        {
            // other child
            continue;
        } 
        if (errno == EINTR) continue;
        // interrupted
        myErrorout("Could not wait for child to finish.");
    }
    // Check if child 2 was executed successfully 
    if (WEXITSTATUS(child2.status) != EXIT_SUCCESS) 
    {
        myErrorout("Child2 Prozess failed.");
    }

    // Open File Pointer to read the solucation from Child 1 Re (Even Array Entries)
    if((r1=fdopen(child1.piperead[0],"r")) == NULL)
    {
         myErrorout("Failed to open Objekt to read.");
         
    }
    // Open File Pointer to read the solucation from Child 2 Ro (Odd Array Entries)
    if((r2=fdopen(child2.piperead[0],"r")) == NULL)
    {
        myErrorout("Failed to open Objekt to read.");
    }

    int i;
    char *buffer3 = NULL;
    char *buffer4 = NULL;
    size_t length3 = 0;
    size_t length4 = 0;
    
    Re = malloc(sizeof(float complex) * count);
    Ro = malloc(sizeof(float complex) * count);
    float complex number1;
    float complex number2;
    // Read the amount of soluations that were wrote to the childs
    for (i = 0; i < count; i++)
    {
        // Get one entrie of Solution Re (Even Array Entries)
        if(getline(&buffer3, &length3, r1) == EOF) 
        {
           free(buffer3);
           myErrorout("Failed to to read from child1.");
        }
        convertToFloat(buffer3,&number1);
        Re[(i)] = number1;

        // Get one entrie of Solution Ro (Odd Array Entries)
        if(getline(&buffer4, &length4, r2) == EOF) 
        {
            free(buffer4);
            myErrorout("Failed to to read from child2.");
        }
        convertToFloat(buffer4,&number2);
        Ro[(i)] = number2;
    }
    fclose(r1);
    fclose(r2);
    
    R = malloc(sizeof(float complex) * count * 2);
    int k;
    // count = n/2
    int n;
    n = count * 2;

    // Calculate Bufferfly operation
    for (k = 0; k < count; k++)
    {
       R[k]=Re[k] + ((ccosf((-2*PI / n) * k)+ I * csinf((-2*PI / n) * k)) * Ro[k]); 
       R[(k+count)]=Re[k] - ((ccosf((-2*PI / n) * k)+ I * csinf((-2*PI / n) * k)) * Ro[k]);    
    }
    
    // Output the R array after the Bufferfly operation
    int s;
    for(s=0;s<count*2;s++)
    {
        outputComplex(R[s]);
    }
    free(buffer3);
    free(buffer4);
    
    // Cleanup and Exit
    cleanUp();
    exit(EXIT_SUCCESS);
}