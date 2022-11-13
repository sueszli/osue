/**
 * module name: circular_buffer.c
 * @author      Huber Samuel 11905181
 * @brief       enables communication between generator.c and supervisor.c
 * @details     circular buffer containing generated lists of graphs
 *              writen by generator.c and read from supervisor.c
 * @date        06.11.2021
**/

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include "circular_buffer.h"


#define SHM_NAME "/11905181_shm"     // shared memory name
// name of semaphores
#define SEM_FREE "/11905181_free"
#define SEM_USED "/11905181_used"
#define SEM_MUTEX "/11905181_mutex"

#define STORAGE_SIZE 42        // fixed size of the buffer

/**
 * structure representing a circular buffer
 * @param reader: position in data list of which to read from next
 * @param writer: position in data list of which to write to next
 * @param data: list of calculated solutions (2.856KiB)
 * @param isTerminating: flag if programm is terminating
 */
typedef struct {
    int reader;
    int writer;
    edgeData_t data[STORAGE_SIZE];
    int isTerminating;
} circularBuffer_t;

char *prog;         // name of the called program

int shmfd;          // file descriptor of the created shared memory object

sem_t *s_free;     // semaphore for tracking the free space in the buffer
sem_t *s_used;     // semaphore for tracking the used space in the buffer
sem_t *s_mutex;    // semaphore for handling the mutual exclusive access to the buffer write end

circularBuffer_t *buffer;   // the circular buffer object

/**
 * @brief informs user about occurred error
 * @details prints message of occurred error\n
 *          global variables used: 'prog'
 * @param msg: msg regarding occurred error
 * @param error: actual error that was thrown
 * @param e: flag if program should exit after throwing error
 */
static void errorHandling(char *msg, char *error, int e);

void initiateBuffer(void){

    // create and/or open the shared memory object:
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        removeBuffer();
        errorHandling("shm_open failed, while opening shared memory", strerror(errno),1);
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(circularBuffer_t)) < 0){
        removeBuffer();
        errorHandling("ftruncate failed, while setting shared memory size", strerror(errno),1);
    }

    // map shared memory object:
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buffer == MAP_FAILED){
        removeBuffer();
        errorHandling("mmap failed, while mapping shared memory object", strerror(errno),1);
    }


    // reset reader & writer
    buffer->reader = 0;
    buffer->writer = 0;

    // open new semaphores
    s_free = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, STORAGE_SIZE);
    if (s_free == SEM_FAILED) {
        removeBuffer();
        errorHandling("opening semaphore 's_free' failed", strerror(errno),1);
    }
    s_used = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
    if (s_used == SEM_FAILED) {
        removeBuffer();
        errorHandling("opening semaphore 's_used' failed", strerror(errno),1);
    }
    s_mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1);
    if (s_mutex == SEM_FAILED) {
        removeBuffer();
        errorHandling("opening semaphore 's_mutex' failed", strerror(errno),1);
    }

    setTermination(0);
}

void removeBuffer(void){

    closeBuffer();

    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1){
        errorHandling("unlinking shared memory failed", strerror(errno),0);
    }

    // remove semaphores
    if (sem_unlink(SEM_FREE) == -1){
        errorHandling("unlinking semaphore 'SEM_FREE' failed", strerror(errno),0);
    }
    if (sem_unlink(SEM_USED) == -1){
        errorHandling("unlinking semaphore 'SEM_USED' failed", strerror(errno),0);
    }
    if (sem_unlink(SEM_MUTEX) == -1){
        errorHandling("unlinking semaphore 'SEM_MUTEX' failed", strerror(errno),0);
    }
}

void openBuffer(void) {

    // create and/or open the shared memory object:
    shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        closeBuffer();
        errorHandling("shm_open failed, while opening shared memory", strerror(errno),1);
    }

    // map shared memory object:
    buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (buffer == MAP_FAILED){
        closeBuffer();
        errorHandling("mmap failed, while mapping shared memory object", strerror(errno),1);
    }

    // open new semaphores
    s_free = sem_open(SEM_FREE,0);
    if (s_free == SEM_FAILED) {
        closeBuffer();
        errorHandling("opening semaphore 's_free' failed", strerror(errno),1);
    }
    s_used = sem_open(SEM_USED, 0);
    if (s_used == SEM_FAILED) {
        closeBuffer();
        errorHandling("opening semaphore 's_used' failed", strerror(errno),1);
    }
    s_mutex = sem_open(SEM_MUTEX,0);
    if (s_mutex == SEM_FAILED) {
        closeBuffer();
        errorHandling("opening semaphore 's_mutex' failed", strerror(errno),1);
    }
}

void closeBuffer(void) {
    // close shared memory object
    if (close(shmfd) == -1){
        errorHandling("closing shared memory object failed", strerror(errno),0);
    }

    // unmap shared memory:
    if (munmap(buffer, sizeof(*buffer)) == -1){
        errorHandling("unmapping shared memory failed", strerror(errno),0);
    }

    // close semaphores
    if (sem_close(s_free) == -1){
        errorHandling("closing semaphore 's_free' failed", strerror(errno),0);
    }
    if (sem_close(s_used) == -1){
        errorHandling("closing semaphore 's_used' failed", strerror(errno),0);
    }
    if (sem_close(s_mutex) == -1){
        errorHandling("closing semaphore 's_mutex' failed", strerror(errno),0);
    }
}

void readBuffer(edgeData_t *readData) {
    if (sem_wait(s_used) == -1){
        // signal interrupt
        if (errno == EINTR) {
            return;
        } else {
            removeBuffer();
            errorHandling("wait semaphore 's_used' returned an error", strerror(errno),1);
        }
    }

    memcpy(readData,&(buffer->data[buffer->reader]), sizeof(edgeData_t));

    buffer->reader += 1;
    buffer->reader %= STORAGE_SIZE;

    if (sem_post(s_free) == -1){
        removeBuffer();
        errorHandling("post semaphore 's_free' returned an error", strerror(errno),1);
    }
}

void writeBuffer(edge_t *edges, int size) {

    if (sem_wait(s_free) == -1){
        // signal interrupt
        if (errno == EINTR) {
            return;
        } else {
            closeBuffer();
            errorHandling("wait semaphore 's_free' returned an error", strerror(errno), 1);
        }
    }

    // guarantees mutually exclusive access to the write-end of the circular buffer
    if (sem_wait(s_mutex) == -1){
        // signal interrupt
        if (errno == EINTR) {
            return;
        } else {
            closeBuffer();
            errorHandling("wait semaphore 's_mutex' returned an error", strerror(errno),1);
        }
    }

    edgeData_t current;
    for (int i = 0; i < size; ++i) {
        current.list[i].from = edges[i].from;
        current.list[i].to = edges[i].to;
    }
    current.size = size;
    memcpy(&(buffer->data[buffer->writer]),&current,sizeof(edgeData_t));

    buffer->writer += 1;
    buffer->writer %= STORAGE_SIZE;

    if (sem_post(s_mutex) == -1){
        closeBuffer();
        errorHandling("post semaphore 's_mutex' returned an error", strerror(errno),1);
    }
    if (sem_post(s_used) == -1){
        closeBuffer();
        errorHandling("post semaphore 's_used' returned an error", strerror(errno),1);
    }
}

void setTermination(int flag) {
    while (sem_wait(s_mutex) == -1) {
        // signal interrupt
        if (errno == EINTR) {
            continue;
        } else {
            removeBuffer();
            errorHandling("wait semaphore 's_mutex' returned an error while setting termination", strerror(errno), 1);
        }
    }

    buffer->isTerminating = flag;

    if (sem_post(s_mutex) == -1) {
        removeBuffer();
        errorHandling("wait semaphore 's_mutex' returned an error while setting termination", strerror(errno), 1);
    }
}

int isTerminating(void){
    while (sem_wait(s_mutex) == -1) {
        // signal interrupt
        if (errno == EINTR) {
            continue;
        } else {
            closeBuffer();
            errorHandling("wait semaphore 's_mutex' returned an error while getting termination", strerror(errno), 1);
        }
    }

     int flag = buffer->isTerminating;

    if (sem_post(s_mutex) == -1) {
        closeBuffer();
        errorHandling("post semaphore 's_mutex' returned an error while getting termination", strerror(errno), 1);
    }

    return flag;

}

static void errorHandling(char *msg, char *error, int e){

    fprintf(stderr, "[%s] ERROR: %s:\n %s\n",prog, msg, error);
    if(e == 1){
        exit(EXIT_FAILURE);
    }
}