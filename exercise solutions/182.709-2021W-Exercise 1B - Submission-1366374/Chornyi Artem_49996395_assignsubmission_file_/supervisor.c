/**
 * @author Artem Chornyi. 11922295
 * @brief A supervisor that listens on shared circular buffer, to which generator writes,
 *        and saves best solutions for acyclic graph.
 * 
 * @details Reconfigures signal handling, namely signals (SIGINT, SIGTERM) 
 *          to notify generator, that its time to exit.
 * 
 *          Supervisor saves the best solution (edges to remove) for making a graph, that was 
 *          specified in generator arguments, acyclic.
 * 
 *          If a specified graph in generator is already acyclic, the solution with no edges
 *          will eventually be found. At that moment supervisor notifes generator to exit.
 * @date 9-th November 2021 (09.11.2021)
 * 
 */


#ifndef EDGE
#include "./edge.c"
#endif

#ifndef SHARED_STRUCTURES
#include "./shared_structures.c"
#endif

#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

char* filename;
/**
 * @brief Callback function, which occurs when SIGINT, SIGTERM signals occur.
 *
 * @details Maps shared_control structure from shared memory and writes recievedInterrupt, that
 * notifies both supervisor and generator, that they should free their resources and exit. 
 * 
 * @param signal Signal code (SIGINT, SIGTERM), that supervisor received. 
 */
static void handle_signals(int signal){
    int shcfd;
    if((shcfd = shm_open(SHARED_CONTROL_NAME, O_CREAT | O_RDWR, 0644)) == -1){
        fprintf(stderr, "[%s] shm_open with name %s failed: %s\n", filename, SHARED_CONTROL_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shcfd, sizeof(shared_control)) < 0){
        close(shcfd);
        fprintf(stderr, "[%s] ftruncate for shared_control failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }
    shared_control *pControl;
    if((pControl = mmap(NULL, sizeof(shared_control), PROT_WRITE | PROT_READ, MAP_SHARED, shcfd, 0)) == MAP_FAILED){
        close(shcfd);
        fprintf(stderr, "[%s] mmap shared_control structure failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    pControl->recievedInterrupt = 1;

    munmap(pControl, sizeof(shared_control));
    close(shcfd);
}

/**
 * @brief Sets up SIGINT and SIGTERM signals to be handled by handle_signals() function.
 *
 */
static void setup_signals(void){
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signals; 
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}


/**
 * @brief Reads next circular buffer entry and evaluates if the solution there is better 
 *        than supervisor has. 
 *
 * @details For synchronization purposes waits 'read semaphore', which indicates how many 
 *          entries are in circular buffer, that need to be read. 
 *          
 *          Posts 'write semaphore' to let generator know, a place was read and is now free
 *          to be written.
 *          
 *          Function reads from circular buffer only at indexes from 0 to (CIR_BUFFER_LENGTH - 1)
 *          i.e.(eadPos = (readPos + 1) % CIR_BUFFER_LENGTH;)
 * 
 *          If read entry is a better solution (has less edges), then it will be written to
 *          pResultEntry.
 * 
 * @param semaphores struct with
 *                   writeSem: pointer to semaphore object,
 *                      which indicates how many entries are free to be written
 *                   readSem pointer to semaphore object, 
 *                      which indicates how many entries are to be read.
 * @param pBuffer pointer to mapped shared cir_buffer structure, which holds all recent entries
 *                written by generator.
 * @param pResultEntry pointer to cir_buffer_entry, which holds best solution to acyclic graph.
 * @param pControl pointer to shared_control object, which holds flag, that denotes
 *                 if it is time to terminate.
 * 
 */
static void read_entry(const semaphores semaphores, const cir_buffer *const pBuffer, cir_buffer_entry *pResultEntry, shared_control *pControl){
    sem_wait(semaphores.readSem);
    static int readPos = 0;
    cir_buffer_entry localResultEntry = pBuffer->entries[readPos];
    if(pControl->recievedInterrupt == 1){
        printf("\nGot interrupt. \nNot reading further entries...\n");
        return;
    }
    int resultEntryLength = 0;
    
    // Counting edges in a read entry.
    for (int i = 0; i < MAX_EDGES_PER_ENTRY && (localResultEntry.edges[i].from != 0 || localResultEntry.edges[i].to != 0); i++)
    {
        resultEntryLength += 1;
    }

    // If a better solution was found.
    if(pResultEntry->edgesAmount > resultEntryLength){
        printf("[%d Edges] ", resultEntryLength);
        for (int i = 0; i < resultEntryLength; i++)
        {
            pResultEntry->edges[i] = localResultEntry.edges[i];
            printf("%d-%d ", localResultEntry.edges[i].from, localResultEntry.edges[i].to);
        }
        printf("\n");
        pResultEntry->edgesAmount = resultEntryLength;
    }
    readPos = (readPos + 1) % CIR_BUFFER_LENGTH;
    sem_post(semaphores.writeSem);
}

int main(int argc, char **argv){
    // If any argument was specified, we should exit.
    filename = argv[0];
    if(argc > 1) {
        fprintf(stderr, "[%s] takes no arguments!\n", filename);
        exit(EXIT_FAILURE);
    }

    // Configure SIGINT,SIGTERM.
    setup_signals();
    
    // Descriptor pointing to shared memory of circular buffer.
    int shmfd;
    if((shmfd = shm_open(SHARED_BUFFER_NAME, O_CREAT | O_RDWR, 0644)) == -1){
        fprintf(stderr, "[%s] shm_open with name %s failed: %s\n", filename, SHARED_BUFFER_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if(ftruncate(shmfd, sizeof(cir_buffer)) == -1){
        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);

        fprintf(stderr, "[%s] ftruncate failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }


    // Descriptor pointing to shared memory of control structure.
    int shcfd;
    if((shcfd = shm_open(SHARED_CONTROL_NAME, O_CREAT | O_RDWR, 0644)) == -1){
        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);
        
        fprintf(stderr, "[%s] shm_open with name %s failed: %s\n", filename, SHARED_CONTROL_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if(ftruncate(shcfd, sizeof(shared_control)) < 0){
        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);
        
        close(shcfd);
        shm_unlink(SHARED_CONTROL_NAME);
        fprintf(stderr, "[%s] ftruncate for shared_control failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    shared_control *pControl;
    if((pControl = mmap(NULL, sizeof(shared_control), PROT_WRITE | PROT_READ, MAP_SHARED, shcfd, 0)) == MAP_FAILED){
        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);
        
        close(shcfd);
        shm_unlink(SHARED_CONTROL_NAME);
        
        fprintf(stderr, "[%s] mmap for shared_control failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    cir_buffer *pBuffer;
    if((pBuffer = mmap(NULL, sizeof(cir_buffer), PROT_READ, MAP_SHARED, shmfd, 0)) == MAP_FAILED){
        munmap(pControl, sizeof(*pControl));
        
        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);
        
        close(shcfd);
        shm_unlink(SHARED_CONTROL_NAME);

        fprintf(stderr, "[%s] mmap for cir_buffer failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    sem_t *readSem;
    if((readSem = sem_open(READ_SEMAPHORE_NAME, O_EXCL | O_CREAT, 0644, 0)) == SEM_FAILED){
        munmap(pControl, sizeof(*pControl));
        munmap(pBuffer, sizeof(*pBuffer));

        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);
        
        close(shcfd);
        shm_unlink(SHARED_CONTROL_NAME);

        fprintf(stderr, "[%s] sem_open with name %s failed: %s\n", filename, READ_SEMAPHORE_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    sem_t *writeSem;
    if((writeSem = sem_open(WRITE_SEMAPHORE_NAME, O_EXCL | O_CREAT, 0644, CIR_BUFFER_LENGTH)) == SEM_FAILED){
        munmap(pControl, sizeof(*pControl));
        munmap(pBuffer, sizeof(*pBuffer));

        close(shmfd);
        shm_unlink(SHARED_BUFFER_NAME);
        
        close(shcfd);
        shm_unlink(SHARED_CONTROL_NAME);
        
        sem_close(readSem);
        sem_unlink(READ_SEMAPHORE_NAME);

        fprintf(stderr, "[%s] sem_open with name %s failed: %s\n", filename, READ_SEMAPHORE_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }


    printf("Started reading...\n");
    
    semaphores semaphores = {
        .readSem=readSem,
        .writeSem=writeSem
    };

    // Initializes a solition entry.
    cir_buffer_entry resultEntry = {.edgesAmount=MAX_EDGES_PER_ENTRY+1};
    //Iterate as long as no signals were caught.
    while(1){
        read_entry(semaphores, pBuffer, &resultEntry, pControl);
        if(pControl->recievedInterrupt == 1){
            printf("Stoping loop...\n");
            break;
        }
        if(resultEntry.edgesAmount == 0){
            pControl->recievedInterrupt = 1;
            printf("The graph is acyclic!\n");
            break;
        }
    }

    // MUNMAP
    if(munmap(pControl, sizeof(pControl)) == -1){
        fprintf(stderr, "[%s] munmap for shared_control failed: %s\n", filename, strerror(errno));
    }
    if(munmap(pBuffer, sizeof(pBuffer)) == -1){
        fprintf(stderr, "[%s] munmap for cir_buffer failed: %s\n", filename, strerror(errno));
    }

    // SEM_CLOSE
    if(sem_close(writeSem) == -1){
        fprintf(stderr, "[%s] sem_close with name %s failed: %s\n", filename, WRITE_SEMAPHORE_NAME, strerror(errno));
    }
    if(sem_close(readSem) == -1){
        fprintf(stderr, "[%s] sem_close with name %s failed: %s\n", filename, READ_SEMAPHORE_NAME, strerror(errno));
    }

    // CLOSE
    if(close(shmfd) != 0){
        fprintf(stderr, "[%s] close for fd of cir_buffer failed: %s\n", filename, strerror(errno));
    }
    if(close(shcfd) != 0){
        fprintf(stderr, "[%s] close for fd of shared_control failed: %s\n", filename, strerror(errno));
    }

    //SHM_UNLINK
    if(shm_unlink(SHARED_BUFFER_NAME) == -1){
        fprintf(stderr, "[%s] shm_unlink with name %s failed: %s\n", filename, SHARED_BUFFER_NAME, strerror(errno));
    }
    if(shm_unlink(SHARED_CONTROL_NAME) == -1){
        fprintf(stderr, "[%s] shm_unlink with name %s failed: %s\n", filename, SHARED_CONTROL_NAME, strerror(errno));
    }

    //SEM_UNLINK
    if(sem_unlink(MUTEX_NAME) == -1){
        fprintf(stderr, "[%s] sem_unlink with name %s failed: %s\n", filename, MUTEX_NAME, strerror(errno));
    }
    if(sem_unlink(WRITE_SEMAPHORE_NAME) == -1){
        fprintf(stderr, "[%s] sem_unlink with name %s failed: %s\n", filename, WRITE_SEMAPHORE_NAME, strerror(errno));
    }
    if(sem_unlink(READ_SEMAPHORE_NAME) == -1){
        fprintf(stderr, "[%s] sem_unlink with name %s failed: %s\n", filename, READ_SEMAPHORE_NAME, strerror(errno));
    }


    exit(EXIT_SUCCESS);//return 0;
}