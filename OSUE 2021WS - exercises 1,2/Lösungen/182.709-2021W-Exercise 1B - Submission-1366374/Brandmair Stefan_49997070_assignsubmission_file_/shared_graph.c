#include "shared_graph.h"
#include <stdlib.h>

struct SharedMemory createSharedMemory(int numberOfEntries, bool isServer)
{
  struct SharedMemory shm = {0};
  shm.name = SG_SHM_NAME;

  int flags = O_RDWR;
  if (isServer)
  {
    flags = flags | O_CREAT;
  }
  shm.fileDescriptor = shm_open(shm.name, flags, 0777);
  if (shm.fileDescriptor == -1)
  {
    // Why can't C have proper error handling
    shm.size = -1;
    return shm;
  }

  shm.size = sizeof(struct ShmMetadata) + numberOfEntries * sizeof(struct ShmSolution);
  if (isServer)
  {
    if (ftruncate(shm.fileDescriptor, shm.size) < 0)
    {
      shm.size = -1;
      return shm;
    }
  }

  return shm;
}

int openSharedMemory(struct SharedMemory *shm, struct SharedMemoryData *data)
{
  data->metadata = NULL;
  data->solutions = NULL;

  void *sharedMemory = mmap(NULL, shm->size, PROT_READ | PROT_WRITE, MAP_SHARED, shm->fileDescriptor, 0);
  data->metadata = sharedMemory;
  data->metadata->quit = 0;
  data->metadata->readIndex = 0;
  data->metadata->writeIndex = 0;
  data->solutions = (void *)((char *)sharedMemory + sizeof(struct ShmMetadata));

  if (data->metadata == MAP_FAILED || data->solutions == MAP_FAILED)
  {
    return -1;
  }

  return 0;
}

int closeSharedMemory(struct SharedMemory *shm, struct SharedMemoryData *data)
{
  int returnCode = 0;
  if (munmap(data->metadata, shm->size) == -1)
  {
    returnCode = -1;
  }
  if (close(shm->fileDescriptor) == -1)
  {
    returnCode = -1;
  }
  return returnCode;
}

int destroySharedMemory(struct SharedMemory *shm)
{
  int returnCode = 0;

  if (shm_unlink(shm->name) == -1)
  {
    returnCode = -1;
  }
  return returnCode;
}
