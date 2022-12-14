#include "common.h"

static volatile sig_atomic_t quit = false;
static void onSignal(int sig, siginfo_t *si, void *unused) { quit = true; }
static void initSignalHandler(void) {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = onSignal;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    error("sigaction");
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    error("sigaction");
  }
  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    error("sigaction");
  }
}

static void readSolution(Shm_t *shmp) {
  EdgeList solution = shmp->buf[shmp->readIndex];
  shmp->readIndex = (shmp->readIndex + 1) % BUF_LEN;

  if (solution.len == 0) {
    printf("The graph is 3-colorable!\n");
    quit = true;
    return;
  }

  static size_t bestLen = ULONG_MAX;
  if (solution.len < bestLen) {
    bestLen = solution.len;
    printf("[./supervisor] Solution with %ld edges:", solution.len);
    printEdgeList("", solution);
  }
}

int main(int argc, char *argv[]) {
  initSignalHandler();

  int fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    error("shm_open");
  }
  if (ftruncate(fd, sizeof(Shm_t)) == -1) {
    error("ftruncate");
  }
  Shm_t *shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  shmp->terminateGenerators = false;
  shmp->numGenerators = 0;
  shmp->readIndex = 0;
  shmp->writeIndex = 0;
  if (sem_init(&shmp->writeMutex, true, 1) == -1) {
    error("sem_init");
  }
  if (sem_init(&shmp->numUsed, true, 0) == -1) {
    error("sem_init");
  }
  if (sem_init(&shmp->numFree, true, BUF_LEN) == -1) {
    error("sem_init");
  }

  while (!quit) {
    if (sem_wait(&shmp->numUsed) == -1) {
      if ((errno == EINTR) && (shmp->numGenerators == 0)) {
        perror("Quit before running generators");
        break;
      }
      if (errno == EINTR) {
        perror("Quit while waiting for read");
        break;
      }
      error("sem_wait");
    }

    readSolution(shmp);

    if (sem_post(&shmp->numFree) == -1) {
      error("sem_post");
    }
  }

  printf("Terminating the %ld active generators...\n", shmp->numGenerators);
  shmp->terminateGenerators = true;
  for (size_t i = 0; i < shmp->numGenerators; i++) {
    if (sem_post(&shmp->numFree) == -1) {
      perror("sem_post - error occured while freeing the waiting generators");
    }
  }

  if (sem_destroy(&(shmp->writeMutex)) == -1) {
    error("sem_destroy");
  }
  if (sem_destroy(&(shmp->numUsed)) == -1) {
    error("sem_destroy");
  }
  if (sem_destroy(&(shmp->numFree)) == -1) {
    error("sem_destroy");
  }

  if (munmap(shmp, sizeof(*shmp)) == -1) {
    error("munmap");
  }
  if (shm_unlink(SHM_PATH) == -1) {
    error("shm_unlink");
  }
  if (close(fd) == -1) {
    error("close");
  }
  exit(EXIT_SUCCESS);
}
