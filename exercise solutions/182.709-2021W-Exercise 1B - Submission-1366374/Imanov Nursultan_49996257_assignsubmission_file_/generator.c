/**
 * @file generator.c
 * @author Nursultan Imanov <01528474@student.tuwien.ac.at>
 * @date 01.11.2021
 * 
 * @brief Program that makes a graph 3-colorable by removing the least edges possible.
 * @details In order to reduce the runtime of the algorithm,
 * multiple processes generate the random sets of edges in parallel and 
 * report their results to a supervisor process, which remembers the set with the least edges.
 */

#include "3coloring.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <time.h>

const char *progname = "generator";

sem_t *used_sem;
sem_t *free_sem;
sem_t *mutex_sem;

struct buffer *buffer;

static void usage(void);
static void init_shm(void);
static void cleanup(void);
static void termination(void);
static int findNodeMax(char **argv);
static void findSolution(solution_t *solution, int N, char **argv);
static void generateSolutions(char **argv);
static void process_arguments(int argc, char **argv);

/**
 * @brief The generator program takes as arguments the set of edges of the graph:
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
  srand(time(NULL));
  process_arguments(argc, argv);

  if (argc < 2)
  {
    usage();
  }

  generateSolutions(argv);
  exit(EXIT_SUCCESS);
}

/**
 * @brief documents correct calling interface
 * @details options are not allowed
 */
static void usage(void)
{
  fprintf(stderr, "USAGE: %s EDGE1...\n", progname);
  exit(1);
}

/**
 * @brief 
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
static void process_arguments(int argc, char **argv)
{
  int c;
  while ((c = getopt(argc, argv, "")) != -1)
  {
    usage();
  }
}

/**
 * @brief initializes shared memory and semaphores
 * 
 */
static void init_shm(void)
{

  int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
  if (shmfd == -1)
  {
    fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (buffer == MAP_FAILED)
  {
    fprintf(stderr, "[%s] ERROR: mmap failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (close(shmfd) == -1)
  {
    fprintf(stderr, "[%s] ERROR: close failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  used_sem = sem_open(SEM_1, 0);
  free_sem = sem_open(SEM_2, 0);
  mutex_sem = sem_open(SEM_3, 0);

  if (used_sem == SEM_FAILED || free_sem == SEM_FAILED || mutex_sem == SEM_FAILED)
  {
    fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief cleans up shared memory and close semaphores
 * 
 */
static void cleanup(void)
{
  if (munmap(buffer, sizeof(*buffer)) == -1)
  {
    fprintf(stderr, "[%s] ERROR: munmap failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (sem_close(used_sem) == -1 || sem_close(free_sem) == -1 || sem_close(mutex_sem) == -1)
  {
    fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }
}

/**
 * @brief is called from the supervisor process to terminate the program
 * and clean up the resources.
 * 
 */
static void termination(void)
{
  cleanup();
  exit(EXIT_SUCCESS);
}

/**
 * @brief Finds the number of nodes in the graph
 * 
 * @param argv 
 * @return int 
 */
static int findNodeMax(char **argv)
{
  int node_max = 0;
  for (int i = 1; argv[i] != NULL; i++)
  {
    char *ptr;
    int from = strtol(argv[i], &ptr, 10);
    ptr++;
    int to = strtol(ptr, &ptr, 10);

    if (from > node_max)
    {
      node_max = from;
    }
    else if (to > node_max)
    {
      node_max = to;
    }
  }
  return node_max + 1;
}

/**
 * @brief Finds the solution of the graph
 * @details Nodes array is used to assign colors to the nodes and
 * adjacency matrix is used to store the edges of the graph.
 * If adjMatrix[i][j] == 1, then there is an edge between node i and node j.
 * The edge connecting 2 nodes with the same color should be removed.
 * 
 * @param solution 
 * @param N 
 * @param argv 
 */
static void findSolution(solution_t *solution, int N, char **argv)
{
  solution->edges_count = 0;

  int adjMat[N][N];
  int nodes[N];

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      adjMat[i][j] = 0;
    }
  }

  int i = 1;

  while (argv[i] != NULL)
  {
    char *ptr;
    int from = strtol(argv[i], &ptr, 10);
    ptr++;
    int to = strtol(ptr, &ptr, 10);
    adjMat[from][to] = 1;
    adjMat[to][from] = 1;

    i++;
  }

  for (int i = 0; i < N; i++)
  {
    nodes[i] = (rand() % 3) + 1;
    printf("%d", nodes[i]);
  }
  printf("\n");

  for (int i = 0; i < N; i++)
  {
    for (int j = 0; j < N; j++)
    {
      if (adjMat[i][j] == 0)
      {
        continue;
      }

      if (nodes[i] == nodes[j])
      {
        adjMat[i][j] = 0;
        adjMat[j][i] = 0;
        edge_t edge = {.from = i, .to = j};
        solution->edge[solution->edges_count] = edge;
        solution->edges_count += 1;
        printf("Solution: %d %d\n", i, j);
      }
    }
  }
}

/**
 * @brief simultaniously finds a better solution and
 * writes to the circular buffer.
 * 
 * @param argv 
 */
static void generateSolutions(char **argv)
{
  init_shm();
  int N = findNodeMax(argv);

  while (1)
  {
    if (buffer->quit == 1)
    {
      termination();
    }
    sem_wait(mutex_sem);

    sem_wait(free_sem);

    int wr_pos = buffer->wr_pos;

    solution_t *solution;
    solution = malloc(sizeof(solution_t));

    findSolution(solution, N, argv);

    buffer->solutions[wr_pos] = *solution;

    printf("critical: %s: i = %d\n", argv[0], wr_pos);
    sem_post(used_sem);

    wr_pos += 1;
    wr_pos %= BUFFER_LENGTH;
    buffer->wr_pos = wr_pos;
    free(solution);
    sem_post(mutex_sem);
  }
}
