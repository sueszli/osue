#ifndef _SERVER_H_
#define _SERVER_H_

#include "common.h"

/** A bank account. */
typedef struct {
    char iban[LEN_IBAN];
    int  balance;
} bank_account_t;

/** Number of accounts in the array `bank_accounts`. */
extern const size_t num_bank_accounts;
/** Array of bank accounts. */
extern bank_account_t bank_accounts[];

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
