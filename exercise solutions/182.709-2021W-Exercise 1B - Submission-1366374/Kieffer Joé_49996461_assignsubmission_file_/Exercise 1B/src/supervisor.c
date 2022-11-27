/**
 * @file supervisor.c
 * @author Kieffer Joé <e11814254@student.tuwien.ac.at>
 * @date 08.11.2021
 * 
 * @brief One of the main program modules
 * 
 * This program implements the supervisor which prints out which edges to remove from a graph to make it 3 colorable,
 * if the graph is not already 3 colorable. Moreover, this program opens the shared memory and the semaphores for generators
 * to use.
 */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "shared.h"

const char *progName;
struct myshm *myshm;

sem_t *sem_Free;
sem_t *sem_Used;
sem_t *sem_Mutex;

/**
 * @brief Method to handle signals
 *
 * @details If a signal is send, this function sets the quit flag in the shared memory, so that the supervisor and all generators
 * could end to use it and begin to cleanup the ressources and finally end.
 */
void handle_signal(){
    myshm->quit = 1;
}

/**
 * @brief This method reads the shared memory and prints it out
 *
 * @details Every time this method reads a better solution than already found then it gets printed out. Is a solution with 0 edges
 * found than the function sets the quit flag in the shared memory and ends. If no solution with 0 edges exist the supervisor remains
 * in this function as long as no signal is sent.
 */
void readSharedMemory(void){
    int bestSolution = INT_MAX;
    int readPos = 0;
    while(!(myshm->quit)) { 
        if(sem_wait(sem_Used) == -1){
            if(errno == EINTR){
                continue;
            }
        }
        if(myshm->ndata[readPos] == 0){
            fprintf(stdout, "[%s] The graph is 3-colorable!\n", progName);
            bestSolution = 0;
            myshm->quit = 1;
        }
        if(myshm->ndata[readPos] < bestSolution){
            fprintf(stdout, "[%s] Solution with %i edges: ", progName, myshm->ndata[readPos]);
            for(int i=0; i < myshm->ndata[readPos]; i++){
                fprintf(stdout, "%i-%i ", myshm->data[readPos][i][0], myshm->data[readPos][i][1]);
            }
            fprintf(stdout, "\n");
            bestSolution = myshm->ndata[readPos];
        }
        sem_post(sem_Free);
        readPos++;
        readPos %= MAX_DATA;
    }
    sem_post(sem_Free);
}

/**
 * @brief This method closes the filedescriptor
 * 
 * @details This method assumes it is called with a valid filedescriptor and closes it. Should the close function not succeed, an error message
 * is displayed.
 * 
 * @param fd The filedescriptor to close
 * @return 0 on success, 1 if an error occured while closing.
 */
int closeFd(int fd){
    assert(fcntl(fd, F_GETFD) != -1);
    int closed = close(fd);
    if (closed == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't close file descriptor! close failed: %s\n", progName, __LINE__, strerror(errno));
        return 1;
        //exit(EXIT_FAILURE);
    }
    return 0;
}

/**
 * @brief This method closes the shared memory
 * 
 * @details This method unmaps the shared memory if already mapped and unlinks the shared memory. If the shared memory wasn't mapped this
 * function also calls the function to close the filedescriptor.
 * 
 * @param errorStep indictates which ressources need to get cleanup: 1 shm_open was successfull, 0 mmap was successfull and no error may occured
 * @param fd the filedescriptor if no mapping occured, should be -1 if no descriptor is given
 */
void closeSharedMemory(int errorStep, int fd){
    int unmapped;
    int unlinked;
    int error;
    switch (errorStep){
        case 0:
            unmapped = munmap(myshm, sizeof(*myshm));
            if(unmapped == -1){
                fprintf(stderr, "[%s:%i] ERROR: Coudn't unmap shared memory! munmap failed: %s\n", progName, __LINE__, strerror(errno));
                exit(EXIT_FAILURE);
            }
        case 1:
            unlinked = shm_unlink(SHM_NAME);
            if (unlinked == -1){
                fprintf(stderr, "[%s:%i] ERROR: Coudn't unlink shared memory! shm_unlink failed: %s\n", progName, __LINE__, strerror(errno));
                exit(EXIT_FAILURE);
            }
            if(errorStep == 1){
                error = closeFd(fd);
                if(error){
                    exit(EXIT_FAILURE);
                }
            }
            break;
        default:
            assert(0);
            break;
    }
}

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
    int error = 0;
    if(closed == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't close %s! sem_close failed: %s\n", progName, __LINE__, sem_Name, strerror(errno));
        error = 1;
    }

    closed = sem_unlink(sem_Name);
    if(closed == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't unlink %s! sem_unlink failed: %s\n", progName, __LINE__, sem_Name, strerror(errno));
        error = 1;
    }
    return error;
}

/**
 * @brief This function closes all semaphores
 * 
 * @details This function tries to close all semaphores with the function closeSemaphore. If at least one has an error this function ends the program after trying to close
 * all semaphores.
 */
void closeSemaphores(void){
    int i = closeSemaphore(sem_Free, SEM_FREE);
    i += closeSemaphore(sem_Used, SEM_USED);
    i += closeSemaphore(sem_Mutex, SEM_MUTEX);
    if(i > 0){
        exit(EXIT_FAILURE);
    }
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
        closeSharedMemory(0, -1);
        closeSemaphores();     
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief This function opens the semaphores and initialises them
 * 
 * @details This function creates and initialises all the semaphores and calls the function checkOpenSemaphore to check if the openings were successfull.
 */
void openSemaphores(void){
    sem_Free = sem_open(SEM_FREE, O_CREAT | O_EXCL | O_RDWR, 0600, MAX_DATA);
    checkOpenSemaphore(sem_Free, SEM_FREE);
    sem_Used = sem_open(SEM_USED, O_CREAT | O_EXCL | O_RDWR, 0600, 0);
    checkOpenSemaphore(sem_Used, SEM_USED);
    sem_Mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL | O_RDWR, 0600, 1);
    checkOpenSemaphore(sem_Mutex, SEM_MUTEX);
}

/**
 * @brief This function opens the shared memory
 * 
 * @details This function creates and opens a filedescriptor to a shared memory, truncates the shared memory and then maps it. Finally the filedescriptor is closed.
 * Should an error occur all created ressources gets cleanup with another function. This function also initialise the quit flag and the write position.
 */
void openSharedMemory(void){
    int fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't open shared memory! shm_open failed: %s\n", progName, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int truncated = ftruncate(fd, sizeof(struct myshm));
    if (truncated == -1){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't truncate shared memory! ftruncate failed: %s\n", progName, __LINE__, strerror(errno));
        closeSharedMemory(1, fd);
        exit(EXIT_FAILURE);
    }
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if(myshm == MAP_FAILED){
        fprintf(stderr, "[%s:%i] ERROR: Coudn't map shared memory! mmap failed: %s\n", progName, __LINE__, strerror(errno));
        closeSharedMemory(1, fd);
        exit(EXIT_FAILURE);
    }
    int error = closeFd(fd);
    if(error){
        closeSharedMemory(0, fd);
        exit(EXIT_FAILURE);
    }
    myshm->quit = 0;
    myshm->writePos=0;
}

/**
 * @brief The main function
 * 
 * @details This function checks if no argument is given, initialises the signal handling and calls the functions needed to run this program. This functions
 * open the shared memory and the semaphores, read the shared memory and closes the shared memory and the semaphores.
 * 
 * @param argc The argument counter
 * @param argv The arguments given at program start
 * @return EXIT_SUCCESS if successfull, else EXIT_FAILURE
 */
int main( int argc, char *argv[]){
    progName = argv[0];
    if(argc > 1){
        fprintf(stderr, "[%s:%i] ERROR: supervisor takes no arguments!\n", progName, __LINE__);
        exit(EXIT_FAILURE);
    }
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    openSharedMemory();
    openSemaphores();

    readSharedMemory();

    closeSharedMemory(0, -1);
    closeSemaphores();

    return EXIT_SUCCESS;
}