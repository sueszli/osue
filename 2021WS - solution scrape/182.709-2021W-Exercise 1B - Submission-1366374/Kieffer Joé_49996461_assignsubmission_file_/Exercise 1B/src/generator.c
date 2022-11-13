/**
 * @file generator.c
 * @author Kieffer Joé <e11814254@student.tuwien.ac.at>
 * @date 08.11.2021
 * 
 * @brief One of the main program modules
 * 
 * This program implements the generator which colors the graph randomly and write which edges to remove from the graph to make it 3 colorable
 * to the shared memory.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "shared.h"

#define NCOLOR (3)

const char *progName;
struct myshm *myshm;

sem_t *sem_Free;
sem_t *sem_Used;
sem_t *sem_Mutex;

/**
 * @brief This function closes a semaphore
 * 
 * @details This function closes the indicated semaphore and unlinks it. On error messages are displayed and 1 is returned.
 * 
 * @param sem the semaphore to close
 * @param sem_Name the name of the semaphore to close
 * @return 0 on success and 1 if at least 1 of close or unlink failed
 */
int closeSemaphore(sem_t *sem, char *sem_Name){
    int closed = sem_close(sem);
    if(closed == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't close %s! sem_close failed: %s\n", progName, __LINE__, sem_Name, strerror(errno));
        return 1;
    }
    return 0;
}

/**
 * @brief This function closes all semaphores
 * 
 * @details This function tries to close all semaphores with the function closeSemaphore. If at least one has an error this function ends the program after trying to close
 * all semaphores.
 */
void closeSemaphores(void){
    int i = closeSemaphore(sem_Free, SEM_FREE);
    i+= closeSemaphore(sem_Used, SEM_USED);
    i+= closeSemaphore(sem_Mutex, SEM_MUTEX);
    if(i >0){
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief unmaps the shared memory
 * 
 * @details This function unmaps the shared memory from the generator and prints an error message if unsuccessfull.
 */
void closeSharedMemory(void){
    int unmapped = munmap(myshm, sizeof(*myshm));
    if(unmapped == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't unmap shared memory! munmap failed: %s\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This method closes the filedescriptor
 * 
 * @details This method assumes it is called with a valid filedescriptor and closes it. Should the close function not succeed, an error message
 * is displayed.
 * 
 * @param fd The filedescriptor to close
 */
void closeFd(int fd){
    assert(fcntl(fd, F_GETFD) != -1);
    int closed = close(fd);
    if(closed == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't close file descriptor! close failed: %s\n", progName, __LINE__, strerror(errno));
        closeSharedMemory();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This function cleansup the ressources
 * 
 * @details This function calls the functions to close the shared memory and the semaphores
 */
void cleanup(void){
    closeSharedMemory();
    closeSemaphores();
}

/**
 * @brief This function colors the graph
 * 
 * @details This function colors every node of the graph randomly. The macro NCOLOR is used to determine how much colors should be used
 * 
 * @param nodes an array with all the nodes of the graph and all the colors of these nodes (if already colored)
 * @param nNodes the number of nodes in the nodes array
 */
void colorGraph(int nodes[][2], int nNodes){
    for(int i=0; i< nNodes; i++){
        nodes[i][1] = (rand() % NCOLOR);
    }
}

/**
 * @brief This function writes solutions to the shared memory
 * 
 * @details This function gets an array with all the nodes and the color of these and an array with all the edges of the graph including
 * a reference of the nodes of these edges where they are in the nodes array. While the quit flag is not set in the shared memory this function
 * repeadetly colors the graph. Makes an array with all the edges where the nodes have the same length and counts these edges. Then this function
 * tries to get the semaphore for free space and then the semaphore for exclusive memory access. Finally it writes all the edges of the created
 * array in the shared memory and the number of edges too before it increments the write position in the shared memory and releases the
 * accuired semaphores.
 * 
 * @param edges all the edges of the graph
 * @param nEdge the argument counter so the number of edges +1
 * @param nodes the nodes of the graph with their color
 * @param nNodes the number of different nodes
 */
void writeToSharedMemory(int edges[][2][2], int nEdge, int nodes[][2], int nNodes){
    nEdge--; //argc is given but first argument is the programname and no edge therefore -1
    while(!(myshm->quit)){
        colorGraph(nodes, nNodes);
        int writeOut[nEdge][2];
        int nWriteOut = 0;
        for(int i=0; i < nEdge; i++){
            int posN1 = edges[i][0][1];
            int posN2 = edges[i][1][1];
            //Compare the two colors of connected nodes
            if(nodes[posN1][1] == nodes[posN2][1]){
                writeOut[nWriteOut][0] = edges[i][0][0];
                writeOut[nWriteOut][1] = edges[i][1][0];
                nWriteOut++;
            }
        }
        if(sem_wait(sem_Free) == -1){
            if(errno == EINTR){
                continue;
            }
        }
        if(sem_wait(sem_Mutex) == -1){
            if(errno == EINTR){
                sem_post(sem_Free);
                continue;
            }
        }
        for(int i=0; i < nWriteOut; i++){
            myshm->data[myshm->writePos][i][0] = writeOut[i][0];
            myshm->data[myshm->writePos][i][1] = writeOut[i][1];
        }
        myshm->ndata[myshm->writePos] = nWriteOut;
        myshm->writePos++;
        myshm->writePos %= MAX_DATA;
        sem_post(sem_Mutex);
        sem_post(sem_Used);
        
        
    }
    sem_post(sem_Free);
}

/**
 * @brief This function checks if a semaphore was successfully opened
 * 
 * @details This function gets a semaphore and the semaphore name and checks if the semaphore has opened, if not it calls the function to close
 * the shared memory and to close all semaphores (also the ones which may not be open)
 * 
 * @param sem the semaphore to check 
 * @param sem_Name the name of the semaphore to check 
 */
void checkOpenSemaphore(sem_t *sem, char *sem_Name){
    if(sem == SEM_FAILED){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't open %s! sem_open failed: %s\n", progName, __LINE__, sem_Name, strerror(errno));
        closeSharedMemory();
        closeSemaphores();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This function opens the shared ressources
 * 
 * @details This function opens the shared memory and the semaphores created by the supervisor module. If the shared memory is not successfully opened or mapped
 * the function displays an error message. The function also calls the functions to check if the semaphores are opened correctly.
 */
void openSharedRessources(void){
    int fd = shm_open(SHM_NAME, O_RDWR, 0600);
    if(fd == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't open shared memory! shm_open failed: %s\n Try to execute supervisor first!\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    closeFd(fd);
    if(myshm == MAP_FAILED){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't map shared memory! mmap failed: %s\n", progName, __LINE__, strerror(errno));    
        closeSharedMemory();
        exit(EXIT_FAILURE);
    }
    sem_Free = sem_open(SEM_FREE, O_RDWR);
    checkOpenSemaphore(sem_Free, SEM_FREE);
    sem_Used = sem_open(SEM_USED, O_RDWR);
    checkOpenSemaphore(sem_Used, SEM_USED);
    sem_Mutex = sem_open(SEM_MUTEX, O_RDWR);
    checkOpenSemaphore(sem_Mutex, SEM_MUTEX);
}

/**
 * @brief This function checks an edge
 * 
 * @details This function gets an edge and checks if the format is correct, if not an appropriate error message is displayed and the program exits after calling the
 * cleanup function to close shared ressources.
 * 
 * @param edge the edge to check
 */
void checkedge(char *edge){
    char *limiter = strstr(edge, "-");
    if(limiter == NULL){
        fprintf(stderr, "[%s:%i] ERROR: Edges must be splitted by -, %s is invalid!\n", progName, __LINE__, edge);
        exit(EXIT_FAILURE);
    }
    int limPos = -1;
    for(int i = 0; i < strlen(edge); i++){
        if(edge[i] == '-'){
            if(limPos == -1){
                limPos = i;
            }else{
                fprintf(stderr, "[%s:%i] ERROR: Edges must contain maximum one -, %s is invalid!\n", progName, __LINE__, edge);
                cleanup();
                exit(EXIT_FAILURE);
            }
        }else{
            if(edge[i] < '0' || edge[i] > '9'){
                fprintf(stderr, "[%s:%i] ERROR: Nodes must contain Numbers, %s is invalid!\n", progName, __LINE__, edge);
                cleanup();
                exit(EXIT_FAILURE);
            }
        }
    }
    if(limPos < 1 || limPos >= strlen(edge)){
        fprintf(stderr, "[%s:%i] ERROR: Edge must contain 2 Nodes limited by -, %s is invalid!\n", progName, __LINE__, edge);
        cleanup();
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This function adds the nodes of the graph to an array
 * 
 * @details This function adds a given node to a given array if not already contained.
 * 
 * @param node The node which should be added
 * @param nodes The array with all the known nodes and where to add the new node
 * @param nNodes The number of nodes already in the array
 * @return The position of the node in the nodes array (nNodes if it was a new node) 
 */
int addNode(int node, int nodes[][2], int nNodes){
    for(int i = 0; i < nNodes; i++){
        if(nodes[i][0] == node){
            return i;
        }
    }
    nodes[nNodes][0] = node;
    return nNodes;
}

/**
 * @brief This function reads all the the edges
 * 
 * @details This function reads all the edges given at the program start. First a function is called to check the correct format.
 * After that the edge is splitted and the first node is read and changed in another format. Then the position of the nodes in a
 * given array is looked up. If not in this array, the node will be added. If a new node was added this function increments the
 * counter for the number of nodes. The nodes are also added to an edge array which has all the edges in a number format.
 * 
 * @param argc The number of arguments given at program start
 * @param argv The arguments given at program start
 * @param nodes An array to save the nodes in and later the color
 * @param edges An array to save the edges in and the reference position of the nodes in the nodes array
 * @return the number of nodes in the nodes array
 */
int readEdges(int argc, char *argv[], int nodes[][2], int edges[][2][2]){
    int nNodes = 0;
    for (int i = 1; i < argc; i++){
        checkedge(argv[i]);
        char *nodeChar = strtok(argv[i], "-");
        int node = strtol(nodeChar, NULL, 10);
        int pos = addNode(node, nodes, nNodes);
        if(pos == nNodes){
            nNodes++;
        }
        int edgePos = i - 1;
        edges[edgePos][0][0] = node;
        edges[edgePos][0][1] = pos;
        nodeChar = strtok(NULL,"-");
        node = strtol(nodeChar, NULL, 10);
        pos = addNode(node, nodes, nNodes);
        if(pos == nNodes){
            nNodes++;
        }
        edges[edgePos][1][0] = node;
        edges[edgePos][1][1] = pos;
    }
    return nNodes;
}

/**
 * @brief The main function
 * 
 * @details This function calls the functions needed to run this program. This functions
 * open the shared memory and the semaphores, read the given edges, writes to the shared memory and closes the shared memory and the semaphores.
 * 
 * @param argc The argument counter
 * @param argv The arguments given at program start
 * @return EXIT_SUCCESS if successfull, else EXIT_FAILURE
 */
int main(int argc, char *argv[]){
    progName = argv[0];
    openSharedRessources();

    srand(getpid()*time(NULL));
    int nodes[2*(argc-1)][2];
    int edges[(argc-1)][2][2];
    int nNodes = readEdges(argc, argv, nodes, edges);
    writeToSharedMemory(edges, argc, nodes, nNodes);

    cleanup();
    return EXIT_SUCCESS;
}