/**
 * @file SemUtils.h
 * @author Alexander Gube <e12023988@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief this module provides convenice functions for semaphores
 *
 **/

#include <semaphore.h>

#define SYNCH_GEM "/12023988synchGenSem"        //ensures that only one generator may access the circular buffer at a time  -> init to 1
#define READ_SEM "/12023988readSem"             //ensures that reading header does not exceed writing header                -> init to 0
#define WRITE_SEM "/12023988writeSem"           //ensures that yet unread data is overwritten                               -> init to num of entries of buffer

/**
 * @brief close a semaphore
 * @param sem semaphore to close
 * @param progName name of program
 * @return 0 on success, -1 on error
 * */
int closeSEM(sem_t *sem, char *progName);

/**
 * @brief unlink a semaphore
 * @param name name of semaphore to unlink
 * @param progName name of program
 * @return 0 on success, -1 on error
 * */
int unlinkSEM(char *name, char *progName);

/**
 * @brief cleanup a semaphore (close and unlink)
 * @param sem semaphore to close
 * @param name name of semaphore to unlink
 * @param progName name of program
 * @return 0 on success, -1 on error
 * */
void cleanupSEM(sem_t *sem, char *name, char *progName);
