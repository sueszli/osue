/**
 * @file supervisor.c
 * @author Leonhard Ender (12027408)
 * @date 14.11.2021
 *
 * @brief Sets up a buffer in shared memory
 * where generators can push solutions, then
 * checks these solutions and retains the best
 * one.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include "circ_buf.h"

/** A global variable holding the name of the program. */
char *myprog;


circ_buf *buf; // mapped buffer

/**
 * @brief Print the usage to stderr and exit.
 * @details global variables: myprog
 */
static void usage(void) {
    fprintf(stderr,"Usage: %s\n", myprog);
    exit(EXIT_FAILURE);
}

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
 * @brief Handle a SIGTERM or SIGINT signal by freeing all
 * resources and exiting.
 */
static void handle_signal(int signal) {
    buf->shutdown = 1;
    fprintf(stdout,"shutting down\n");
    sem_unlink("/12027408_buf_free_space");
    sem_unlink("/12027408_buf_used_space");
    sem_unlink("/12027408_buf_mutex");
    shm_unlink("/12027408_buf");
    exit(EXIT_SUCCESS);
}

/**
 * @brief Print a feedback arc to stdout.
 * @param fa The feedback arc.
 */
static void print_feedback_arc(feedback_arc fa) {
    fprintf(stdout, "Solution with %d edges:\n", fa.edge_count);
    for (int i = 0; i<fa.edge_count; i++) {
        fprintf(stdout, "%d-%d\n", fa.edges[i].node1,fa.edges[i].node2);
    }
}

/**
 * @brief Configure signal handling, set up shared memory
 * and semaphores, then read the circular buffer and print
 * out solutions, should they be improvements. Exit when
 * either the graph is found to be acyclic or a SIGTERM or
 * SIGINT signal occurs. No arguments expected. 
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
    myprog = argv[0]; // program name
    
    if (argc>1) {
        usage();
    }

    /* configure handling of SIGINT and SIGTERM signals */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;

    sigaction(SIGINT, &sa, NULL);   
    sigaction(SIGTERM, &sa, NULL);
 

    int buf_fd; // file descriptor for buffer
    sem_t *buf_free_space, *buf_used_space, *buf_mutex; // semaphores

    /* create shared memory buffer */
    if ((buf_fd = shm_open("/12027408_buf",O_RDWR | O_CREAT, 0600 )) == -1){
        print_error("shm_open failed");
        exit(EXIT_FAILURE);
    }

    /* set size of shared memory buffer */
    if (ftruncate(buf_fd, sizeof(circ_buf)) < 0) {
        print_error("ftruncate failed");
        shm_unlink("/12027408_buf");
        exit(EXIT_FAILURE);
    }

    /* map shared memory buffer */
    if ((buf = mmap(NULL, sizeof(*buf), PROT_READ | PROT_WRITE, MAP_SHARED, buf_fd, 0))==MAP_FAILED) {
        print_error("mmap failed");
        close(buf_fd);
        shm_unlink("/12027408_buf");
        exit(EXIT_FAILURE);
    }

    close(buf_fd); // at this point the file can be closed

    /* semaphore indicating how much free space is available */
    if ((buf_free_space = sem_open("/12027408_buf_free_space", O_CREAT,0600,32))==SEM_FAILED) {
        print_error("sem_open failed");
        munmap(buf,  sizeof(*buf));
        shm_unlink("/12027408_buf");
    }   
    /* semaphore indicating how much space is currently used */
    if ((buf_used_space = sem_open("/12027408_buf_used_space", O_CREAT,0600,0))==SEM_FAILED) {
        print_error("sem_open failed");
        munmap(buf,  sizeof(*buf));
        sem_unlink("/12027408_buf_free_space");
        shm_unlink("/12027408_buf");
    }
    /* semaphore indicating is someone is currently writing to the buffer */
    if ((buf_mutex = sem_open("/12027408_buf_mutex", O_CREAT,0600,1))==SEM_FAILED) {
        print_error("sem_open failed");
        munmap(buf,  sizeof(*buf));
        sem_unlink("/12027408_buf_free_space");
        sem_unlink("/12027408_buf_used_space");
        shm_unlink("/12027408_buf");
    }


    /* best solution so far (lowest number of edges) */
    feedback_arc best; 
    best.edge_count = 8;

    /* initialize circular buffer */
    buf->head = 0;
    buf->tail = 0;
    buf->shutdown = 0;
    feedback_arc new;

    while (1) {
        /* wait while buffer is empty */
        sem_wait(buf_used_space);
        
        /* pop item from buffer */
        buf_pop(buf,&new);
        if (new.edge_count == 0){ // solution has no edges
            fprintf(stdout,"The graph is acyclic!\n");
            break;
        }
        if (new.edge_count < best.edge_count) { // solution has less edges than current best
            best = new;
            print_feedback_arc(best);
        }

        sem_post(buf_free_space); // space has been freed up
        
    }


    buf->shutdown = 1; // indicate the shutdown to generators
    sem_close(buf_free_space);
    sem_close(buf_used_space);
    sem_close(buf_mutex);
    sem_unlink("/12027408_buf_free_space");
    sem_unlink("/12027408_buf_used_space");
    sem_unlink("/12027408_buf_mutex");
    munmap(buf,  sizeof(*buf));
    shm_unlink("/12027408_buf");
    return(EXIT_SUCCESS);
    
}
