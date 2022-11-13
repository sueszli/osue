#include "myutil.h"

#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int mySemClose(sem_t * sem, char * arg) {
  int ret_val = 0;
  if (sem_close(sem) != 0) {
    fprintf(stderr, "%s Error @ sem_close() ! Errno: %s\n", arg, strerror(errno));
    ret_val = -1;
  }
  return ret_val;
}

int mySemOpen(sem_t * sem, char * arg) {
  int ret_val = 0;
  if (sem == SEM_FAILED) {
    fprintf(stderr, "%s Error @ sem_open() ! Errno: %s\n", arg, strerror(errno));
    ret_val = -1;
  }
  return ret_val;
}

int mySemPost(sem_t * sem, char * arg) {
  int ret_val = sem_post(sem);
  if (ret_val == -1) {
    fprintf(stderr, "%s Error @ sem_post() ! Errno: %s\n", arg, strerror(errno));
  }
  return ret_val;
}

int mySemWait(sem_t * sem, char * arg) {
  int ret_val = sem_wait(sem);
  if (ret_val == -1) {
    fprintf(stderr, "%s Error @ sem_post() ! Errno: %s\n", arg, strerror(errno));
  }
  return ret_val;
}