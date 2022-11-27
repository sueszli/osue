/**
 * @file common.h
 * @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
 * @date 09.11.2022
 *
 * @brief This file contains the common definitions and functions used by all
 * processes.
 */

#ifndef COMMON_H
#define COMMON_H

//#region print macros
/*
 * Convenience macros for formatting error messages and exiting or
 * printing optional debug messages if the flag -DDEBUG is set.
 */
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
//#endregion

/**
 * @brief Number edges that a solution is allowed to have at most to be allowed
 * to be submitted. Can be chosen freely.
 * @invariant >= 8 (given by the assignment)
 */
#define MAX_SOLUTION_SIZE (64)

/*
 * Shared memory and semaphore paths.
 * see the path '/dev/shm/<name>' on local machine.
 */
#define SHM_PATH "/11912007shm"
#define SEM_USED_SPACE_PATH "/11912007used"
#define SEM_AVAILABLE_SPACE_PATH "/11912007available"
#define SEM_MUTEX_PATH "/11912007mutex"

typedef struct {
  char *from;
  char *to;
} Edge; /**< A directed edge from one vertice to another. */

typedef struct {
  size_t numEdges;
  Edge *fst;
} EdgeList; /**< A list of edges. */

typedef char **NodeList; /**< A list of nodes. */

#define BUFFER_SIZE (32)
typedef struct {
  EdgeList buffer[BUFFER_SIZE];
  uint8_t writeIndex;
  uint8_t readIndex;
  bool terminate;
  int numGenerators;
} CircularBuffer; /**< A circular buffer used as shared memory between
                     generators and supervisor. */

#endif