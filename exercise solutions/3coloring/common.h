#ifndef COMMON
#define COMMON

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (true)

#define usage(msg)                                                            \
  do {                                                                        \
    fprintf(stderr, "%s\n", msg);                                             \
    fprintf(                                                                  \
        stderr,                                                               \
        "SYNOPSIS\n\t./generator EDGE1...\nEXAMPLE\n\tgenerator 0-1 0-2 0-3 " \
        "1-2 1-3 2-3");                                                       \
    exit(EXIT_FAILURE);                                                       \
  } while (0)

#define printEdgeList(msg, edgeList)                \
  printf("%s ", msg);                               \
  for (size_t i = 0; i < edgeList.len; i++) {       \
    printf("%ld-%ld ", edgeList.edges[i].from.name, \
           edgeList.edges[i].to.name);              \
  }                                                 \
  printf("\n");

enum Color { RED = 0, GREEN = 1, BLUE = 2 };

typedef struct Node {
  size_t name;
  enum Color color;
} Node;

typedef struct Edge {
  Node from;
  Node to;
} Edge;

#define MAX_NUM_EDGES (1024)

typedef struct EdgeList {
  Edge edges[MAX_NUM_EDGES];  // array to avoid pointer in shared memory
  size_t len;
} EdgeList;

#define BUF_LEN (32)
#define SHM_PATH "/11912007shm"

typedef struct Shm_t {
  bool terminateGenerators;
  size_t numGenerators;

  EdgeList buf[BUF_LEN];
  size_t readIndex;
  size_t writeIndex;

  sem_t writeMutex;  // write mutex for generators
  sem_t numUsed;     // num of used indices in buffer (= supervisor should read)
  sem_t numFree;     // num of used indices in buffer (= generator can write)
} Shm_t;

#endif