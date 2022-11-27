#include <sys/mman.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <fcntl.h> 
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "generator.h"

/** 
 * @author Florian Jäger (11810847)
 * @brief removes randomized edge sets and writes to shared memory
 * @date 2021-11-13
 */

static struct edge result[MAX_SOLUTION];
static struct edge edges[MAX_DATA];

struct shm {
    unsigned int state;
    struct edge results[BUFFER_SIZE][MAX_SOLUTION];
};

/**
    @brief randomizes nodes-array and removes edges(u,v) where v < u
    @param edgeCount: count of edges
    @param nodes: nodes-array
    @param nodeCount: count of nodes-array
    @return count of removed edges
*/
static int findNewSolution(int edgeCount,int *nodes, int nodeCount) {
    srand(time(0));
    int i;
    //randomize nodes-array
    for (i = nodeCount-1; i > 0; i--) {
        int j = rand() % (i+1);
        int temp = nodes[i];
        nodes[i] = nodes[j];
        nodes[j] = temp;
    }

    int setCount = 0;
    int v1,v2;
    int pos1,pos2;
    //find edges to remove
    for(int i = 0; i < edgeCount; i++) {
        v1 = edges[i].v1;
        v2 = edges[i].v2;
        for(int j = 0; j < nodeCount; j++) {
            if(v1 == nodes[j]) pos1 = j;
            if(v2 == nodes[j]) pos2 = j;
        }
        if(pos1 > pos2) {
            if(setCount == MAX_SOLUTION) return -1; 
            //add edge to removed set
            result[setCount].v1 = v1;
            result[setCount].v2 = v2;
            setCount++;
            
        }
    }
    return setCount;
}
/**
    @brief resets results-array
*/
static void reset() {
    memset(result, 0, sizeof(result));
}

/**
    @brief adds new node to nodes-array
    @param nodes: nodes-array
    @param nodeCount: count of elements in nodes-array
    @param v: to add node
*/
static void addNode(int *nodes, int *nodeCount, int v) {
    for(int i = 0; i < *nodeCount; i++) {
        if (nodes[i] == v) return;
    }
    nodes[*nodeCount] = v;
    *nodeCount = *nodeCount + 1;
    
    return;
    } 

int main(int argc, char* argv[]) {
    int edgeCount = 0;
    int nodes[MAX_DATA];
    int nodeCount = 0;
    int setCount = 0;

    if ((argc) == 1) {
        fprintf(stderr, "%s: No Edges specified\n",argv[0]);
        return EXIT_FAILURE;
    } else {
        //get all edges
        for(int i = 1; i < argc; i++) {
            int v1,v2;
            sscanf(argv[i],"%d-%d",&v1,&v2);
            edges[i-1].v1 = v1;
            edges[i-1].v2 = v2;
            edgeCount++;
            //add nodes
            addNode(nodes,&nodeCount,v1);
            addNode(nodes,&nodeCount,v2);
        }
            
        int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
        if (shmfd == -1) {
                perror("shm_open error");
                return 1;
            }
        //connect to shared memory
        struct shm *shm;
        shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
        if (shm == MAP_FAILED) {
            perror("mmap error");
            return EXIT_FAILURE;
        }

        sem_t *s1 = sem_open(SEM_1, 0);
        sem_t *s2 = sem_open(SEM_2, 0);
        sem_t *s3 = sem_open(SEM_3, 0);

        int wr_pos = 0;
        int quit = 0;
        while(!quit) {
            if(quit == 1) break;
            //sem_wait(s3);
            if(sem_wait(s1) == -1) {
                break;
            }
            //find new solution
            setCount = findNewSolution(edgeCount, nodes, nodeCount);
            printf("\nSolution: ");
            for(int i = 0; i < setCount; i++) {
                int v1 = result[i].v1;
                int v2 = result[i].v2;
                printf("%i-%i ",v1,v2);
            }
            printf("\n");
            if(setCount != -1) {
                if(sem_wait(s3) == -1) {
                break;
                }
                //write solution
                memcpy(shm->results[wr_pos],result,sizeof(result));
                reset();
                wr_pos += 1;
                wr_pos %= BUFFER_SIZE;
                
                
            } else {
                printf("Solution too big\n");
                reset();
            }
            
            //reset(setCount);
            sem_post(s2);
            sem_post(s3);

            quit = shm->state;
        }

        printf("\nClosing\n");
        //unmap shared memory
        if (munmap(shm, sizeof(*shm)) == -1) {
        perror("error unmap shared memory");
        return EXIT_FAILURE;
        }
        //close semaphores
        sem_close(s1);
        sem_close(s2);
        sem_close(s3);
        return EXIT_SUCCESS;
    }
}