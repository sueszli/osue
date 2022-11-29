#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

#include "common.h"
#include "server.h"

/******************************************************************************
 * Global variables
 *****************************************************************************/

const char *pgm_name = "<not_yet_set>";

static int shmfd = -1;
static shm_data_t *shmp = MAP_FAILED;

static sem_t *sem_server = SEM_FAILED;
static sem_t *sem_ready = SEM_FAILED;
static sem_t *sem_client = SEM_FAILED;

static volatile sig_atomic_t quit = 0;

/******************************************************************************
 * Function declarations
 *****************************************************************************/

/** Create the shared memory object and the semaphores. (Task 1) */
void task_1a(void);
void task_1b(void);

/** Repeatedly process requests from clients in a synchronized way. (Task 2) */
void task_2(void);

/** Process a single request in the shared memory. (Task 3) */
void task_3(shm_data_t *);

/** Free the allocated resources. (already implemented completely) */
void free_resources(void);

/*****************************************************************************/

/** Signal handler that just sets the global variable 'quit' */
static void signal_handler(int sig)
{
    quit = 1;
}

int main(int argc, char *argv[])
{
    pgm_name = argv[0];

    // Register signal handlers
    struct sigaction s;
    s.sa_handler = signal_handler;
    s.sa_flags = 0;             // no SA_RESTART!
    if (sigfillset(&s.sa_mask) < 0) {
        error_exit("sigfillset");
    }
    if (sigaction(SIGINT, &s, NULL) < 0) {
        error_exit("sigaction SIGINT");
    }
    if (sigaction(SIGTERM, &s, NULL) < 0) {
        error_exit("sigaction SIGTERM");
    }
    // register function to free resources at normal process termination
    if (atexit(free_resources) == -1) {
        error_exit("atexit failed");
    }
    // allocate resources
    task_1a();
    task_1b();

    // service loop (calls task_3)
    task_2();

    (void) fprintf(stderr, "server exiting regularly\n");
    return EXIT_SUCCESS;
}

/***********************************************************************
 * Task 1
 * ------
 * Allocate resources for interprocess communication.
 * General remarks:
 * - Creation of the shared resources should fail if they already exist!
 * - Use PERMISSIONS (defined in common.h), which grants read and write
 *   permissions to owner, group and others.
 * - You can use the provided function 'error_exit' to exit your program
 *   in case of an error.
 * - You *MUST* use the POSIX interfaces.
 ***********************************************************************/

/***********************************************************************
 * Task 1a
 * -------
 * Create and map a named POSIX shared memory object.
 *
 * Use global variable 'shmfd' for creation.
 * Let global variable 'shmp' point to the shared memory area.
 *
 * Hints:
 * - Use SHM_NAME as name for the shared memory object (defined in common.h).
 * - Do NOT set 'shmfd' back to -1 (if you do, unlinking will fail in
 *   'free_resources')! You do not need to close the file descriptor after
 *   mapping.
 * - Don't forget to set an appropriate size for the shared memory object.
 *
 * See also: shm_overview(7), ftruncate(2), mmap(2)
 ***********************************************************************/

void task_1a(void)
{
    shmfd = shm_open(SHM_NAME,O_RDWR | O_CREAT | O_EXCL, PERMISSIONS);
    if(shmfd<0){
        error_exit("shm_open failed");
    }

    if(ftruncate(shmfd, sizeof(*shmp))<0){
        error_exit("truncate failed");
    }
    shmp = mmap(NULL, sizeof(*shmp),PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if(shmp == MAP_FAILED){
        error_exit("mmap failed");
    }
}

/***********************************************************************
 * Task 1b
 * -------
 * Allocate the POSIX semaphores.
 *
 * Let global variables 'sem_server', 'sem_ready', and 'sem_client' point
 * to the created semaphores.
 *
 * Hints:
 * - Use SEM_NAME_SERVER, SEM_NAME_CLIENT and SEM_NAME_READY as names
 *   for the semaphores (defined in common.h).
 *
 * Note: In the demo solution, sem_client is initialized with 1.
 *
 * See also: sem_overview(7)
 ***********************************************************************/

void task_1b(void)
{
    sem_server = sem_open(SEM_NAME_SERVER,O_CREAT | O_EXCL, PERMISSIONS,0);
        if (sem_server == SEM_FAILED) {
        error_exit("sem_server open failed");
    }    
    sem_ready = sem_open(SEM_NAME_READY,O_CREAT | O_EXCL, PERMISSIONS,0);
    if (sem_ready == SEM_FAILED) {
        error_exit("sem_ready open failed");
    }
    sem_client = sem_open(SEM_NAME_CLIENT,O_CREAT | O_EXCL,PERMISSIONS,1);
    if (sem_client == SEM_FAILED) {
        error_exit("sem_client open failed");
    }
}

/***********************************************************************
 * Task 2
 * -------
 * Handle a request from a client in a synchronized way.
 * Call 'task_3(shmp);' and enclose it by correct calls of
 * sem_post and sem_wait with the available semaphores.
 *
 * Hints:
 * - the synchronization protocol is already defined by the client
 *   (see pseudocode in 'clientps.c')
 * - don't forget that signals can interrupt certain blocking
 *   synchronization calls!
 * - the 'quit' flag is set by the signal handler
 *
 * See also: sem_overview(7)
 ***********************************************************************/

void task_2(void)
{
    while (!quit) {
        if(sem_wait(sem_server) == -1){
            error_exit("sem_wait failed");
        }

        task_3(shmp);
        
        if(sem_post(sem_ready) == -1){
            error_exit("sem_post failed");
        }
    }
}

/***********************************************************************
 * Task 3
 * -------
 *
 * Iterate through bank_accounts and adjust balance based on the command.
 * If a bank account is not found, set 'amount' to '-1'.
 *
 * Hints:
 * - the structures that store the bank accounts can be found in
 *   'server.h'
 *
 ***********************************************************************/

void task_3(shm_data_t * shmp)
{
    int iban_found = 0;
    for(int i = 0; i < num_bank_accounts; ++i){
        if(strcmp(bank_accounts[i].iban,shmp->iban) == 0){
            if(shmp->cmd == DEPOSIT){
                bank_accounts[i].balance += shmp->amount;
            }else{
                bank_accounts[i].balance -= shmp->amount;
            }
            shmp->amount = bank_accounts[i].balance;
            iban_found = 1;
            break;
        }
    }
    if(iban_found == 0){
        shmp->amount = -1;
    }
}

void free_resources(void)
{
    if (shmp != MAP_FAILED) {
        if (munmap(shmp, sizeof *shmp) == -1) {
            print_error("Could not unmap shared memory");
        }
        shmp = MAP_FAILED;
    }

    if (shmfd != -1) {
        (void) close(shmfd);
        if (shm_unlink(SHM_NAME) == -1) {
            print_error("Could not remove shared memory");
        }
        shmfd = -1;
    }

    if (sem_server != SEM_FAILED) {
        (void) sem_close(sem_server);
        if (sem_unlink(SEM_NAME_SERVER) == -1) {
            print_error("Could not remove semaphore");
        }
        sem_server = SEM_FAILED;
    }

    if (sem_client != SEM_FAILED) {
        (void) sem_close(sem_client);
        if (sem_unlink(SEM_NAME_CLIENT) == -1) {
            print_error("Could not remove semaphore");
        }
        sem_client = SEM_FAILED;
    }

    if (sem_ready != SEM_FAILED) {
        (void) sem_close(sem_ready);
        if (sem_unlink(SEM_NAME_READY) == -1) {
            print_error("Could not remove semaphore");
        }
        sem_ready = SEM_FAILED;
    }
}
