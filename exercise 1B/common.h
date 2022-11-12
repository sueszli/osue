#ifndef COMMON_H
#define COMMON_H

#include <sys/types.h>
#include <unistd.h>

// print macros ::
#ifdef DEBUG
#define log(fmt, ...)                                                  \
  fprintf(stderr, "==%d== [%s:%d] " fmt, getpid(), __FILE__, __LINE__, \
          ##__VA_ARGS__);

#define logEdgeList(msg, edgeList)                                  \
  fprintf(stderr, "==%d== [%s:%d] ", getpid(), __FILE__, __LINE__); \
  fprintf(stderr, "%s: ", msg);                                     \
  for (size_t i = 0; i < edgeList.numEdges; i++) {                  \
    char *left = edgeList.fst[i].from;                              \
    char *right = edgeList.fst[i].to;                               \
    fprintf(stderr, "(%s-%s) ", left, right);                       \
  }                                                                 \
  fprintf(stderr, "\n");

#define logNodeList(msg, nodeList)                                  \
  fprintf(stderr, "==%d== [%s:%d] ", getpid(), __FILE__, __LINE__); \
  fprintf(stderr, "%s: ", msg);                                     \
  size_t len = 0;                                                   \
  while (nodeList[len] != NULL) {                                   \
    fprintf(stderr, "%s ", nodeList[len]);                          \
    len++;                                                          \
  }                                                                 \
  fprintf(stderr, "\n");

#else
#define log(fmt, ...)              /* NOP */
#define logEdgeList(msg, edgeList) /* NOP */
#define logNodeList(msg, nodeList) /* NOP */
#endif

#define error(s)                                   \
  fprintf(stderr, "%s: %s\n", s, strerror(errno)); \
  exit(EXIT_FAILURE);

#define argumentError(s)                                                       \
  fprintf(stderr, "%s\n", s);                                                  \
  fprintf(stderr, "%s\n",                                                      \
          "This program uses multiple generators working in parallel and a "   \
          "supervisor which manages them and logs the best solution.");        \
  fprintf(stderr, "%s\n", "---");                                              \
  fprintf(stderr, "%s\n", "SUPERVISOR USAGE:");                                \
  fprintf(stderr, "\t%s\n", "supervisor");                                     \
  fprintf(stderr, "%s\n", "---");                                              \
  fprintf(stderr, "%s\n", "GENERATOR USAGE:");                                 \
  fprintf(stderr, "\t%s\n", "generator EDGE1...");                             \
  fprintf(stderr, "%s\n", "GENERATOR EXAMPLE:");                               \
  fprintf(stderr, "\t%s\n", "# Each 'x-y' argument below is a directed edge"); \
  fprintf(stderr, "\t%s\n", "# from vertice 'x' to vertice 'y' in a graph.");  \
  fprintf(stderr, "\t%s\n", "generator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0");  \
  exit(EXIT_FAILURE);
// :: print macros

// graph ::
typedef struct {
  char *from;
  char *to;
} Edge;

typedef struct {
  size_t numEdges;
  Edge *fst;
} EdgeList;

typedef char **NodeList;
// :: graph

// buffer ::
#define MAX_SOLUTION_SIZE                                                    \
  (999) /**< max num of edges in submitted solutions -> set by assignment to \
           be at least 8 */

#define SHM_NAME "/shm" /**< name of shared memory in '/dev/shm/<SHM_NAME>' */

typedef struct {
} CircularBufferSHM;

// :: buffer

#endif
