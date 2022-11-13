#ifndef SHARED_GRAPH_H
#define SHARED_GRAPH_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define NUMBER_OF_ENTRIES 10
#define MAX_SOLUTION_SIZE 8

struct Vertex
{
  int id;
  int color;
};

struct Edge
{
  int vertexAId;
  int vertexBId;
};

struct Graph
{
  int vertexCount;
  struct Vertex *vertices;
  int edgesCount;
  struct Edge *edges;
};

struct ShmMetadata
{
  int quit;
  int readIndex;
  int writeIndex;
};

struct ShmSolution
{
  int size;
  struct Edge edges[MAX_SOLUTION_SIZE];
};

struct SharedMemory
{
  const char *name;
  int fileDescriptor;
  int size;
};

struct SharedMemoryData
{
  struct ShmMetadata *metadata;
  struct ShmSolution *solutions;
};

#define SG_SHM_NAME "/12024754_shm"
#define SG_SEM_FREE_NAME "/12024754_free"
#define SG_SEM_USED_NAME "/12024754_used"
#define SG_SEM_WRITERS_NAME "/12024754_readers"

/**
 * @brief Creates a shared memory for our graph
 * @param numberOfEntries how many solutions should fit into this shared memory
 * @return a shared memory wrapper struct. If the size is -1, then a failure occurred.
 */
struct SharedMemory createSharedMemory(int numberOfEntries, bool isServer);

/**
 * @brief Opens a shared memory for our graph
 * @param shm shared memory to use
 * @param data shared memory data, will be written to by this function
 * @return 0 on success, -1 on failure
 */
int openSharedMemory(struct SharedMemory *shm, struct SharedMemoryData *data);

/**
 * @brief Closes a shared memory for our graph
 * @param shm shared memory to use
 * @param numberOfEntries how many solutions should fit into this shared memory
 * @param data shared memory data
 * @return 0 on success, -1 on failure
 */
int closeSharedMemory(struct SharedMemory *shm, struct SharedMemoryData *data);

/**
 * @brief Destroys a shared memory for our graph
 * @param shm shared memory to use
 * @param numberOfEntries how many solutions should fit into this shared memory
 * @return 0 on success, -1 on failure
 */
int destroySharedMemory(struct SharedMemory *shm);
#endif