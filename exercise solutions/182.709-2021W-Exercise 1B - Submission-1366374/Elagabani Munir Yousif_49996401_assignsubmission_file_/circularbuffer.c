#include "circularbuffer.h"

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static sem_t *free_sem, *used_sem, *write_sem;
static fbArcBuffer_t *buffer;
static int shdfd;
static bool isServer;
static char *progname;

void write_buffer(fbArc_t val) {
  sem_wait(write_sem);
  if (sem_wait(free_sem) == -1) {
    if (errno == EINTR) {
      fprintf(stderr, "%s: sem_wait failed: %s\n", progname, strerror(errno));
      return;
    }
  }

  buffer->solutions[buffer->wr_pos] = val;
  if (sem_post(used_sem) == -1) {
    if (errno == EINTR) {
      fprintf(stderr, "%s: sem_post failed: %s\n", progname, strerror(errno));
      return;
    }
  }
  buffer->wr_pos += 1;
  buffer->wr_pos %= BUFFER_LENGTH;
  sem_post(write_sem);
}

fbArc_t read_buffer(void) {
  if (sem_wait(used_sem) == -1) {
    if (errno == EINTR) {
      fprintf(stderr, "%s: sem_wait failed: %s\n", progname, strerror(errno));
      fbArc_t invalid = {.numEdges = -1};
      return invalid;
    }
  }
  fbArc_t val = buffer->solutions[buffer->rd_pos];
  if (sem_post(free_sem)) {
    if (errno == EINTR) {
      fprintf(stderr, "%s: sem_post failed: %s\n", progname, strerror(errno));
      fbArc_t invalid = {.numEdges = -1};
      return invalid;
    }
  }
  buffer->rd_pos += 1;
  buffer->rd_pos %= BUFFER_LENGTH;
  return val;
}

int load_buffer(bool server, char *progamname) {
  isServer = server;
  progname = progamname;

  shdfd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600);
  if (shdfd == -1) {
    fprintf(stderr, "%s: shm_open failed: %s\n", progname, strerror(errno));
    cleanup_buffer();
    return EXIT_FAILURE;
  }

  if (isServer) {
    if (ftruncate(shdfd, sizeof(fbArcBuffer_t)) == -1) {
      fprintf(stderr, "%s: ftruncate failed: %s\n", progname, strerror(errno));
      cleanup_buffer();
      return EXIT_FAILURE;
    }
  }

  buffer = mmap(NULL, sizeof(fbArcBuffer_t), PROT_READ | PROT_WRITE, MAP_SHARED,
                shdfd, 0);
  if (buffer == MAP_FAILED) {
    fprintf(stderr, "%s: mmap failed: %s\n", progname, strerror(errno));
    cleanup_buffer();
    return EXIT_FAILURE;
  }

  free_sem = sem_open(FREE_SEM_NAME, O_CREAT, 0600, BUFFER_LENGTH);
  used_sem = sem_open(USED_SEM_NAME, O_CREAT, 0600, 0);
  write_sem = sem_open(WRITE_SEM_NAME, O_CREAT, 0600, 1);
  if (free_sem == SEM_FAILED || used_sem == SEM_FAILED ||
      write_sem == SEM_FAILED) {
    fprintf(stderr, "%s: sem_open failed: %s\n", progname, strerror(errno));
    cleanup_buffer();
    return EXIT_FAILURE;
  }

  if (isServer) {
    buffer->rd_pos = 0;
    buffer->wr_pos = 0;
    buffer->bestSolutionSize = MAX_EDGES_FB_ARC + 1;
  }
  return EXIT_SUCCESS;
}

void cleanup_buffer(void) {
  munmap(buffer, sizeof(fbArcBuffer_t));
  close(shdfd);
  if (isServer) {
    shm_unlink(SHM_NAME);
  }
  sem_close(free_sem);
  sem_close(used_sem);
  sem_close(write_sem);
  if (isServer) {
    sem_unlink(FREE_SEM_NAME);
    sem_unlink(USED_SEM_NAME);
    sem_unlink(WRITE_SEM_NAME);
  }
}

void terminate(void) { buffer->bestSolutionSize = -1; }

bool shouldTerminate(void) { return buffer->bestSolutionSize == -1; }

int get_min_solution_size(void) { return buffer->bestSolutionSize; }

void set_min_solution_size(size_t size) { buffer->bestSolutionSize = size; }