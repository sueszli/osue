/**
 * @file supervisor.c
 * @author David Jahn, 12020634
 * @brief The supervisor-program
 * @version 1.0
 * @date 2021-11-14
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include "circular_buffer.h"

#define SHM_CIRC_BUFFER "12020634_circ_buf1"
#define MY_SEM_READ "12020634_sem_read1"
#define MY_SEM_WRITE "12020634_sem_write1"

/**
 * @brief variable used for signal handling
 * 
 */
volatile sig_atomic_t quit = 0;
/**
 * @brief Programm name
 * 
 */
const char *PROGRAM_NAME;

/**
 * @brief Semaphore for writing in the buffer
 * 
 */
sem_t *writeSem;
/**
 * @brief Semaphore for reading out of the buffer
 * 
 */
sem_t *readSem;

/**
 * @brief Shared memory circular Buffer
 * 
 */
circular_buffer_t *buffer;
/**
 * @brief Shared memory file descriptor
 * 
 */
int shmfd;

/**
 * @brief Method to close all Ressources if an error occures
 * 
 */
void closeRessourcesOnError()
{
    close(shmfd);
    munmap(buffer, sizeof(*buffer));
    shm_unlink(SHM_CIRC_BUFFER);
    //---- close semaphores
    sem_close(readSem);
    sem_close(writeSem);
    sem_unlink(MY_SEM_READ);
    sem_unlink(MY_SEM_WRITE);
}

/**
 * @brief A Mehtod to call if an error occures. It prints an error message and closes all ressources.
 * 
 * @param errorMsg Error Message to print
 */
void onError(char *errorMsg)
{
    fprintf(stderr, "An error occured in %s: ", PROGRAM_NAME);
    fprintf(stderr, errorMsg);
    fprintf(stderr, strerror(errno));
    fflush(stderr);
    closeRessourcesOnError();
    exit(EXIT_FAILURE);
}

/**
 * @brief Method to close all Ressources in a normal way.
 * 
 */
void closeRessourcesNormal()
{
    if (close(shmfd) == -1)
    {
        onError("Error while close of shm");
    }
    if (munmap(buffer, sizeof(*buffer)) == -1)
    {
        onError("Error while munmap");
    }
    if (shm_unlink(SHM_CIRC_BUFFER) == -1)
    {
        onError("Error while shm_unlink");
    }
    //---- close semaphores
    if (sem_close(readSem) == -1)
    {
        onError("Error while sem_close");
    }
    if ((sem_close(writeSem)) == -1)
    {
        onError("Error while sem_close");
    }
    if (sem_unlink(MY_SEM_READ) == -1)
    {
        onError("Error while sem_unlink");
    }
    if (sem_unlink(MY_SEM_WRITE) == -1)
    {
        onError("Error while sem_unlink");
    }
}



/**
 * @brief Method used for signal handling
 * 
 * @param signal What signal happened
 */
void handle_signal(int signal){
    buffer->terminate= true;
    quit = 1;
}

/**
 * @brief Prints the new results 
 * 
 * @param toPrint Graph to print
 */
void printNewResult(graph_t toPrint){
    fprintf(stdout, "Solution with %d edges: ",toPrint.edges_len);
    for(int i=0; i<toPrint.edges_len; i++){
        fprintf(stdout , "%d-%d", toPrint.edges[i].from.id, toPrint.edges[i].to.id);
        if(toPrint.edges_len - i > 1){
            fprintf(stdout,", ");
        }
    }
    fprintf(stdout, "\n");
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    PROGRAM_NAME = argv[0];
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    //-------Shared memory / buffer opening
    shmfd = shm_open(SHM_CIRC_BUFFER, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1)
    {
        onError("Error while shm_open");
    }
    if (ftruncate(shmfd, sizeof(circular_buffer_t)) < 0)
    {
        onError("Error while ftruncate");
    }

    buffer = mmap(NULL, sizeof(circular_buffer_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buffer == MAP_FAILED)
    {
        onError("Error while mmap");
    }
    //---------- semaphore openig things
    readSem = sem_open(MY_SEM_READ, O_CREAT | O_EXCL, 0600, 0);
    if (readSem == SEM_FAILED)
    {
        onError("Error while sem_open");
    }
    writeSem = sem_open(MY_SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);
    if (writeSem == SEM_FAILED)
    {
        onError("Error while sem_open");
    }


    graph_t min_result;
    min_result.edges_len = MAX_EDGES;
    int rd_pos=0;
    graph_t tmp;
    while(!quit){
        sem_wait(readSem);
        tmp = buffer->graphs[rd_pos];
        sem_post(writeSem);
        rd_pos++;
        rd_pos %= MAX_ELEMENTS_IN_BUFFER;
        if(tmp.edges_len == 0){
            fprintf(stdout,"Graph is acylic. Done.");
            fflush(stdout);
            buffer->terminate = true;
            closeRessourcesNormal();
            exit(EXIT_SUCCESS);
        }
        if(tmp.edges_len< min_result.edges_len){
            min_result = tmp;
            printNewResult(min_result);
        }
    }

   closeRessourcesNormal();
}