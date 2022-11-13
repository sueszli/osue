/*
*   @file   generator.c
*
*   @author Jakob Frank (11837319)
*
*   @details The generator program takes in a free ammount of arguments, which represent the edges
*            of a graph. It then generates a random solution using the Fischer Yates shuffle. In the
*            end, the program sends its solution to the shared memory.
*
*   @date   10/11/2021
*/


#include "../circular_buffer.h"


/*
* @brief    "void usage" indicates incorrect arguments and stops the program
*/


void usage(void); //checks command line usage; returns error if done incorrectly


/*
*   @brief  Filters all unique vertices from the edges (which are recieved as arguments)
*
*   @param  **input a string array to be filtered
*   @param  *output an integer array which will be filled with values from **input
*   @param  size    indicates the size of the input array
*   @param  *vCount reference to the current size of the output array
*/


void getVertices(char **input, int *output, size_t size, size_t * vCount);


/*
*   @brief  exists checks if an integer exists within an array (returns 1 if yes, 0 if not)
*
*   @param  *input  array to be checked
*   @param  comp    integer to look for within array
*   @param  size    size of input array
*/


int exists(int * input, int comp, size_t size);


/*
*   @brief  shuffles the input using the Fisher-Yates Algorithm
*
*   @param  *input  array to be shuffled
*   @param  size    size of input array
*/


void shuffleFY(int * input, size_t size);


/*
*   @brief checks if an edge is part of arc set solution (1 if eligible, 0 if not)
*
*   @param  edgeStart   is vertice where the edge started
*   @param  edgeEnd     is vertice where the edge ended
*   @param  *vertices   array of all vertices with edges
*   @param  *vCount     size of vertice array
*/


int isEligible(int edgeStart, int edgeEnd, int * vertices, size_t * vCount);

/*
*   @brief programName  stores arg[0] which also is the name of the process
*/
char * programName;


int main(int argc, char **argv)
{
    programName = argv[0];
    srand(time(NULL));


    /* checking if edges were written as args at the beginning so the program doesn't need to continue in case of error */
    if (argc == 1)
    {
        usage();
    }


    int edgeCount = argc - 1;
    char ** edges = &argv[1]; //Modified reference on argv
    size_t vCount = 0; //returns how many different vertices exist...
    
    int * vertices = malloc(2 * edgeCount * sizeof(int)); //biggest possible vertice array

    getVertices(edges, vertices, edgeCount, &vCount);

    size_t getsize = 0;
    for (size_t i = 0; i < edgeCount; i++)
    {
        getsize += (strlen(edges[i]));
    }


    //initialize the circular buffer


    int fd;
    buf * bufptr;

    memInitGenerator(&fd, &bufptr);


    //initialize the semaphores


    //write to buffer
    sem_free = sem_open(SEM_FREE, O_RDWR, 0600, BUF_SIZE);

    if (sem_free == SEM_FAILED)
    {
        fprintf(stderr, "Error, sem_free couldn't be opened by client");
        memClearGenerator(&fd, &bufptr);
        exit(EXIT_FAILURE);
    }
    
    //read from buffer
    sem_used = sem_open(SEM_USED, O_RDWR, 0600, 0);

    if (sem_used == SEM_FAILED)
    {
        fprintf(stderr, "Error, sem_used couldn't be opened by client");
        memClearGenerator(&fd, &bufptr);
        exit(EXIT_FAILURE);
    }

    //ensure mutex over multiple generators (only 1 process can write in mem at a time)
    sem_write = sem_open(SEM_WRITE, O_RDWR, 0600, 1);

    if (sem_used == SEM_FAILED)
    {
        fprintf(stderr, "Error, sem_used couldn't be opened by client");
        memClearGenerator(&fd, &bufptr);
        exit(EXIT_FAILURE);
    }


    while (bufptr->quit == 0)
    {
    
        shm_t result;
        result.size = 0;

        shuffleFY(vertices, vCount);

        int edgeStart = 0;
        int edgeEnd = 0;


        for (size_t i = 0; i < edgeCount; ++i)
        {
            sscanf(edges[i], "%d-%d", &edgeStart, &edgeEnd);

            if ((isEligible(edgeStart, edgeEnd, vertices, &vCount)) == 0)
            {
                continue;
            }
            else
            {
                edge eligible;
                eligible.start = edgeStart;
                eligible.end = edgeEnd;

                result.edges[result.size] = eligible;
                ++result.size;
            }
        }    

        if (result.size >= MAX_DATA)
        {
            continue;
        }
        else
        {    
            writeMem(bufptr, &result);
        }
    }
    
    memClearGenerator(&fd, &bufptr);
    free(vertices);

    return 0;
}


void usage(void)
{
    fprintf(stderr, "%s EDGE1 ... /nPlease enter at least one edge (Syntax: x-y, x != y)!\n", programName);
    exit(EXIT_FAILURE);
}


void getVertices(char **input, int *output, size_t size, size_t * vCount)
{
    int num1 = 0;
    int num2 = 0;

    for (size_t i = 0; i < size; ++i)
    {
        sscanf(input[i], "%d-%d", &num1, &num2);


        if ((exists(output, num1, *vCount)) == 0)
        {
            output[*vCount] = num1;
            *vCount += 1;
        }

        if ((exists(output, num2, *vCount)) == 0)
        {
            output[*vCount] = num2;
            *vCount += 1;
        }    
    }
}


int exists(int * input, int comp, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (input[i] == comp)
        {
            return 1;
        }
        
    }
    return 0;
}


void shuffleFY(int * input, size_t size)
{ int j, tmp;

     for (int i = --size; i > 0; --i) {

         j = rand() % (i + 1);

         tmp = input[j];
         input[j] = input[i];
         input[i] = tmp;
     }
}


int isEligible(int edgeStart, int edgeEnd, int * vertices, size_t * vCount)
{
    int startPos = -1;
    int endPos = -1;

    for (size_t i = 0; i < *vCount; i++)
    {
        if (vertices[i] == edgeStart)
        {
            startPos = i;
            continue;
        }

        if (vertices[i] == edgeEnd)
        {
            endPos = i;
        }    
    }
    
    if ((startPos == -1)||(endPos == -1))
    {
        fprintf(stderr, "%s, Error with input args. Vertices in edge must not be equal \n", programName);
        usage();
    }

    if (startPos > endPos)
    {
        return 1;
    }
    
    return 0;
}