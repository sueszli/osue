/**
* @file circular-buffer.c
* @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>
* @date 14.11.2021
*
* @brief Manages the shared memory.
* @details Allows the generator and supervisor to access the shared memory.
*/

#include "circular-buffer.h"
int fd;



/**
 * If there was a problem, terminate the program and print the error into stderr.
 * @param arr
 */
void failedExit(char* arr) {
    fprintf(stderr, "[%s] Something went wrong managing the shared memory: %s\n",CIRCULAR_BUFFER_NAME, arr);
    exit(EXIT_FAILURE);
}

/**
 * Clean the generator
 */
void disposeRes() {
    if(semFree != NULL) {
        sem_close(semFree);
    }

    if(semUsed != NULL) {
        sem_close(semUsed);
    }

    if(semExcl != NULL) {
        sem_close(semExcl);
    }

    shm_unlink(MEM_NAME);
}
/**
 * Clean the supervisor
 */
void disposeSuper(){
    disposeRes();

    close(fd);

    if(shmp != NULL) {
        munmap(shmp, sizeof(*shmp));
    }

    if(semFree != NULL) {
        sem_unlink(SEM_FREE);
    }

    if(semUsed != NULL) {
        sem_unlink(SEM_USED);
    }

    if(semExcl != NULL) {
        sem_unlink(SEM_EXCL);
    }
}
/**
 * Sets state to 2 -> terminated
 */
void setStateClosed(){
    //End immediately if the error was not an interruption by the call handler
    while(sem_wait(semExcl)!=0){
        if (errno != EINTR) {
            disposeSuper();
            failedExit("writeBuffer,semFree");
        }
    }
    shmp->state=2;
    sem_post(semExcl);
    disposeSuper();
}
void handle_signal(int signal){
    //printf("Handler called, %d",signal);
    if (signal == SIGINT || signal == SIGTERM) {
        setStateClosed();
        exit(EXIT_SUCCESS);
    }
}

int stillRunning(){
    while(sem_wait(semExcl)!=0){
        if (errno != EINTR) {
            disposeSuper();
            failedExit("writeBuffer,semFree");
        }
    }
    int state = shmp->state;
    sem_post(semExcl);
    return state;
}

void createMemory() {
    /* Create shared memory object and set its size to the size of our structure */
    fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        failedExit("shm_open");


    if (ftruncate(fd, sizeof(shmbuf)) == -1)
        failedExit("ftruncate");

    /* Map the object into the caller's address space */
    shmp = mmap(NULL, sizeof(*shmp),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
        failedExit("mmap");

    shmp -> state = 0;
    semFree = sem_open(SEM_FREE, O_CREAT, S_IRUSR | S_IWUSR, BUF_SIZE);
    semUsed = sem_open(SEM_USED, O_CREAT, S_IRUSR | S_IWUSR, 0);
    semExcl = sem_open(SEM_EXCL, O_CREAT, S_IRUSR | S_IWUSR, 1);
    if (semFree == SEM_FAILED || semUsed == SEM_FAILED || semExcl == SEM_FAILED) {
        disposeSuper();
        failedExit("createMemory");
    }
}

void openMemory() {
    if(shmp != NULL) {
        failedExit("Supervisor not started.");
    }

    /* Create shared memory object and set its size to the size of our structure */
    fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1)
        failedExit("shm_open");

    /* Map the object into the caller's address space */
    shmp = mmap(NULL, sizeof(*shmp),PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
        failedExit("mmap");

    semFree = sem_open(SEM_FREE,  BUF_SIZE);
    semUsed = sem_open(SEM_USED, 0);
    semExcl = sem_open(SEM_EXCL, 1);
    if (semFree == SEM_FAILED || semUsed == SEM_FAILED || semExcl == SEM_FAILED) {
        disposeRes();
        failedExit("openMemory");
    }
}

void writeBuffer(int *firstNodes, int *secondNodes,int listCount){
    //if sem_wait is 0 then there was an error
    while(sem_wait(semFree)!=0) {
        if (errno != EINTR) {
            //End immediately if the error was not an interruption by the call handler
            disposeRes();
            failedExit("writeBuffer,semFree");
        }
    }

    while(sem_wait(semExcl)!=0){
        if (errno != EINTR) {
            disposeRes();
            failedExit("writeBuffer,semFree");
        }
    }

    if (listCount == 0) {
        shmp->state = 2;
    }
    else
        shmp->state = 1;
    for(int i = 0; i<listCount;i++) {
        shmp -> secondNodeOfEdge[i] = secondNodes[i];
        shmp -> firstNodeOfEdge[i] = firstNodes[i];
    }

    shmp -> count = listCount;
    sem_post(semExcl);
    sem_post(semUsed);
}

shmbuf* readBuffer(){
    while(sem_wait(semUsed)!=0){
        if (errno != EINTR) {
            //End immediately if the error was not an interruption by the call handler
            setStateClosed();
            failedExit("readBuffer,semUsed");
        }
    }
    sem_post(semFree);
    return shmp;
}


