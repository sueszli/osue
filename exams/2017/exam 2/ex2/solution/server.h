#ifndef _SERVER_H_
#define _SERVER_H_

#include "common.h"

/*
had to put the other imports from "server.c" here because the microsoft C linter
in VSCode forces you to put server.h at the top (which causes the type definitions
here to break) 
*/
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>


typedef struct {
    char iban[LEN_IBAN];
    int  balance;
} bank_account_t;
extern bank_account_t bank_accounts[];
extern const size_t num_bank_accounts; // bank_accounts[] size

/******************************************************************************
 * Function declarations (all functions below are already implemented)
 *****************************************************************************/

/** Print an error message to stderr.  */
void print_error(const char *msg);

/** Print an error message to stderr and exit with EXIT_FAILURE  */
void error_exit(const char *msg);

/** Declarations of demo solution implementations. */
void task_1a_DEMO(int *shmfd, shm_data_t **shmp);
void task_1b_DEMO(sem_t **sem_server, sem_t **sem_ready, sem_t **sem_client);
void task_2_DEMO(sem_t *sem_server, sem_t *sem_ready, sem_t *sem_client,
    shm_data_t *shmp);
void task_3_DEMO(shm_data_t *shmp);

#endif /* _SERVER_H_ */
