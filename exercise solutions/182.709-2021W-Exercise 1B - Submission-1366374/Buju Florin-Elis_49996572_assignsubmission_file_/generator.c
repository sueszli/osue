/**
 * @file generator.c
 * @author 12024755, Florin-Elis Buju <e12024755@student.tuwien.ac.at> 
 * @date 04.11.2021
 * @brief Betriebssysteme 1B Feedback Arc Set generator
 * @details The program receives a graph as input and permantly sends the 
 * calculated Set Arc to the program supervisor which analyzes it. 
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

#include <fcntl.h> 
#include <stdio.h> 
#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <semaphore.h>

#define SHM_NAME "/12024755sharedmem"  /** Name of shared memory */
#define MAX_SIZE (32) /** Max Size of Circular Buffer */
#define STOP_VALUE (7200) /** Defines the value of the Stop Signal for Generators */
#define MAX_EDGES (8) /** Max Edges for an Arcset */

#define SEM_1 "/12024755sem_1" /** Name from "Free Space" Semaphore */
#define SEM_2 "/12024755sem_2" /** Name from "Used Space" Semaphore */
#define SEM_3 "/12024755sem_3" /** Name from "mutually exclusive access" Semaphore */

typedef struct {int x, y;} edge; /** struct of an single edge */
typedef struct {edge edges[MAX_EDGES]; int size;} arcSet; /** struct of an single edge */

static char *program; /** name of program */
static int shmid = -1; /** shared memeory for circual buffer */
static arcSet *mapshm = NULL; /** ArcSet array as circual buffer, used to write or read */
static sem_t *free_sem = NULL; /** Free Space Semaphore, free space in the circual buffer*/
static sem_t *used_sem = NULL; /** Used Space Semaphore, used space in the circual buffer*/
static sem_t *access_sem = NULL; /** Mutually exclusive access Semaphore, can only write mutually exclusive to the buffer individually */

static edge *edges = NULL; /** Represents the input graph as Edge Array */
static int *vertices = NULL;  /** Random shuffeld array of int, used to compute the edges, used for the new arcset */
static bool cleanupError = false; /** Shows if the clean up prozess had an Error and program will not excute futher */

static void myErrorout(char *failedto); 

/**
 * @brief Cleanup function, removes all semaphore and mmaps and shared memorys
 * @details Funktion is called when program terminates or had an error, 
 * @param none
 * @return no return value (void)  
 **/
static void cleanUp()
{
    // Clear Up all semaphores, mmaps and shared memorys
    if(mapshm != NULL)
    {
        if (munmap(mapshm, sizeof(*mapshm)) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to unmap the mapped memory.");
        }
    }

    if(shmid != -1)
    {
        if (close(shmid) < 0)
        {
            cleanupError = true;
            myErrorout("Closing the shared memory failed.");
        }
    }
    
    if(free_sem != NULL)
    {
        if (sem_close(free_sem) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to clean up Semephore free_sem.");
        }
    }
    if(used_sem != NULL)
    {
        if (sem_close(used_sem) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to clean up Semephore used_sem.");
        }
    }
    if(access_sem != NULL)
    {
        if (sem_close(access_sem) == -1)
        {
            cleanupError = true;
            myErrorout("Failed to clean up Semephores access_sem.");
        }
    }
    // Free allocated memorys
    if(edges != NULL)
    {
        free(edges);
    }
    if(vertices != NULL)
    {
        free(vertices);
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

    if(cleanupError == false)
    {
        cleanUp();
    }
    exit(EXIT_FAILURE);
}

/**
 * @brief Implementation of random shuffling of an int array
 * @details The mondern FisherYatesShuffle will be used to randomize the content of an array 
 * @param Countvertices Is the number of nodes in the given graph, it is used to know how many nodes exist
 * @return no return value (void)  
 **/
static void FisherYatesShuffle(int Countvertices) 
{
    vertices = malloc(sizeof(int) * (Countvertices+1));
    if (vertices == NULL) 
    {
        myErrorout("Memory reservation failed.");
    }
    // Create vertices Array for 0 to Countvertices
    int forc;
    for (forc = 0; forc < (Countvertices+1); forc++)
    {
        vertices[forc] = forc;
    }
    
    //Fisher Yates Shuffle, The modern algorithm (lowest to highest)
    int i;
    for (i = 0; i < (Countvertices-1); i++)
    {
        int j = (rand() % ((Countvertices) - i)) + i; // i <= j < n,n=(Countvertices+1), n exlusive
        
        //Exchange vertices[i] und vertices[j]
        int safe = vertices[i];
        vertices[i] = vertices[j];
        vertices[j] = safe;
    }
}
/**
 * @brief Finds the index of an given Element x in an Array
 * @details While looping through the array, each entry will be cheak for equality with int x
 * @param x is the search element, length indicates how many entries the array has 
 * @return the position of the element to be searched for is given back as an int
 **/
int findIndex(int x,const int *array,int length)
{
    int c1; 
    for (c1 = 0; c1 < length +1; c1++)
    {
        if(x == array[c1])
        {
            return c1;
        }
    }
    assert(0);
}
/**
 * @brief checkEdge is used to find out, if a given edge should be included in the new arcSet
 * @param (u,v) is the analyzed edge, vertices is an array with in values and "length" is its length
 * @return retruns if an edge should be in the arcset
 **/
bool checkEdge(int u, int v, int length)
{
    int pos1 = findIndex(u,vertices,length);
    int pos2 = findIndex(v,vertices,length);

    if(pos1 > pos2)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/**
 * @brief Generates many solutions for a new arcset depending on the input graph and then communicates them to a supervisor
 * @details To realize the application, semaphore and shared memories are used.
 **/
int main(int argc, char *argv[]) 
{
    program = argv[0];
    int c;
    int verticescount = 0;
    // Use getopt to handle input of argv
    while ((c = getopt(argc, argv, "io:")) != -1 ){
        switch (c) {
        case '?':
        //Illegal option as input 
        myErrorout("Usage: generator EDGE1 EDGE2..."); 
        break;
        default:
        // Shouldn't get into this part
        assert(0);
        break;
        }
    }
    if(argv[optind] == NULL)
    {
        myErrorout("Not a single Edge was submitted. Usage: generator EDGE1 EDGE2..."); 
    }
    int i=optind;
    // Counter for edges
    int counter = 0; 
    // Read all edges with dynamic memory
    edges = malloc((argc)*sizeof(edge));
    if (edges == NULL) 
    {
        myErrorout("Memory reservation failed.");
    }
    while (i<argc)
    {    
        //Search in the given param for the char '-' to check if usage is correct 
        if(argv[i]==NULL || strstr(argv[i], "-") == NULL)
        {
            myErrorout("Something went wrong reading the edges. Usage of EDGE: 'x-y' with x,y from the natural numbers"); 
        }
        else
        {
            char *leftInt = strtok(argv[i], "-");
            char *rightInt = strtok(NULL, "-");
            //Check if the splitting has been successful 
            if(strstr(leftInt, "") == NULL || leftInt == NULL || strstr(rightInt, "") == NULL || rightInt == NULL)
            {
                myErrorout("A Node was not entered properly when defining an edge. Usage of EDGE: 'x-y' with x,y Element from the natural numbers"); 
            }
            char *leftoverstr1,*leftoverstr2;
            int left = -1;
            int right = -1;
            left = strtol(leftInt, &leftoverstr1, 10);
            right = strtol(rightInt, &leftoverstr2, 10);
            
            if(left < 0 || right < 0 || strstr(leftoverstr1," ") != NULL || strstr(leftoverstr2," ") != NULL)
            {
                myErrorout("Usage of EDGE: 'x-y' with x,y Element from the natural numbers"); 
            }
            else
            {
                edges[counter].x=left;
                edges[counter].y=right;
                if(left > verticescount)
                {
                    verticescount = left;
                }
                if(right > verticescount)
                {
                    verticescount = right;
                }
                
            }           
        }
        i++;
        counter++;        
    }

    //Open shared memory for usage of circualr buffer
    shmid = shm_open(SHM_NAME, O_RDWR ,0600);
    if(shmid == -1)
    {
        myErrorout("Failed to open a shared memory."); 
    }
    
    // Size of arcSize ca. 64, 32*64 = 2048 Byte mapping, can be changed with MAX_SIZE
    mapshm = mmap(NULL, sizeof(arcSet) * (MAX_SIZE+1), PROT_READ | PROT_WRITE,MAP_SHARED, shmid, 0);
    if (mapshm == MAP_FAILED)
    {
         myErrorout("Failed to map memory."); 
    }
    // Open free Space semaphore
    free_sem = sem_open(SEM_1, 0);
    // Open used Space semaphore
    used_sem = sem_open(SEM_2, 0); 
    // Open mutually exclusive access semaphore
    access_sem = sem_open(SEM_3, 0);

    if(free_sem == SEM_FAILED)
    {
         myErrorout("Could not open SEM_1."); 
    }
    if(used_sem == SEM_FAILED)
    {
         myErrorout("Could not open SEM_2."); 
    }
    if(access_sem == SEM_FAILED)
    {
         myErrorout("Could not open SEM_3."); 
    }

    int arcsetsgen = 0;
    while(true)
    {
        arcsetsgen ++;
        // Generate a random shuffeld array saved in vertices
        FisherYatesShuffle(verticescount);
        if(vertices==NULL)
        {
            myErrorout("Error shuffeling the vertices.");
        }

        arcSet newarcSet;
        int arcsetcount = 0;
        int count1;
        // Check witch egde should be included in the new arc set 
        for (count1 = 0; count1 < counter; count1++)
        {
            int x = edges[count1].x;
            int y = edges[count1].y;
            if(checkEdge(x,y,verticescount) == true)
            {
                newarcSet.edges[arcsetcount] =edges[count1];
                arcsetcount++;
            }
            // Limit of MAX_EDGES
            if(arcsetcount==MAX_EDGES)
            {
                break;
            }
        }
        newarcSet.size = arcsetcount;


        if(mapshm[MAX_SIZE].edges[0].x == STOP_VALUE)
        {
            // Stop Value was recived, so stop the generator
            break;
        }
        else
        {  
            if (sem_wait(free_sem) == -1) {
                if (errno == EINTR)
                {
                    continue;
                } 
                myErrorout("An error accured while wating for semaphore free_sem."); 
            }

            if (sem_wait(access_sem) == -1) {
                if (errno == EINTR)
                {
                    continue;
                } 
                myErrorout("An error accured while wating for semaphore free_sem."); 
            }
            // This generator now is allowed to write into the circular buffer on pos writepos
            int writepos =  mapshm[MAX_SIZE].size;
            memcpy(&mapshm[writepos],&newarcSet,sizeof(newarcSet));
            printf("\n[%d] Got ArcSet with %d edges:",arcsetsgen, newarcSet.size);
            int s;
            for (s = 0; s < newarcSet.size; s++)
            {
                printf(" %d-%d",newarcSet.edges[s].x,newarcSet.edges[s].y);             
            }
            writepos += 1;
            writepos %=MAX_SIZE;
            memcpy(&mapshm[MAX_SIZE].size,&writepos,sizeof(int));
           
            if (sem_post(access_sem) == -1) {
                myErrorout("An error accured while wating for semaphore free_sem."); 
            }

            if (sem_post(used_sem) == -1) {
                myErrorout("An error accured while wating for semaphore free_sem."); 
            }
            free(vertices);
        }
    }
    // Clean up the program and exit
    cleanUp();
    printf("\nGenerator completed!\n");
    exit(EXIT_SUCCESS);
}

