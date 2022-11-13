/**
 * @author Stefan Brandmair
 * @date 2021-11-01
 *
 * @brief Generates random solutions to a three-color problem instance.
 * @details It generates a solution and writes it to a shared memory.
 **/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
// #include <ctype.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <string.h>
#include "shared_graph.h"

char *PROGRAM_NAME;

/**
 * @brief Reads the edges that have been passed to the program and returns the highest vertex ID.
 * @param edgesCount number of edges
 * @param argv the argv that has been passed to the main, potentially shifted a bit so that we skip the program name
 * @return highest vertex ID or -1
 */
static int getMaxVertexId(int edgesCount, char *argv[]);

/**
 * @brief Reads the edges and vertices that have been passed to the program
 * @param edgesCount number of edges
 * @param argv the argv that has been passed to the main, potentially shifted a bit so that we skip the program name
 * @param vertexCount number of vertices
 * @param vertices we'll write the vertices into this already allocated array
 * @param edges we'll write the edges into this already allocated array
 * @return 0 on success
 */
static int readVerticesAndEdges(int edgesCount, char *argv[], int vertexCount, struct Vertex *vertices, struct Edge *edges);

/**
 * @brief Assigns random "colors" to the vertices
 * @param graph our graph/problem
 */
static void randomColors(struct Graph *graph);

/**
 * @brief Finds all edges with the same color and notes them down in solutionEdges
 * @param graph our graph/problem
 * @param solutionEdgesSize the size of solutionEdges
 * @param solutionEdges where the edges to be deleted are noted down
 * @return size of solution or -1
 */
static int getSolution(struct Graph *graph, int solutionEdgesSize, struct Edge *solutionEdges);

static void printError(const char *message)
{

  fprintf(stderr, "[%s] %s with error %d %s", PROGRAM_NAME, message, errno, strerror(errno));
}

int main(int argc, char *argv[])
{
  PROGRAM_NAME = argv[0];

  if (argc <= 1)
  {
    fprintf(stderr, "[%s] Not enough arguments (edges)", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  struct SharedMemory shm = createSharedMemory(NUMBER_OF_ENTRIES, false);
  if (shm.size == -1)
  {
    fprintf(stderr, "[%s] Getting shared memory failed, try starting the supervisor first", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  struct SharedMemoryData shmData;
  if (openSharedMemory(&shm, &shmData) == -1)
  {
    printError("Opening shared memory failed");
    exit(EXIT_FAILURE);
  }

  sem_t *semFree = sem_open(SG_SEM_FREE_NAME, 0);
  sem_t *semUsed = sem_open(SG_SEM_USED_NAME, 0);
  sem_t *semWriters = sem_open(SG_SEM_WRITERS_NAME, 0);

  if (semFree == SEM_FAILED || semUsed == SEM_FAILED || semWriters == SEM_FAILED)
  {
    fprintf(stderr, "[%s] opening semaphore failed", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  // We'll assume that the IDs are mostly sequential. Thus, we can use an array to keep track of them.
  int edgesCount = argc - 1;
  int maxVertexId = getMaxVertexId(edgesCount, argv + 1);
  if (maxVertexId == -1)
  {
    fprintf(stderr, "[%s] Invalid edge detected", PROGRAM_NAME);
    exit(EXIT_FAILURE);
  }

  int vertexCount = maxVertexId + 1;
  printf("[%s] Read %d vertices\n", PROGRAM_NAME, maxVertexId + 1);
  fflush(stdout);
  struct Vertex vertices[vertexCount]; // TODO: Use malloc instead of a variable length array
  struct Edge edges[edgesCount];

  if (readVerticesAndEdges(edgesCount, argv + 1, vertexCount, vertices, edges) != 0)
  {
    printError("Reading vertices and edges failed");
    exit(EXIT_FAILURE);
  }

  srand(time(NULL));

  struct Edge solutionEdges[MAX_SOLUTION_SIZE];

  struct Graph graph;
  graph.edgesCount = edgesCount;
  graph.edges = edges;
  graph.vertexCount = vertexCount;
  graph.vertices = vertices;

  int bestSolutionSize = MAX_SOLUTION_SIZE + 1;
  while (true)
  {
    // Quitty quit
    if (shmData.metadata->quit != 0)
    {
      break;
    }

    randomColors(&graph);

    int solutionSize = getSolution(&graph, MAX_SOLUTION_SIZE, solutionEdges);
    if (solutionSize < 0)
    {
      // Invalid solution, just ignore it
      continue;
    }
    if (solutionSize >= bestSolutionSize)
    {
      // Crappy solution, just ignore it
      continue;
    }

    // Print solution
    /*
    fprintf(stdout, "Solution: ");
    for (int i = 0; i < solutionSize; i++)
    {
      fprintf(stdout, "%d-%d ", solutionEdges[i].vertexAId, solutionEdges[i].vertexBId);
    }
    fprintf(stdout, "\n");
    */

    // Write solution to shared memory
    if (sem_wait(semWriters) == -1)
    {
      printError("Writer semaphore failed");
    }
    if (sem_wait(semFree) == -1)
    {
      printError("Freeing semaphore failed");
    }

    int writeIndex = shmData.metadata->writeIndex;
    shmData.solutions[writeIndex].size = solutionSize;
    for (int i = 0; i < solutionSize; i++)
    {
      shmData.solutions[writeIndex].edges[i] = solutionEdges[i];
    }

    shmData.metadata->writeIndex = (writeIndex + 1) % NUMBER_OF_ENTRIES;
    if (sem_post(semUsed) == -1)
    {
      printError("Posting semaphore failed");
    }
    if (sem_post(semWriters) == -1)
    {
      printError("Writer semaphore failed");
    }
  }
  printf("\n");

  if (closeSharedMemory(&shm, &shmData) == -1)
  {
    printError("Error closing the shared memory");
    exit(EXIT_FAILURE);
  }

  return 0;
}

int getSolution(struct Graph *graph, int solutionEdgesSize, struct Edge *solutionEdges)
{
  int edgesCount = graph->edgesCount;
  int solutionIndex = 0;
  for (int i = 0; i < edgesCount; i++)
  {
    int vertexAId = graph->edges[i].vertexAId;
    int vertexBId = graph->edges[i].vertexBId;

    // Same color, time to nuke it :P
    if (graph->vertices[vertexAId].color == graph->vertices[vertexBId].color)
    {
      if (solutionIndex >= solutionEdgesSize)
      {
        return -1;
      }

      // Copy the edge into the solution
      solutionEdges[solutionIndex] = graph->edges[i];

      solutionIndex += 1;
    }
  }

  return solutionIndex;
}

void randomColors(struct Graph *graph)
{
  int vertexCount = graph->vertexCount;
  struct Vertex *vertices = graph->vertices;
  for (int i = 0; i < vertexCount; i++)
  {
    vertices[i].color = rand() % 3;
  }
}

int readVerticesAndEdges(int edgesCount, char *argv[], int vertexCount, struct Vertex *vertices, struct Edge *edges)
{
  int edgeIndex = 0;

  int vertexIdA = -1;
  int vertexIdB = -1;
  for (int i = 0; i < edgesCount; i++)
  {
    if (sscanf(argv[i], "%d-%d", &vertexIdA, &vertexIdB) != 2)
    {
      return -1;
    }

    if (vertexIdA >= vertexCount || vertexIdB >= vertexCount || edgeIndex > edgesCount)
    {
      return -1;
    }

    vertices[vertexIdA].id = vertexIdA;
    vertices[vertexIdA].color = 0;

    vertices[vertexIdB].id = vertexIdB;
    vertices[vertexIdB].color = 0;

    edges[edgeIndex].vertexAId = vertexIdA;
    edges[edgeIndex].vertexBId = vertexIdB;

    edgeIndex += 1;
  }

  // Invalid number of edges
  if (edgesCount != edgeIndex)
  {
    return -2;
  }

  return 0;
}

int getMaxVertexId(int edgesCount, char *argv[])
{
  int maxVertexId = -1;
  int vertexIdA = -1;
  int vertexIdB = -1;

  // Read all valid edges
  for (int i = 0; i < edgesCount; i++)
  {
    if (sscanf(argv[i], "%d-%d", &vertexIdA, &vertexIdB) != 2)
    {
      return -1;
    }

    if (vertexIdA > maxVertexId)
    {
      maxVertexId = vertexIdA;
    }
    if (vertexIdB > maxVertexId)
    {
      maxVertexId = vertexIdB;
    }
  }

  return maxVertexId;
}