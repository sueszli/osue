#include "common.h"

#define usage(msg)                                                      \
  do {                                                                  \
    fprintf(stderr, "Wrong usage: %s\nSYNOPSIS:\n\tsupervisor\n", msg); \
    exit(EXIT_FAILURE);                                                 \
  } while (0);

static volatile sig_atomic_t quit = false;
static void onSignal(int sig, siginfo_t *si, void *unused) { quit = true; }

static void initSignalListener(void) {
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
}

static void readSubmission(ShmStruct *shmp) {
  // side-effect: may change state of static variable 'best'
  // pre-condition: must be called in mutually exclusive zone

  static int best;

  EdgeList submission = shmp->buf[shmp->read_index];
  shmp->read_index = (shmp->read_index + 1) % BUF_SIZE;

  if (submission.size == 0) {
    printf("%s\n", "Input graph is acyclic");
    quit = true;
  }

  // if (submission.size < best) {
  if (true) {
    best = submission.size;
    printf("[./supervisor] Solution with %d edges: ", submission.size);
    for (int i = 0; i < submission.size; i++) {
      Edge e = submission.edges[i];
      printf("%c-%c ", e.from, e.to);
    }
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    usage("no arguments allowed");
  }

  initSignalListener();

  // create shared memory
  int fd = shm_open(SHM_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    error("shm_open");
  }
  if (ftruncate(fd, sizeof(ShmStruct)) == -1) {
    error("ftruncate");
  }
  ShmStruct *shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  // initialize shared memory's content
  shmp->terminate = false;
  shmp->write_index = 0;
  shmp->read_index = 0;
  if (sem_init(&(shmp->num_free), 1, BUF_SIZE) == -1) {
    error("sem_init");
  }
  if (sem_init(&(shmp->num_used), 1, 0) == -1) {
    error("sem_init");
  }
  if (sem_init(&(shmp->write_mutex), 1, 1) == -1) {
    error("sem_init");
  }

  // read generator submissions
  while (!quit) {
    if (sem_wait(&shmp->num_used) == -1) {
      error("sem_wait");
      if (errno != EINTR) {  // ie. keyboard signal
        break;
      }
    }
    readSubmission(shmp);
    if (sem_post(&shmp->num_free) == -1) {
      error("sem_post");
    }
  }

  // shutdown generators
  shmp->terminate = true;

  // destroy semaphores in shared memory
  if (sem_destroy(&(shmp->num_free)) == -1) {
    error("sem_destroy");
  }
  if (sem_destroy(&(shmp->num_used)) == -1) {
    error("sem_destroy");
  }
  if (sem_destroy(&(shmp->write_mutex)) == -1) {
    error("sem_destroy");
  }

  // close shared memory
  if (munmap(shmp, sizeof(*shmp)) == -1) {
    error("munmap");
  }
  if (shm_unlink(SHM_PATH) == -1) {
    error("shm_unlink");
  }
  if (close(fd) == -1) {
    error("close");
  }

  return EXIT_SUCCESS;
}