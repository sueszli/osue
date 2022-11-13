/**
 * @file supervisor.c
 * @author Marcel Boros e11937377@student.tuwien.ac.at
 * @date 12.11.2021
 *
 * @brief Main generator module.
 * 
 * This program is generating feedback-arc-sets for a graph specified as input using a shared memory.
 **/
 
#include "supervisor.h"

/**
 * @brief The supervisor reads solutions for feedback-arc-sets from the shared memory and prints new solution to stdout, 
 * if the new one is better than the best solution and resets it. This function also works in an infinite loop waiting
 * for interrupting signals. If a solution with zero edges (acyclic graph) has been found or a signal occurs, the supervisor
 * terminates and commands the generator process to terminate aswell. Two semaphores are used to synchronize acces to the
 * circular buffer and the shared memory. 
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */

struct myshm* myshm;    //for signal exit

int main(int argc, char**argv) {
    
    //register signal handler
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
        
    
    int fd = createSharedMemory();
    
    sem_t* free_sem;
    sem_t* used_sem;
    sem_t* mutex_sem;
    
    myshm = mapSharedMemory(fd);
    
    //create/open semaphores
    if((free_sem = sem_open(S1, O_CREAT | O_EXCL, 0600, ENTRIES)) == SEM_FAILED) {
        fprintf(stderr, "Error at supervisor.c : sem_open : %s\n", strerror(errno));
    }
    
    if((used_sem = sem_open(S2, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED) {
        fprintf(stderr, "Error at supervisor.c : sem_open : %s\n", strerror(errno));
    }
    
    if((mutex_sem = sem_open(S3, O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED) {
        fprintf(stderr, "Error at supervisor.c : sem_open : %s\n", strerror(errno));
    }
    
    
    int min_size = -1;
    int terminate = 0;
    while(terminate == 0) {
        
        //read from shared memory
        readSHM(myshm, free_sem, mutex_sem, used_sem);
        
        //print, if a better solution was found
        if(((min_size == -1) && (myshm->bestSolution_length != -1)) || (myshm->bestSolution_length < min_size)) {
            min_size = myshm->bestSolution_length;
            printf("[./supervisor] Solution with %d edges: ", min_size);
             for(int i=0; i<MAX_LENGTH; ++i)  {
                printf("%c", myshm->bestSolution[i]);
            }
            printf("\n");
        }
        
        //terminate if graph is acyclic
        if(myshm->bestSolution_length == 0) {
            //tell generators to stop solving
            myshm->terminate = 1;
            terminate = 1;
        }
        
        
    }
    
    closeSharedMemory(myshm, fd, free_sem, mutex_sem, used_sem);
    
    return EXIT_SUCCESS;
}


//handles SIGINT and SIGTERM
void signal_handler(int signal) {
    myshm->terminate = 1;
    
     //unlink shared memory object
    if(shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "Error at supervisor.c : shm_unlink : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    //unlink semaphores
    if((sem_unlink(S1) == -1) || (sem_unlink(S2)) == -1 || (sem_unlink(S3)) == -1) {
        fprintf(stderr, "Error at supervisor.c : sem_unlink : %s\n", strerror(errno));
    }
    
    exit(EXIT_SUCCESS);
}



int createSharedMemory() {
    //unlink potential previously created shared memory object
    shm_unlink(SHM_NAME);
    
    //openSharedMemory
    int fd;
    if((fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600)) == -1) {
        fprintf(stderr, "Error at supervisor.c : shm_open : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}


//set values of the given struct in the shared memory
void initialize(struct myshm* myshm) {
    myshm->terminate = 0;
    myshm->bestSolution_length = -1;
    myshm->readEnd = 0;
    myshm->writeEnd = 0;
    memset(myshm->bestSolution, 0, MAX_LENGTH);
}


struct myshm* mapSharedMemory(int fd) {
    
    struct myshm* myshm;
    
    
    //set the size of the shared memory
    if(ftruncate(fd, CIRCULAR_BUFFER_SIZE) == -1) {
        fprintf(stderr, "Error at supervisor.c : ftruncate : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    //map to virtual memory of the process
    if((myshm = mmap(NULL, CIRCULAR_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error at supervisor.c : mmap : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    initialize(myshm);
    
    return myshm;
}


void closeSharedMemory(struct myshm* myshm, int fd, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem) {    
    
       
    //close semaphores
    if((sem_close(free_sem) == -1) || (sem_close(used_sem) == -1) 
    || (sem_close(mutex_sem) == -1)) {
            
        fprintf(stderr, "Error at supervisor.c : sem_close : %s\n", strerror(errno));
    }
   
    
    //unlink semaphores
    if((sem_unlink(S1) == -1) || (sem_unlink(S2)) == -1 || (sem_unlink(S3)) == -1) {
        fprintf(stderr, "Error at supervisor.c : sem_unlink : %s\n", strerror(errno));
    }
    
    
    //release mapping
    if(munmap(myshm, CIRCULAR_BUFFER_SIZE) == -1) {
        fprintf(stderr, "Error at supervisor.c : munmap : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    //close shared memory object
    if(close(fd) == -1) {
        fprintf(stderr, "Error at supervisor.c : close : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    
    //unlink shared memory object
    if(shm_unlink(SHM_NAME) == -1) {
        fprintf(stderr, "Error at supervisor.c : shm_unlink : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
}


//count edges in solution for graph
int edgesInSolution(char* sol) {
    int edges = 0;
    
    for(int i=0; i<MAX_LENGTH; ++i) {
        if(sol[i] == ' ') {
            ++edges;
        }
    }
    return edges;
}


//read from shared memory
void readSHM(struct myshm* myshm, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem) {
    
    //decrement semaphore -> wait for data to read
    if(sem_wait(used_sem) == -1) {
        fprintf(stderr, "Error at generator.c : sem_wait : %s\n", strerror(errno));
    }
        
    int size = edgesInSolution(myshm->data[myshm->readEnd]);
    
    
    //set newly found solution
    if((size < myshm->bestSolution_length) || (myshm->bestSolution_length == -1)) {
        
        myshm->bestSolution_length = size;
        memset(myshm->bestSolution, 0, MAX_LENGTH);
        
        for(int i=0; i<MAX_LENGTH; ++i) {
            myshm->bestSolution[i] = myshm->data[myshm->readEnd][i];
        }
        
    }
    
    myshm->readEnd = ((myshm->readEnd) + 1);
    myshm->readEnd = ((myshm->readEnd) % ((int)ENTRIES));
    
    
    //increment semaphore
    if(sem_post(free_sem) == -1) {
        fprintf(stderr, "Error at supervisor.c : sem_post : %s\n", strerror(errno));
    }
}