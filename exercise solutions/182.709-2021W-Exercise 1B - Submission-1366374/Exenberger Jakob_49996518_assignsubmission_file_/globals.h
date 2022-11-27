#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>

/* 
* @author: Jakob Exenberger - 11707692
* @name: globals
* @brief: specification for the 3coloring program, semaphore names, constants and structs
* @details: definition for constants, structs, shared memory names, semaphore names and structs, which are used by the supervisor and the generator
* @date: 08.11.2021  
*/

//name of the shared memory 
#define SHM_NAME "/11707692_myshm"

//names of the semaphores used to synchronise the program parts
#define SEM_free "/11707692_sem_free_space"
#define SEM_used "/11707692_sem_used_space"
#define SEM_write "/11707692_sem_excl_write"

//size of the buffer which stores the solutions
#define SOLUTION_BUFFER_SIZE 12

//length of maximal solution  
#define MAX_EDGES 8

//definition of the node struct. One node contains a color and a name, which both are represented by an integer. 
typedef struct 
{
    int color;
    int name;
} node;

//definition of the edge struct. One edge contains a start and a end, which both are represented by an integer. 
//Both start and end are interpreted as nodes, which the edge connects 
typedef struct 
{
    int start;
    int end;
} edge;

//definition of the solution struct. One solution contains an edgesAmount, which is an integer representing how many edges are stored in the edges array.
//Additional it contains an array of edges 
typedef struct 
{
    int edgesAmount;
    edge edges[MAX_EDGES];
} solution;

//definition of the shared memory. The shared memory contains an integer endProgram, which is set by the supervisor and read by 
//the genrators to know when the program should be stopped. The second int writeIndex is used by the different generators to synchronize 
//on which index in the buffer they should write next. The solutions array contains the different solutions 
typedef struct 
{
    int endProgram;
    int writeIndex;
    solution solutions[SOLUTION_BUFFER_SIZE];
} myshm;