/**
*@file generator.c
*@author daniel brauneis 12021357
*@details tries to find the least possible amount of edges of graph. To make it acyclic if they are removed
*@date 2021-11-07
*Main Program
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h> 
#include <stdio.h> 
#include <sys/mman.h>
#include <sys/types.h> 
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#define SHM_NAME "/e12021357myshm"
#define SEM_FREE "/e12021357sem_free"
#define SEM_USED "/e12021357sem_used"

//struct for a single edge
typedef struct {int a,b;} edge;
typedef struct {edge edges[sizeof(edge)*8]; int size;} arcset;
struct arcset {
    edge edges[sizeof(edge)*8];
    int size;
};
static edge transformArgToEdge(char *str, int *nodesOfGraph);
static int isIntNotInArray(int *array, int value);
static int *shuffleArray(int *array);
static arcset findarcset(int *shuffleArray,edge *graph, int givenEdges);

//current index of empty filed in nodesofGraph array (size of array)
static int sizeNodesOfGraph = 0;
volatile sig_atomic_t quit = 0;

/**
* Main Function
* @brief This is the main function of the program
* @param arc number of command-line arguments
* @param argv list of arguments
*/
int main(int argc, char **argv){
     if(argc == 1){
        fprintf(stderr, "at least one edge must be given");
        exit(EXIT_FAILURE);
    }  
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0777);
    arcset *e12021357myshm;
    e12021357myshm = mmap(NULL, sizeof(arcset)*3, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (e12021357myshm == MAP_FAILED){
        fprintf(stderr, "Error while mapping");
        exit(EXIT_FAILURE);
    } 
    if (close(shmfd) == -1) exit(EXIT_FAILURE);
    //Sem_open
    sem_t *e12021357free =sem_open(SEM_FREE, O_RDWR);
    sem_t *e12021357used =sem_open(SEM_USED, O_RDWR);
    //Code

    int wr_pos = 0;
    int exitchecker = 0;
    while(!quit){
        sizeNodesOfGraph = 0;
        edge *graph = malloc(sizeof(edge) * argc-1);
        int *nodesOfGraph = malloc(sizeof(int)* (argc-1) *2); //max amount of possible nodes 
        for(int i = 1; i < argc;i++){
            graph[i-1] = transformArgToEdge(argv[i], nodesOfGraph);
        }
        int *shuffledArray = shuffleArray(nodesOfGraph);
        arcset foundset = findarcset(shuffledArray, graph, argc-1);
        printf("checking on pos: %d\n", exitchecker);
        if(e12021357myshm[exitchecker].size == 999999999){
            quit = 1;
            continue;
        } 
        if(foundset.size >= 8){
            exitchecker++;
            exitchecker %= sizeof(e12021357myshm)-1;
            printf("No solution found\n");
            continue;
        }
	    printf("waiting\n");
        sem_wait(e12021357free);
        e12021357myshm[wr_pos] = foundset;
        printf("foundset(%d): \n", foundset.size);
        for(int i = 0; i < foundset.size;i++){
            printf("%d-%d \n", e12021357myshm[wr_pos].edges[i].a, e12021357myshm[wr_pos].edges[i].b);
        }
        sem_post(e12021357used);
        wr_pos += 1;
        wr_pos %= sizeof(e12021357myshm)-1;

        exitchecker++;
        exitchecker %= sizeof(e12021357myshm)-1;
    }

    if(munmap(e12021357myshm, sizeof(*e12021357myshm)*3) == -1) exit(EXIT_FAILURE);
    //Sem_close    
    sem_close(e12021357free);
    sem_close(e12021357used);
     
    printf("exiting\n");
    exit(EXIT_SUCCESS);
}

/**
* transformArgToEdge Function
* @brief Transforms the given string to an edge and adds the nodes to the Array nodesOfGraph
* @param str string of format [0-9]*-[0-9]*
* @return edge
*/
static edge transformArgToEdge(char *str, int *nodesOfGraph){
    //finds index of '-' in str
    int index = strchr(str, '-') - str;
    int len = strlen(str);
    char *ach = malloc(index);
    char *bch = malloc(len-index);
    for(int i = 0; i < index;i++){
        ach[i] = str[i];
    } 
    for(int i = index+1; i < len;i++){
        bch[i-index-1] = str[i];
    } 
    char *ptr;
    int a = strtol(ach, &ptr, 10);
    int b = strtol(bch, &ptr, 10);
    //workaround for 0 values, since the array is prefilled with 0's
    if(a == 0) a= 999999999;
    if(b == 0) b= 999999999;
    edge ret = {a, b};
    if(isIntNotInArray(nodesOfGraph, a)) nodesOfGraph[sizeNodesOfGraph++] = a;
    if(isIntNotInArray(nodesOfGraph, b)) nodesOfGraph[sizeNodesOfGraph++] = b;
    return ret;
}

/**
* isIntNotInArray Function
* @brief checks wheter a value is in an array or not
* @param array search array
* @param value value to be looked for
* @return 1 if the value is NOT in given array, 0 if otherwise
*/
static int isIntNotInArray(int *array, int value){
    for(int i = 0; i < sizeof(array);i++){
        if(array[i] == value) return 0;
    }
    return 1;
}

/**
* shuffleArray Function
* @brief returns a shuffled Version of the given Array
* @param array to be shuffled
* @return shuffled array
*/
static int *shuffleArray(int *array){
    int *copiedArray = malloc(sizeNodesOfGraph);
    pid_t psid = getpid();
    //current time as seed
    srand(time(0)*psid);
    for(int i = 1; i <= sizeNodesOfGraph;i++){
        int k = rand()%sizeNodesOfGraph;
        if(array[k] != -1){
            copiedArray[sizeNodesOfGraph-i] = array[k];
            array[k]=-1;
        }else{
            i--;
        }        
    }
    return copiedArray;
}

static arcset findarcset(int *shuffleArray,edge *graph, int givenEdges){
    edge edges[sizeof(edge)*8]; //max size of arcset = 8
    int sizeofArcSet = 0;
    for(int i = 0; i < givenEdges;i++){
        if(sizeofArcSet >= 8){
            arcset err = {{{0}},99};
            return err;
        }
        edge selectedEdge = graph[i];
        int a = selectedEdge.a;
        int b = selectedEdge.b;
        int foundB = 0;
        for(int x = 0; x < sizeNodesOfGraph;x++){
            if(shuffleArray[x] == b){
                foundB = 1;
            }else if(foundB){
                if(shuffleArray[x] == a){
                    edges[sizeofArcSet] = selectedEdge;
                    sizeofArcSet++;
                }
            }           
        }
    }
    arcset ret = {{{0}}, sizeofArcSet};
    memcpy(ret.edges, edges, sizeof(edges));
    return ret;
}
