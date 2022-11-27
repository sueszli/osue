/**
 * @file circularbuffer.c
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief see header
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "circularbuffer.h"

static sem_t *free_sem; /*! semaphore denoting amount of free space in the circular buffer */
static sem_t *used_sem; /*! semaphore denoting amount of used space in the circular buffer */
static sem_t *write_sem; /*! semaphore to disable simultaineous writing */
struct circularBuffer* circularBuffer; /*! circular buffer file descriptor in shared memory */

/**
 * @brief Increments the index modulo CIRC_BUFFER_SIZE
 * 
 * @param index 
 */
static void increment_index(int* index);

/**
 * helper function to handle termination after errors occurred
 *
 * @brief prints error message containing msgStart and exits program with EXIT_FAILURE
 * @param msgStart
 */
static void handle_error(char* msgStart);

int init_circ_buf(void){
    int shmfd = shm_open(CB_NAME, O_CREAT | O_RDWR, 0600);
    if(shmfd == -1){
        fprintf(stderr, "Failed to open shared memory: %s\n", strerror(errno));
        return -1;}
    if(ftruncate(shmfd, sizeof(struct circularBuffer))<0){
        fprintf(stderr, "Failed to truncate shared memory: %s\n", strerror(errno));
        return -1;}
    
    circularBuffer = mmap(NULL, sizeof(struct circularBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circularBuffer == MAP_FAILED){
        fprintf(stderr, "Failed to map circular buffer: %s\n", strerror(errno));
        return -1;}
    memset(circularBuffer, 0, sizeof(struct circularBuffer));
    circularBuffer->readIndex=0;
    circularBuffer->writeIndex=0;

    if(close(shmfd) == -1){
        fprintf(stderr, "Failed to close shared memory: %s\n", strerror(errno));
        return -1;}

    free_sem = sem_open(FREE_SEM, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, CIRC_BUFFER_SIZE);
    used_sem = sem_open(USED_SEM, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, 0);
    write_sem = sem_open(WRITE_SEM, O_CREAT | O_EXCL, S_IWUSR | S_IRUSR, 1);
    return 0;
}

int cleanup_circ_buf(void){
    circularBuffer->stop=true;
    if(munmap(circularBuffer, sizeof(*circularBuffer)) == -1){
        fprintf(stderr, "Failed to unmap circular buffer: %s\n", strerror(errno));
        return -1;}
    if(shm_unlink(CB_NAME)==-1){
        fprintf(stderr, "Failed to unlink shared memory: %s\n", strerror(errno));
        return -1;}
    if(sem_close(free_sem)==-1){
        fprintf(stderr, "Failed to close semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_close(used_sem)==-1){
        fprintf(stderr, "Failed to close semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_close(write_sem)==-1){
        fprintf(stderr, "Failed to close semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_unlink(FREE_SEM)==-1){
        fprintf(stderr, "Failed to unlink semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_unlink(USED_SEM)==-1){
        fprintf(stderr, "Failed to unlink semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_unlink(WRITE_SEM)==-1){
        fprintf(stderr, "Failed to unlink semaphore: %s\n", strerror(errno));
        return -1;}
    return 0;
}

graph* readBuf(void){
    if(sem_wait(used_sem) == -1){
        return NULL;
    }
    graph* g = malloc(sizeof(graph));
    if(g==NULL){
        handle_error("failed to allocate memory");
    }
    memcpy(g->edges, circularBuffer->graphs[circularBuffer->readIndex], MAX_EDGES*sizeof(edge));
    g->numEdges = circularBuffer->sizes[circularBuffer->readIndex];
    increment_index(&(circularBuffer->readIndex));
    sem_post(free_sem);
    return g;
}

int open_circ_buf(void){
    int shmfd = shm_open(CB_NAME, O_RDWR, S_IRUSR | S_IWUSR);
    if(shmfd == -1){
        fprintf(stderr, "Failed to open shared memory: %s\n", strerror(errno));
        return -1;}
    circularBuffer = mmap(NULL, sizeof(*circularBuffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(circularBuffer == MAP_FAILED){
        fprintf(stderr, "Failed to map: %s\n", strerror(errno));
        return -1;}
    if(close(shmfd) == -1){
        fprintf(stderr, "Failed to close shared memory: %s\n", strerror(errno));
        return -1;}
    free_sem = sem_open(FREE_SEM, 0);
    used_sem = sem_open(USED_SEM, 0);
    write_sem = sem_open(WRITE_SEM, 0);
    return 0;
}

int close_circ_buf(void){
    if(munmap(circularBuffer, sizeof(*circularBuffer)) == -1){
        fprintf(stderr, "Failed to unmap circular buffer: %s\n", strerror(errno));
        return -1;}
    if(sem_close(free_sem)==-1){
        fprintf(stderr, "Failed to close semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_close(used_sem)==-1){
        fprintf(stderr, "Failed to close semaphore: %s\n", strerror(errno));
        return -1;}
    if(sem_close(write_sem)==-1){
        fprintf(stderr, "Failed to close semaphore: %s\n", strerror(errno));
        return -1;}
    return 0;
}

int writeToBuf(graph* g){
    if(sem_wait(free_sem) == -1)
        return -1;
    if(circularBuffer->stop==true){
        sem_post(free_sem);
        return -2;
    }
    if(sem_wait(write_sem) == -1)
        return -1;
    memcpy(circularBuffer->graphs[circularBuffer->writeIndex], g->edges, MAX_EDGES*sizeof(edge));
    circularBuffer->sizes[circularBuffer->writeIndex] = g->numEdges;
    increment_index(&(circularBuffer->writeIndex));
    sem_post(write_sem);
    sem_post(used_sem);
    return 0;
}

static void handle_error(char* msgStart){
    exit(EXIT_FAILURE);
}

static void increment_index(int* index){
    (*index)++;
    if(*index>=CIRC_BUFFER_SIZE){
        *index = 0;
    }
}
