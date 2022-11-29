#ifndef COMMON
#define COMMON

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define errorHandler(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)


typedef struct {
  
}


struct shmbuf {

};

#endif