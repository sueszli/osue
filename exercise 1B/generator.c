/**
 * @file generator.c
 * @author Yahya Jabary <mailto: yahya.jabary@tuwien.ac.at>
 * @date 09.11.2022
 *
 * @brief This worker creates feedback arc sets and sends them to the supervisor
 * through shared memory.
 *
 * @details This worker process consists of 3 segments:
 * First, the Initialization: Parsing and validating user arguments, opening up
 * the shared memory used for communication between manager and workers and
 * semaphores used to synchronize all processes.
 * Second, the Main Loop: Generating solutions and sending them to the manager
 * through the shared memory. Writing to the shared memory is synchronized using
 * semaphores and mutual exclusion between all processes is ensured.
 * Third, the cleanup: Cleaning up all resources which were allocated during
 * the initializtion.
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

//#region parsing
/**
 * @brief Parses the command line arguments into the given EdgeList and
 * validates them.
 *
 * @param edgeList Pointer to the EdgeList to be filled.
 * @param argc Number of command line arguments.
 * @param argv Command line arguments.
 */
static void parseEdges(EdgeList edgeList, int argc, char **argv) {
  for (size_t i = 1; i < argc; i++) {
    char *argument = argv[i];

    // validate
    size_t numDashes = 0;
    for (size_t j = 0; j < strlen(argument); j++) {
      if (argument[j] == '-') {
        numDashes++;
      }
    }
    if (numDashes != 1) {
      argumentError("Each argument must have exactly one '-' character");
    }

    char *left = strtok(argument, "-");
    char *right = strtok(NULL, "-");
    edgeList.fst[i - 1] = (Edge){.from = left, .to = right};
  }
}

/**
 * @brief Checks whether the given nodeList contains the given node.
 *
 * @param nodeList List of nodes.
 * @param node Node to be searched for.
 * @return true if the node is in the list, false otherwise.
 */
static bool contains(NodeList nodeList, char *input) {
  for (size_t i = 0; nodeList[i] != NULL; i++) {
    if (strcmp(nodeList[i], input) == 0) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Creates a set of nodes without duplicates from the given edgeList.
 *
 * @param edgeList List of edges (parsed from command line arguments).
 * @param nodeList List of nodes to be determined from the edgeList.
 *
 * @return The filled nodeList, reallocated with only the necessary size to save
 * memory space.
 */
static char **parseNodes(NodeList nodeList, EdgeList edgeList) {
  char **top = nodeList;
  size_t size = 0;

  for (size_t i = 0; i < edgeList.numEdges; i++) {
    char *node1 = edgeList.fst[i].from;
    if (!contains(nodeList, node1)) {
      *top = node1;
      top++;
      size++;
    }
    char *node2 = edgeList.fst[i].to;
    if (!contains(nodeList, node2)) {
      *top = node2;
      top++;
      size++;
    }
  }

  char **newNodeList = realloc(nodeList, (size + 1) * sizeof(char *));
  if (newNodeList == NULL) {
    error("realloc failed");
  }
  newNodeList[size] = NULL;
  return newNodeList;
}
//#endregion

//#region solving
/**
 * @brief Sets a randomization seed for the shuffle function by using the
 * given path to a file in a unix system.
 *
 * @param path Path to a file in a unix system.
 * @return Boolean indicating whether the seed was set successfully.
 */
static bool setSeedFromOS(const char *path) {
  FILE *in = fopen(path, "r");
  if (in == NULL) {
    return false;
  }
  unsigned int seed;

  if (fread(&seed, sizeof seed, 1, in) != -1) {
    fflush(in);
    fclose(in);
    srand(seed);
    return true;
  }
  return false;
}

/**
 * @brief Tries to set a randomization seed for the shuffle function by using
 * the given path to a file in a unix system. If this fails, a seed is set
 * using the current time.
 */
static void setRandomSeed(void) {
  // increase performance by using better seeds if possible
  // seeds are similar when multiple processes get started simultaneously
  const char *path1 = "/dev/random";
  const char *path2 = "/dev/urandom";
  const char *path3 = "/dev/arandom";

  if (setSeedFromOS(path1)) {
    log("Set seed from '%s'\n", path1);
  } else if (setSeedFromOS(path2)) {
    log("Set seed from '%s'\n", path2);
  } else if (setSeedFromOS(path3)) {
    log("Set seed from '%s'\n", path3);
  } else {
    log("%s\n", "Failed to set seed from OS path");
    srand(time(NULL));
  }
}

/**
 * @brief Shuffles the given list of nodes.
 * @param nodeList List of nodes to be shuffled.
 */
static void shuffle(NodeList list) {
  size_t len = 0;
  while (list[len] != NULL) {
    len++;
  }

  for (size_t i = 0; i < len; i++) {
    size_t j = i + rand() / (RAND_MAX / (len - i) + 1);
    char *tmp = list[j];
    list[j] = list[i];
    list[i] = tmp;
  }
}

/**
 * @brief Checks whether the nodes in the given edge contradict the position of
 * the nodes in the given nodeList based on the Monte-Carlo algorithm for
 * randomized minimal feedback arc sets.
 *
 * @param edge Edge to be checked.
 * @param nodeList List of nodes based on which this function checks whether the
 * edge contradicts the order or not.
 * @return true if the edge contradicts the order, false otherwise.
 */
bool contradictsOrder(Edge edge, NodeList nodeList) {
  size_t fromIndex = 0;
  size_t toIndex = 0;
  for (size_t i = 0; nodeList[i] != NULL; i++) {
    if (strcmp(nodeList[i], edge.from) == 0) {
      fromIndex = i;
    }
    if (strcmp(nodeList[i], edge.to) == 0) {
      toIndex = i;
    }
  }
  return fromIndex > toIndex;
}

/**
 * @brief Generates a randomized solution based on the Monte-Carlo algorithm for
 * minimal feedback arc sets.
 *
 * @param edgeList List of edges (parsed from command line arguments).
 * @param nodeList List of nodes (generated from the edges from the commandline
 * args).
 * @return A solution to the given problem as a list of edges.
 */
static EdgeList genSolution(EdgeList allEdges, NodeList nodePermutation) {
  EdgeList solution = {.numEdges = allEdges.numEdges,
                       .fst = malloc(allEdges.numEdges * sizeof(Edge))};
  if (solution.fst == NULL) {
    error("malloc failed");
  }

  shuffle(nodePermutation);

  size_t solutionCounter = 0;
  for (size_t i = 0; i < allEdges.numEdges; i++) {
    Edge e = allEdges.fst[i];
    if (contradictsOrder(e, nodePermutation)) {
      solution.fst[solutionCounter] = e;
      solutionCounter++;
    }
  }
  solution.numEdges = solutionCounter;

  EdgeList rSolution = {
      .numEdges = solution.numEdges,
      .fst = realloc(solution.fst, solution.numEdges * sizeof(Edge))};
  if (rSolution.fst == NULL) {
    error("realloc failed");
  }
  return rSolution;
}
//#endregion

/**
 * @brief This function consists of the following segments:
 * 1. Parsing the command line arguments.
 * 2. Initializing the resources needed to send the solution to the manager.
 * 3. Main loop: Generating a randomized solution and sending it to the manager
 * in a mutually exclusive way.
 * 4. Cleaning up the resources.
 *
 * @param argc Number of command line arguments.
 * @param argv List of command line arguments.
 * @return EXIT_SUCCESS if the program ran successfully, EXIT_FAILURE otherwise.
 */
int main(int argc, char **argv) {
  if (argc < 2) {
    argumentError("At least one argument required");
  }

  // parse
  const int numEdges = argc - 1;
  EdgeList allEdges = {.numEdges = numEdges,
                       .fst = malloc(numEdges * sizeof(Edge))};
  if (allEdges.fst == NULL) {
    error("malloc failed");
  }
  parseEdges(allEdges, argc, argv);
  NodeList allNodes = calloc(2 * numEdges * sizeof(char *), sizeof(char *));
  if (allNodes == NULL) {
    error("calloc failed");
  }
  allNodes = parseNodes(allNodes, allEdges);
  logEdgeList("All edges", allEdges);
  log("Num of all edges: %zu\n", allEdges.numEdges);
  logNodeList("All nodes", allNodes);

  // open shared memory -> shm_open()
  int shm_fd = shm_open(SHM_PATH, O_RDWR, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    if (errno == ENOENT) {
      error("Supervisor process must be running before the generators");
    } else {
      error("shm_open failed");
    }
  }

  // map shared memory into memory -> mmap()
  CircularBuffer *shm = mmap(NULL, sizeof(CircularBuffer),
                             PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm == MAP_FAILED) {
    error("mmap failed");
  }

  // open semaphores -> sem_open()
  sem_t *sem_used_space = sem_open(SEM_USED_SPACE_PATH, 0);
  if (sem_used_space == SEM_FAILED) {
    error("sem_open failed");
  }
  sem_t *sem_available_space = sem_open(SEM_AVAILABLE_SPACE_PATH, 0);
  if (sem_available_space == SEM_FAILED) {
    error("sem_open failed");
  }
  sem_t *sem_mutex = sem_open(SEM_MUTEX_PATH, 0);
  if (sem_mutex == SEM_FAILED) {
    error("sem_open failed");
  }

  // increment generator counter
  shm->numGenerators++;
  log("New generator created - total: %d\n", shm->numGenerators);

  setRandomSeed();
  while ((shm->terminate) == false) {
    EdgeList solution = genSolution(allEdges, allNodes);
    if (solution.numEdges <= MAX_SOLUTION_SIZE) {
      // alternating mutex: wait for free space -> sem_wait()
      if ((sem_wait(sem_available_space) == -1) && (errno != EINTR)) {
        error("sem_wait failed");
      }
      if (shm->terminate == true) {
        break;  // if woken up by supervisor, terminate
      }

      // write solution (critical section) -> sem_wait() + sem_post()
      if ((sem_wait(sem_mutex)) == -1 && (errno != EINTR)) {
        error("sem_wait failed");
      }
      logEdgeList("Solution", solution);
      shm->buffer[shm->writeIndex] = solution;
      shm->writeIndex = (shm->writeIndex + 1) % BUFFER_SIZE;
      if (sem_post(sem_mutex) == -1) {
        error("sem_post failed");
      }

      // alternating mutex: signal space used -> sem_post()
      if (sem_post(sem_used_space) == -1) {
        error("sem_post failed");
      }
    }
    free(solution.fst);
  }

  // decrement generator counter
  log("Terminating generator - total: %d\n", shm->numGenerators);
  shm->numGenerators--;

  // unmap shared memory -> munmap()
  if (munmap(shm, sizeof(CircularBuffer)) == -1) {
    error("munmap failed");
  }

  // close shared memory -> close()
  if (close(shm_fd) == -1) {
    error("close failed");
  }

  // close semaphores -> sem_close()
  if (sem_close(sem_used_space) == -1) {
    error("sem_close failed");
  }
  if (sem_close(sem_available_space) == -1) {
    error("sem_close failed");
  }
  if (sem_close(sem_mutex) == -1) {
    error("sem_close failed");
  }

  free(allEdges.fst);
  free(allNodes);

  exit(EXIT_SUCCESS);
}