/**
 * @file supervisor.c
 * @author Nursultan Imanov <01528474@student.tuwien.ac.at>
 * @date 01.11.2021
 * @brief Program that makes a graph 3-colorable by removing the least edges possible.
 * @details The supervisor sets up the shared memory and the semaphores 
 * and initializes the circular buffer required for the communication with the generators.
 */

#include "3coloring.h"
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

volatile sig_atomic_t quit = 0;
const char *progname = "supervisor";

sem_t *used_sem;
sem_t *free_sem;
sem_t *mutex_sem;

struct buffer *buffer;

static void cleanup(int status);
static void usage(void);
static void init_shm(void);
static void handle_signal(int signal);
static void process_arguments(int argc, char **argv);

/**
 * @brief 
 * @details in the main block supervisor program simultaneously reads solutions 
 * from the shared memory, produced by generators. If new solution is better than
 * the current one, it is shown in console and stored in the shared memory.
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char const *argv[])
{
  process_arguments(argc, (char **)argv);

  if (argc > 1)
  {
    usage();
  }

  init_shm();

  struct sigaction sa = {.sa_handler = handle_signal};
  sigaction(SIGINT, &sa, NULL);

  while (1)
  {
    sem_wait(used_sem);
    int rd_pos = buffer->rd_pos;

    solution_t solution = buffer->solutions[rd_pos];

    // printf("critical: %s: i = %d\n", argv[0], rd_pos);

    if (solution.edges_count < buffer->min_edges)
    {
      buffer->min_edges = solution.edges_count;

      if (buffer->min_edges == 0)
      {
        printf("The graph is 3-colorable!\n");
        buffer->quit = 1;
      }
      else
      {
        printf("Solution with %d edges(%d): ", solution.edges_count, buffer->min_edges);
        for (size_t i = 0; i < solution.edges_count; i++)
        {
          printf("%d-%d ", solution.edge[i].from, solution.edge[i].to);
        }
        printf("\n");
      }
    }
    rd_pos += 1;
    rd_pos %= BUFFER_LENGTH;
    buffer->rd_pos = rd_pos;

    sem_post(free_sem);

    if (buffer->quit)
    {
      break;
    }
  }

  cleanup(EXIT_SUCCESS);
}

/**
 * @brief documents correct calling interface
 * @details options are not allowed
 */
static void usage(void)
{
  fprintf(stderr, "USAGE: %s\n", progname);
  exit(EXIT_FAILURE);
}

/**
 * @brief process the command line arguments
 * @details options are not allowed
 * 
 * @param argc 
 * @param argv 
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
 * @brief set up the shared memory and the semaphores
 * @details
 */
static void init_shm(void)
{
  int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
  if (shmfd == -1)
  {
    fprintf(stderr, "[%s] ERROR: shm_open failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  if (ftruncate(shmfd, sizeof(struct buffer)) == -1)
  {
    fprintf(stderr, "[%s] ERROR: ftruncate failed: %s\n", progname, strerror(errno));
    cleanup(EXIT_FAILURE);
  }

  buffer = mmap(NULL, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (buffer == MAP_FAILED)
  {
    fprintf(stderr, "[%s] ERROR: mmap failed: %s\n", progname, strerror(errno));
    exit(EXIT_FAILURE);
  }

  buffer->quit = 0;
  buffer->min_edges = MAX_EDGES;

  used_sem = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, 0);
  free_sem = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, MAX_DATA);
  mutex_sem = sem_open(SEM_3, O_CREAT | O_EXCL, 0600, 1);

  if (used_sem == SEM_FAILED || free_sem == SEM_FAILED || mutex_sem == SEM_FAILED)
  {
    fprintf(stderr, "[%s] ERROR: sem_open failed: %s\n", progname, strerror(errno));
    cleanup(EXIT_FAILURE);
  }

  if (close(shmfd) == -1)
  {
    fprintf(stderr, "[%s] ERROR: close failed: %s\n", progname, strerror(errno));
    cleanup(EXIT_FAILURE);
  }
}

/**
 * @brief handle signals, clean up and notify the generators to quit
 * @details 
 * 
 * @param signal 
 */
static void handle_signal(int signal)
{
  if (signal == SIGINT || signal == SIGTERM)
  {
    buffer->quit = 1;
    cleanup(EXIT_SUCCESS);
    printf("handle_signal\n");
    exit(EXIT_SUCCESS);
  }
}

/**
 * @brief clean up shared memory and semaphores
 * @details
 * 
 * @param status to indicate success or failure
 */
static void cleanup(int status)
{

  if (munmap(buffer, sizeof(*buffer)) == -1)
  {
    fprintf(stderr, "[%s] ERROR: munmap failed: %s\n", progname, strerror(errno));
  }
  if (shm_unlink(SHM_NAME) == -1)
  {
    fprintf(stderr, "[%s] ERROR: shm_unlink failed: %s\n", progname, strerror(errno));
  }

  if (sem_close(used_sem) == -1 || sem_close(free_sem) == -1 || sem_close(mutex_sem) == -1)
  {
    fprintf(stderr, "[%s] ERROR: sem_close failed: %s\n", progname, strerror(errno));
  }

  if (sem_unlink(SEM_1) == -1 || sem_unlink(SEM_2) == -1 || sem_unlink(SEM_3) == -1)
  {
    fprintf(stderr, "[%s] ERROR: sem_unlink failed: %s\n", progname, strerror(errno));
  }

  exit(status);
}
