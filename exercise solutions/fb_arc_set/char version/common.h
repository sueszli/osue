#ifndef COMMON
#define COMMON

#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define usage(msg)                                                             \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "Wrong usage: %s\nSYNOPSIS "                                       \
            "SUPERVISOR:\n\tsupervisor\nSYNOPSIS GENERATOR:\n\tgenerator "     \
            "EDGE1...\nEXAMPLE:\n\tgenerator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 " \
            "6-0\n",                                                           \
            msg);                                                              \
    exit(EXIT_FAILURE);                                                        \
  } while (0);

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

#define printEdgeList(edgeList)                                     \
  printf("Solution with %d edges: ", edgeList.size);                \
  for (int i = 0; i < edgeList.size; i++) {                         \
    printf("%c-%c ", edgeList.edges[i].from, edgeList.edges[i].to); \
  }                                                                 \
  printf("\n");

#define MAX_EDGELIST_SIZE (64)  // because we can't use pointers in shm

typedef struct {
  char from;
  char to;
} Edge;

typedef struct {
  Edge edges[MAX_EDGELIST_SIZE];
  int size;  // type is int because of argc
} EdgeList;

#define MAX_SOLUTION_SIZE (64)   // only write into shm if list size is smaller
#define SHM_PATH "/11912007shm"  // full path: '/dev/shm/11912007shm'
#define BUF_SIZE (16)

typedef struct {
  bool terminate;         // tell generators to terminate (no mutex needed)
  int generator_counter;  // also uses 'write_mutex'

  EdgeList buf[BUF_SIZE];
  int write_index;  // index for next write into buffer
  int read_index;   // index for next read from buffer

  sem_t num_free;     // free space - used for alternating reads and writes
  sem_t num_used;     // used space - used for alternating reads and writes
  sem_t write_mutex;  // mutual exclusion for generator writes
} ShmStruct;

#endif