/**
 * @file generator.c
 * @author Leonhard Ender (12027408)
 * @date 14.11.2021
 *
 * @brief Generates solutions to the feedback arc problem and
 * passes them to the supervisor.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h> 
#include "circ_buf.h"

/** A global variable holding the name of the program. */
char *myprog;

/**
 * @brief Print the error to stderr and exit.
 * @details Print out the program name, followed by
 * the err passed to the function and the string
 * representation of the errorcode in errno.
 * 
 * global variables: myprog
 */
static void print_error(char *err) {
    fprintf(stderr, "%s: %s: %s\n", myprog,err, strerror(errno));
}

/**
 * @brief Print the usage to stderr and exit.
 * @details global variables: myprog
 */
static void usage(void) {
    fprintf(stderr,"Usage: %s [node1-node2]... \n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief Shuffle the array using the Fisher-Yates algorithm.
 * @details Starting with the last element, each element
 * of the array is swapped with a random element with a lower
 * index.
 * @param arr The array that is to be shuffled.
 * @param size The size of the array.
 */
static void shuffle(int *arr, int size){
    int random;
    int temp;
    srand (time(NULL)); // initialize random number generation
    for (int i=size-1; i>0; i--) {
        random = rand()%i;
        temp = arr[random];
        arr[random] = arr[i];
        arr[i]=temp;
    }
}

/**
 * @brief Generate a solution to the feedback arc
 * problem on the graph specified by edges and nodes.
 * 
 * @details For a graph with x nodes, the nodes are represented
 * by their indices ranging from 0 to x-1. This means 
 * that the shuffled list of nodes can be interpreted
 * as a lookup table for the position of a node in a
 * permutation, e.g. if nodes[0] = 5, this means that
 * the node five has the position 0.
 * Using this, all edges (u,v) where the position of 
 * u is lower than the position of v (i.e. u comes
 * before v) are selected and written into the feedback
 * arc. If the number of edges exceeds 8, the function
 * stops and returns -1, otherwise it returns 0.
 * 
 * @param edges Array containing all edges of the graph.
 * @param edgecount The number of edges (length of edges).
 * @param nodes Array containing all nodes of the graph.
 * @param nodecount The number of nodes (length of nodes).
 * @param arc The feedback arc where the solution is written to.
 * @return -1 if solution has more than 8 edges, 0 otherwise.
 */
static int gen_feedback_arc(edge *edges, int edgecount, int *nodes, int nodecount, feedback_arc *arc ) {
    shuffle(nodes,nodecount); // shuffle nodes
    arc->edge_count = 0;
    int j = 0;

    for (int i = 0; i<edgecount; i++){
        if (nodes[edges[i].node1] < nodes[edges[i].node2]){
            if (j>7){
                return -1; // more than 8 edges were found
            }
            arc->edges[j].node1 =  edges[i].node1;
            arc->edges[j].node2 =  edges[i].node2;
            j++;
            arc->edge_count = j;
        }
    }
    return 0;
}

/**
 * @brief Parse the parameters (specifying the graph),
 * open the shared memory buffer and start generating
 * random solutions for the feedback arc problem and 
 * writing them to the buffer.
 * @details The graph is passed to the program by passing
 * each edge as an argument of the form x-y, specifying
 * an edge from the node x to the node y. Nodes must be 
 * represented by numbers (>= 0). It is assumed that there
 * is a node for every number between (including) zero and
 * the highest node(-number).
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
    myprog = argv[0]; // program name
    printf("This is a generator.\n");

    /**** parameter parsing *******/

    if (argc == 1) { // no parameters
            usage();
    }

    char * p;
    long int li1;
    long int li2;
    int maxindex = 0;
    errno=0;
    edge edges[argc-1];

    for (int i = 1; i< argc; i++) {

        /* convert first part of the argument to int */
        li1 = strtol (argv[i],&p,10); 
        /* check that conversion is valid and next character is '-'*/
        if (*p!='-' || (li1 == 0 && (errno!=0 || p==NULL ))){
            usage();
        }

        /* second part of argument */
        li2 = strtol (p+1,NULL,10);    
        if ((li2 == 0 && errno!=0) || *(p+1)=='\0' ){
            usage();
        }

        /* check if one of the nodes has a higher index than the previous ones */
        if (li1 > maxindex)
            maxindex = li1;

        if (li2 > maxindex)
            maxindex = li2;

        edges[i-1].node1 = li1;
        edges[i-1].node2 = li2;
    }


    /**** setup feedback arc generation ****/
    
    /* there are nodes from 0 to maxindex, so there are maxindex+1 in total */
    int nodes[maxindex+1];

    for (int i = 0; i<=maxindex; i++)
        nodes[i]=i;

    int edgecount = argc-1,  nodecount  = maxindex+1;


    /**** shared memory *******/

    int buf_fd; // file descriptor for buffer
    circ_buf *buf; // mapped buffer
    sem_t *buf_free_space, *buf_used_space, *buf_mutex; // semaphores

    /* open shared memory buffer */
    if ((buf_fd = shm_open("/12027408_buf",O_RDWR , 0 )) == -1){
        print_error("shm_open failed");
        exit(EXIT_FAILURE);
    }

    /* map shared memory buffer */
    if ((buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0))==MAP_FAILED) {
        print_error("mmap failed");
        close(buf_fd);
        shm_unlink("/12027408_buf");
        exit(EXIT_FAILURE);
    }

    close(buf_fd); // file can be closed at this point

    /* semaphore indicating how much free space is available */
    if ((buf_free_space = sem_open("/12027408_buf_free_space", 0))==SEM_FAILED) {
        print_error("sem_open failed");
        munmap(buf,  sizeof(*buf));
        shm_unlink("/12027408_buf");
    }   
    /* semaphore indicating how much space is currently used */
    if ((buf_used_space = sem_open("/12027408_buf_used_space", 0))==SEM_FAILED) {
        print_error("sem_open failed");
        munmap(buf,  sizeof(*buf));
        sem_unlink("/12027408_buf_free_space");
        shm_unlink("/12027408_buf");
    }
    /* semaphore indicating is someone is currently writing to the buffer */
    if ((buf_mutex = sem_open("/12027408_buf_mutex", 0))==SEM_FAILED) {
        print_error("sem_open failed");
        munmap(buf,  sizeof(*buf));
        sem_unlink("/12027408_buf_free_space");
        sem_unlink("/12027408_buf_used_space");
        shm_unlink("/12027408_buf");
    }

    /***** feedback arc generation *****/

    feedback_arc new_arc;
    while (1) {
        /* generate feedback arcs until one with 8 edges or less is found */
        while( gen_feedback_arc(edges, edgecount, nodes, nodecount,&new_arc) == -1) {
            /* check if the supervisor has shut down */
            if (buf->shutdown != 0)
                break; 
        }

        /* check if free space in the buffer is available, otherwise wait */
        sem_wait(buf_free_space);

        /* check if the supervisor has shut down */
        if (buf->shutdown != 0)
            break; 
                
        /* wait for write access to buffer */
        sem_wait(buf_mutex);

        /* push solution onto the shared memory buffer */
        buf_push(buf,new_arc);

        /* give up write access to buffer */
        sem_post(buf_mutex);

        /* there is now less space available on the buffer */
        sem_post(buf_used_space);
    }

    /****** wrapping up *****/

    fprintf(stdout,"shutting down \n");
    sem_close(buf_free_space);
    sem_close(buf_used_space);
    sem_unlink("/12027408_buf_free_space");
    sem_unlink("/12027408_buf_used_space");
    sem_unlink("/12027408_buf_mutex");
    munmap(buf,  sizeof(*buf));
    shm_unlink("/12027408_buf");
    return(EXIT_SUCCESS);

}
