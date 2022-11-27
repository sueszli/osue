#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <semaphore.h>
#include "graph.h"

struct myshm {
  int readPos;
	int writePos;
  char data[MAX_ENTRIES][MAX_EDGES][4];
};

void shmError () {
  printf ("Error creating shared memory!\n");
  exit (1);
}

void semError() {
	printf("Error creating or removing semaphore!\n");
	exit(1);
}

int main (int argc, char **argv) {

  int fd = shm_open (SHM_NAME, O_RDWR, 0600);
  if (fd == -1)
    shmError ();
  struct myshm *myshm;
  myshm = mmap (NULL, sizeof (*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (myshm == MAP_FAILED)
    shmError ();
  if (close (fd) == -1)
    shmError ();

  sem_t *semFree = sem_open(SEM_FREE, 0);
	sem_t *semUsed = sem_open(SEM_USED, 0);
	sem_t *semWrite = sem_open(SEM_WRITE, 0);

  Graph g;
  g.edgeCount = argc - 1;
  g.edges = (Edge *) malloc (g.edgeCount * sizeof (Edge));

  int maxIdx = 0;

  for (int i = 1; i < argc; i++) {
    Edge e;
    e.v1 = argv[i][0] - '0';
    e.v2 = argv[i][2] - '0';
    g.edges[i - 1] = e;

    if (e.v1 > maxIdx)
      maxIdx = e.v1;
    if (e.v2 > maxIdx)
      maxIdx = e.v2;
  }

  g.vertexCount = maxIdx + 1;
  g.vertices = (Vertex *) malloc (g.vertexCount * sizeof (Vertex));
  for (int i = 0; i < g.vertexCount; i++)
    {
      Vertex v;
      v.index = i;
      g.vertices[i] = v;
    }

  srand (time (0));


  while (1) {
    //random color for each vertex
    for (int i = 0; i < g.vertexCount; i++) {
      g.vertices[i].color = rand () % 3;
    }
    //show edges that need to be removed
    for (int i = 0; i < g.edgeCount; i++) {
      sem_wait (semWrite);
      if (hasSameVertexColor (g, g.edges[i]))
        printf ("%d-%d ", g.vertices[g.edges[i].v1].index,g.vertices[g.edges[i].v2].index);
      sem_post (semWrite);
    }
    printf("\n");
    break;
  }

  if (munmap (myshm, sizeof (*myshm)) == -1)
    shmError ();

  if(sem_close(semFree) == -1)
		semError();
	if(sem_close(semUsed) == -1)
		semError();
	if(sem_close(semWrite) == -1)
		semError();

  free (g.edges);
  free (g.vertices);
}
