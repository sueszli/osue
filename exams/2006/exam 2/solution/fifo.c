#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0)

#define SHM_PATH "/11912007shm"
#define BUF_SIZE (50)

typedef struct {
  sem_t write_mutex;
  sem_t num_used;
  sem_t num_free;
  int buf[BUF_SIZE];
  size_t write_index;
  size_t read_index;
} T_Fifo;

static int fd;
static T_Fifo *shmp;

static void initShm(void) {
  fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    error("shm_open");
  }

  if (ftruncate(fd, sizeof(T_Fifo)) == -1) {
    error("ftruncate");
  }

  shmp = mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  if (sem_init(&shmp->write_mutex, true, 1) == -1) {
    error("sem_init");
  }
  if (sem_init(&shmp->num_used, true, 0) == -1) {
    error("sem_init");
  }
  if (sem_init(&shmp->num_free, true, BUF_SIZE) == -1) {
    error("sem_init");
  }
}

static void removeShm(void) {
  if (sem_destroy(&shmp->write_mutex) == -1) {
    error("sem_destroy");
  }
  if (sem_destroy(&shmp->num_used) == -1) {
    error("sem_destroy");
  }
  if (sem_destroy(&shmp->num_free) == -1) {
    error("sem_destroy");
  }

  if (munmap(shmp, sizeof(*shmp)) == -1) {
    error("munmap");
  }

  if (close(fd) == -1) {
    error("close");
  }

  if (shm_unlink(SHM_PATH) == -1) {
    error("shm_unlink");
  }
}

static int getSize(void) {
  int numUsed = 0;
  if (sem_getvalue(&shmp->num_used, &numUsed) == -1) {
    error("sem_getvalue");
  }
  return numUsed;
}

static int put(const int data) {
  // returns current fifo size + 1

  if ((getSize() + 1) > BUF_SIZE) {
    return -1;
  }

  if (sem_wait(&shmp->num_free) == -1) {
    error("sem_wait");
  }

  shmp->buf[shmp->write_index] = data;
  shmp->write_index = (shmp->write_index + 1) % BUF_SIZE;
  printf("wrote: %d\n", data);

  if (sem_post(&shmp->num_used) == -1) {
    error("sem_post");
  }

  return getSize() + 1;
}

static int get(int *data) {
  // returns current fifo size - 1

  if (sem_wait(&shmp->num_used) == -1) {
    error("sem_wait");
  }

  *data = shmp->buf[shmp->read_index];
  shmp->read_index = (shmp->read_index + 1) % BUF_SIZE;
  printf("read: %d\n", *data);

  if (sem_post(&shmp->num_free) == -1) {
    error("sem_post");
  }

  return getSize() - 1;
}

int main(int argc, char *argv[]) {
  initShm();

  int out;
  put(1);
  put(2);
  put(3);
  put(4);
  put(5);
  printf("--> size: %d\n", getSize());
  get(&out);
  get(&out);
  get(&out);
  printf("--> size: %d\n", getSize());

  removeShm();
  exit(EXIT_SUCCESS);
}