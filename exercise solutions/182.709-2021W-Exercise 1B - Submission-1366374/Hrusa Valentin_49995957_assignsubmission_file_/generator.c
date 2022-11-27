/**
 * @file generator.c
 * @author Valentin Hrusa 11808205
 * @brief generates feedback-arc-sets
 * @details generator takes a graph represented by its edges as input,
 *          builds a vertices-list, shuffles it with the fisher-yates algorithm.
 *          According to that shuffled list, a Feedback-Arc-Set is created and written
 *          to a shared memory construct.
 * 
 * @date 12.11.2021
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "structDefs.h"

//global variable for the name of the program
char *name;
//global variables for the shared memory and semaphores
Circular_Buffer *memory;
int shm_fd;
sem_t* sem_write;
sem_t* sem_read;
sem_t* sem_mutex;

/**
 * @brief calculates one Fb-Arc-Set
 * @details uses a heuristic to find a valid Fb-Arc-Set, if the amount of edges is 
 *          lower than the best solution found yet 1 is returned
 * 
 * @param vertexList array of all vertices in random positions from the input 
 * @param edgeList array of all edges from the input
 * @param arcset pointer to the arcset-struct which is filled with the solution
 * @param vcount amount of distinct vertices
 * @return int 0 if the solution has less Edges than the best solution found yet, 1 otherwise
 */
static int calculate_fb_arc_set(int vertexList[], Edge edgeList[], Fb_Arc_Set *arcset, int vcount);

/**
 * @brief adds a vertex to the verteslist
 * @details for every single vertex in the input this function is called. If the vertex hasn't
 *          yet been added to the list, it is added and 1 is returned.
 * 
 * @param vertexList array of all distinct vertices
 * @param vertex new vertex to add to vertexlist
 * @param size size of the vertexlist so far
 * @return int 0 if the vertex is already in vertexlist, 1 otherwise
 */
static int addVertex(int vertexList[], int vertex, int size);


/**
 * @brief randomly shuffels an int-array
 * @details the algorithm of fisher and yates is used to randomly shuffle an array
 *          of vertices
 * 
 * @param vertexList array of all distinct vertices to be randomised
 * @param n size of the vertexlist
 */
static void fisher_yates_shuffle(int vertexList[], int n);

/**
 * @brief prints error and exits
 * @details if wrong usage is detected, this function prints the correct one and 
 *          exits with an error
 */
static void usage_error(void);

/**
 * @brief opens shared memory and semaphores
 * @details opens shared memory and semaphores. Prints errors if any openings failed
 *          then exits with error.
 */
static void res_open(void);

/**
 * @brief closes shared memory and semaphores
 * @details closes shared memory and semaphores. Prints errors if any closings failed.
 *          Returns an exitcode;
 * 
 * @return int EXIT_SUCCESS if all closings were successful, EXIT_FAILURE otherwise
 */
static int res_close(void);

/**
 * @brief calls sem_wait in controlled environment
 * @details calls sem_wait with passed semaphore and checks if any interruption came from signals.
 *          Exits with error if semaphore failed otherwise
 * 
 * @param sem semaphore to call sem_wait on
 */
static void my_sem_wait(sem_t *sem);

int main(int argc, char **argv){
    name = argv[0];
    int argIter = 1;
    if(argc <= argIter) usage_error();
    Edge currentEdge = {.end_node=0,.start_node=0,.endFlag=0};
    Edge edgeList[argc];
    int vertexList[argc];
    edgeList[argc-1] = currentEdge;
    currentEdge.endFlag = 1;
    char *input;
    char *nextEdgePointer;

    //reads input Edges
    while (argIter < argc)
    {
        input = argv[argIter];
        currentEdge.start_node = strtol(input, &nextEdgePointer, 10);
        if(*nextEdgePointer != '-')usage_error();
        input = ++nextEdgePointer;
        currentEdge.end_node = strtol(input, &nextEdgePointer, 10);
    	if(*nextEdgePointer != '\0') usage_error();
        edgeList[argIter-1] = currentEdge;
        argIter++;
    }
    //open shared-memory and semaphores
    res_open();

    //builds vertex-list
    int temp = 0;
    int vcount = 0;
    while(temp < argIter-1){
        if(addVertex(vertexList, edgeList[temp].start_node, vcount)) vcount++;
        if(addVertex(vertexList, edgeList[temp].end_node, vcount)) vcount++;
        temp++;
    }

    //create new arcsets and write them to memory
    Fb_Arc_Set arcset;
    srand(time(0));
    while(!memory->exit){
        fisher_yates_shuffle(vertexList, vcount);
        if(calculate_fb_arc_set(vertexList, edgeList, &arcset, vcount)) continue;
        my_sem_wait(sem_write);
        my_sem_wait(sem_mutex);
        memcpy(memory->buffer + memory->index, &arcset, sizeof(Fb_Arc_Set));
        memory->index = (memory->index + 1) % SHM_MEM_INDEX;
        sem_post(sem_mutex);
        sem_post(sem_read);
        if(arcset.count == 0) break;
    }

    //close semaphores, unmap shared memory and close file-descriptor
    int EXIT_CODE = res_close();
    exit(EXIT_CODE);
}

int calculate_fb_arc_set(int vertexList[], Edge edgeList[], Fb_Arc_Set *arcset, int vcount){
    size_t i = 0;
    size_t j;
    Edge temp = edgeList[0];
    arcset->count = 0;
    while(temp.endFlag)
    {
        for (j = 0; j < vcount; j++)
        {
            if(temp.end_node == vertexList[j]){
                if(arcset->count+1 >= memory->max_edges) return 1;
                arcset->edges[arcset->count] = temp;
                arcset->count++;
                break;
            }
            if(temp.start_node == vertexList[j]){
                break;
            }
        }
        i++;
        temp = edgeList[i];
    }
    return 0;
}

int addVertex(int vertexList[], int vertex, int size){
    size_t i;
    for (i = 0; i <= size; i++)
    {   
        if(vertexList[i] == vertex){
            return 0;
        }
    }
    vertexList[size] = vertex;
    return 1;
}

void fisher_yates_shuffle(int vertexList[], int n){
    int i;
    int j;
    int temp;
    for (i = n-1; i > 0; i--)
    {
        j = rand() % (i+1);
        temp = vertexList[j];
        vertexList[j] = vertexList[i];
        vertexList[i] = temp; 
    } 
}

void usage_error(){
    fprintf(stderr, "ERROR in %s, wrong usage. USAGE: EDGE1...\nEDGE: x1-x2\nx: Integer\n", name);
    exit(EXIT_FAILURE);
}

void res_open(){
    if ((shm_fd = shm_open(SHM_NAME, O_RDWR, 0600)) == -1) {
		fprintf(stderr, "ERROR in %s, Failed to open shared memory\n", name);
        exit(EXIT_FAILURE);
	}
    if((memory = (Circular_Buffer*)mmap(NULL, sizeof(Circular_Buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == MAP_FAILED){
        fprintf(stderr, "ERROR in %s, Failed to map shared memory: %s\n", name, strerror(errno));
        if(close(shm_fd) == -1) fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        exit(EXIT_FAILURE);
    }
    if((sem_write = sem_open(SEM_WRITE_NAME, 0)) == SEM_FAILED ||
       (sem_read = sem_open(SEM_READ_NAME, 0)) == SEM_FAILED ||
       (sem_mutex = sem_open(SEM_MUTEX_NAME, 0)) == SEM_FAILED)
    {
        if(munmap(memory, sizeof(*memory)) == -1) fprintf(stderr, "ERROR in %s, Failed to unmap shared memory: %s\n", name, strerror(errno));
        if(close(shm_fd) == -1) fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        fprintf(stderr, "ERROR in %s, Failed to open semaphores: %s\n", name, strerror(errno));
        exit(EXIT_FAILURE); 
    }
}

int res_close(){
    int EXIT_CODE = EXIT_SUCCESS;
    //close semaphores
    if (sem_close(sem_write) == -1 ||
		sem_close(sem_read) == -1 ||
		sem_close(sem_mutex) == -1) 
    {
		fprintf(stderr, "ERROR in %s, Failed to close semaphores: %s\n", name, strerror(errno));
        EXIT_CODE = EXIT_FAILURE;
	}
    //unmap shared memory and close file-descriptor
    if(munmap(memory, sizeof(*memory)) == -1){
        fprintf(stderr, "ERROR in %s, Failed to unmap shared memory: %s\n", name, strerror(errno));
        EXIT_CODE = EXIT_FAILURE;
    }
    if(close(shm_fd) == -1){
        fprintf(stderr, "ERROR in %s, Failed to close shared-memory file-descriptor\n", name);
        EXIT_CODE = EXIT_FAILURE;
    }
    return EXIT_CODE;
}

void my_sem_wait(sem_t *sem){
    while(!memory->exit){
        if(sem_wait(sem) == -1){
            if(errno == EINTR){
                continue;
            }else{
                fprintf(stderr, "ERROR in %s, sem_wait() failed with %s\n", name, strerror(errno));
                res_close();
                exit(EXIT_FAILURE);
            }
        }else{
            return;
        }
    }
}
