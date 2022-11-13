#ifndef SEM_UTIL_H
#define SEM_UTIL_H

#include <semaphore.h>

void handle_sem_close(sem_t *sem, char *program_name);
void handle_sem_unlink(char *sem, char *program_name);

#endif