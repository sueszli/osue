#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_NAME "/myshm12022502circbufA"
#define SHM_SIZE 8

#define SEM_NAME_USED "/sem12022502usedA"
#define SEM_NAME_FREE "/sem12022502freeA"
#define SEM_NAME_CLIENT "/sem12022502clientA"

#define PROGRAM_NAME "server.c"

void error_exit(char *message);

int main(int argc, char **argv) {
  // ------------------ SHM ------------------
  int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
  if (shmfd == -1) error_exit("shmfd");

  if (ftruncate(shmfd, SHM_SIZE) == -1) error_exit("ftruncate");

  char *shmp =
      mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (shmp == MAP_FAILED) error_exit("shmp");

  if (close(shmfd) == -1) error_exit("close shmfd");

  // ------------------ SEMAPHORE ------------------
  sem_unlink(SEM_NAME_USED);
  sem_unlink(
      SEM_NAME_FREE);  // in order to get rid of all already existing semaphores
  sem_unlink(SEM_NAME_CLIENT);
  sem_t *sem_used = sem_open(SEM_NAME_USED, O_CREAT | O_EXCL, 0600, 0);
  sem_t *sem_free = sem_open(SEM_NAME_FREE, O_CREAT | O_EXCL, 0600, SHM_SIZE);
  sem_t *sem_client = sem_open(SEM_NAME_CLIENT, O_CREAT | O_EXCL, 0600, 1);
  if (sem_used == SEM_FAILED) error_exit("sem setup");
  if (sem_free == SEM_FAILED) error_exit("sem server");
  if (sem_client == SEM_FAILED) error_exit("sem client");

  int read_pos = 0;
  int i = 0;
  for (i; i < 26; i++) {
    if (sem_wait(sem_used) == -1) error_exit("sem_wait used");
    printf("Server reads char: %c\n", shmp[read_pos]);
    read_pos++;
    read_pos %= SHM_SIZE;
    sem_post(sem_free);
  }

  if (munmap(shmp, SHM_SIZE) == -1) error_exit("munmap");
  if (shm_unlink(SHM_NAME) == -1) error_exit("shm_unlink");

  if (sem_close(sem_used) == -1) error_exit("sem_close setup");
  if (sem_close(sem_free) == -1) error_exit("sem_close server");
  if (sem_close(sem_client) == -1) error_exit("sem_close client");
  if (sem_unlink(SEM_NAME_USED) == -1) error_exit("sem_unlink setup");
  if (sem_unlink(SEM_NAME_FREE) == -1) error_exit("sem_unlink server");
  if (sem_unlink(SEM_NAME_CLIENT) == -1) error_exit("sem_unlink client");

  return EXIT_SUCCESS;
}

void error_exit(char *message) {
  fprintf(stderr, "%s: %s failed\n", PROGRAM_NAME, message);
  fprintf(stderr, "Error Code: %d\n", errno);
  exit(EXIT_FAILURE);
}
