/**
 * @author Stefan Brandmair
 * @date 2021-11-01
 *
 * @brief Supervises a bunch of 3 color solution generators.
 * @details It creates a shared memory, some semaphores and then waits for the generators to submit their solutions.
 **/

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <semaphore.h>
#include "shared_graph.h"

// Fun test case
// for i in {1..10}; do (./generator 0-1 0-2 0-3 1-28 1-29 2-30 2-31 3-32 3-33 4-6 4-14 4-16 5-7 5-15 5-17 6-7 6-18 7-19 8-9 8-12 8-23 9-13 9-22 10-15 10-19 10-25 11-14 11-18 11-24 12-17 12-27 13-16 13-26 14-23 15-22 16-21 17-20 18-21 19-20 20-31 21-30 22-33 23-32 24-27 24-29 25-26 25-29 26-28 27-28 30-33 31-32 &); done

char *PROGRAM_NAME;
const int MAX_SHARED_MEMORY_SIZE = 4096;

/**
 * @brief Closes and unlinks a semaphore. Accesses PROGRAM_NAME. Prints an error message to stderr on failure
 * @param semaphore the semaphore to close
 * @param name the name to unlink
 * @return 0 on success, -1 on failure
 */
static int semCloseAndUnlink(sem_t *semaphore, const char *name);

/** Quit when it's not 0 anymore */
volatile int quit = 0;

void handleSignal(int signal)
{
  quit = 1;
}

int main(int argc, char *argv[])
{
  PROGRAM_NAME = argv[0];

  // Signals
  struct sigaction signalAction = {
      .sa_handler = handleSignal};

  sigaction(SIGINT, &signalAction, NULL);
  sigaction(SIGTERM, &signalAction, NULL);

  struct SharedMemory shm = createSharedMemory(NUMBER_OF_ENTRIES, true); // Definitely not over 4 Kibibytes
  if (shm.size == -1)
  {
    fprintf(stderr, "[%s] Creating shared memory failed", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  struct SharedMemoryData shmData;
  if (openSharedMemory(&shm, &shmData) == -1)
  {
    fprintf(stderr, "[%s] Opening shared memory failed", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  sem_unlink(SG_SEM_FREE_NAME);
  sem_unlink(SG_SEM_USED_NAME);
  sem_unlink(SG_SEM_WRITERS_NAME);
  sem_t *semFree = sem_open(SG_SEM_FREE_NAME, O_CREAT | O_EXCL, 0777, NUMBER_OF_ENTRIES);
  sem_t *semUsed = sem_open(SG_SEM_USED_NAME, O_CREAT | O_EXCL, 0777, 0);
  sem_t *semWriters = sem_open(SG_SEM_WRITERS_NAME, O_CREAT | O_EXCL, 0777, 1);

  if (semFree == SEM_FAILED || semUsed == SEM_FAILED || semWriters == SEM_FAILED)
  {
    fprintf(stderr, "[%s] opening semaphore failed", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  struct Edge *solutionEdges = NULL;
  int solutionEdgesSize = -1;
  while (quit == 0)
  {
    if (sem_wait(semUsed) == -1)
    {
      fprintf(stderr, "[%s] killed supervisor", PROGRAM_NAME);
      quit = 1;
      break;
    }

    // Read a solution
    int readIndex = shmData.metadata->readIndex;
    struct ShmSolution solution = shmData.solutions[readIndex];
    shmData.metadata->readIndex = (readIndex + 1) % NUMBER_OF_ENTRIES;

    int oldSolutionSize = solutionEdgesSize;
    bool betterSolution = oldSolutionSize == -1 || solution.size < oldSolutionSize;

    if (solutionEdges == NULL)
    {
      solutionEdges = malloc(solution.size * sizeof(struct Edge));
      solutionEdgesSize = solution.size;
    }

    if (betterSolution)
    {
      solutionEdgesSize = solution.size;
      for (int i = 0; i < solutionEdgesSize; i++)
      {
        solutionEdges[i].vertexAId = solution.edges[i].vertexAId;
        solutionEdges[i].vertexBId = solution.edges[i].vertexBId;
      }

      // Time to brag about it
      fprintf(stdout, "[%s] Solution: ", PROGRAM_NAME);
      for (int i = 0; i < solutionEdgesSize; i++)
      {
        fprintf(stdout, "%d-%d ", solutionEdges[i].vertexAId, solutionEdges[i].vertexBId);
      }
      fprintf(stdout, "\n");
    }

    // Perfect solution!
    if (solutionEdgesSize == 0)
    {
      fprintf(stdout, "[%s] Perfect solution found, yay!", PROGRAM_NAME);
      quit = 1;
    }

    if (sem_post(semFree) == -1)
    {
      fprintf(stderr, "[%s] posting semaphore failed", PROGRAM_NAME);
      quit = 1;
    }
  }

  // Is this actually safe?
  shmData.metadata->quit = 1;

  // Cleanup
  {
    if (solutionEdges != NULL)
    {
      free(solutionEdges);
    }

    bool errorDuringCleanup = false;

    if (closeSharedMemory(&shm, &shmData) == -1)
    {
      fprintf(stderr, "[%s] Error closing the shared memory", PROGRAM_NAME);
      errorDuringCleanup = true;
    }

    if (destroySharedMemory(&shm) == -1)
    {
      fprintf(stderr, "[%s] Error destroying the shared memory", PROGRAM_NAME);
      errorDuringCleanup = true;
    }

    semCloseAndUnlink(semFree, SG_SEM_FREE_NAME);
    semCloseAndUnlink(semUsed, SG_SEM_USED_NAME);
    semCloseAndUnlink(semWriters, SG_SEM_WRITERS_NAME);

    if (errorDuringCleanup)
    {
      exit(EXIT_FAILURE);
    }
  }

  return 0;
}

int semCloseAndUnlink(sem_t *semaphore, const char *name)
{
  int returnCode = 0;
  if (sem_close(semaphore) == -1)
  {
    fprintf(stderr, "[%s] closing the semaphore %s failed", PROGRAM_NAME, name);
    returnCode = -1;
  }

  if (sem_unlink(name) == -1)
  {
    fprintf(stderr, "[%s] unlinking the semaphore %s failed", PROGRAM_NAME, name);
    returnCode = -1;
  }

  return returnCode;
}