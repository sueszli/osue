#include "common.h"

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
  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    error("sigaction");
  }
}

static void readSubmission(ShmStruct *shmp) {
  // side-effect: may change state of static variable 'best'
  // pre-condition: must be called in mutually exclusive zone

  EdgeList submission = shmp->buf[shmp->read_index];
  shmp->read_index = (shmp->read_index + 1) % BUF_SIZE;

  if (submission.size == 0) {
    printEdgeList(submission);
    printf("graph is acyclic\n");
    quit = true;
    return;
  }

  static int best = INT_MAX;
  if (submission.size < best) {
    best = submission.size;
    printEdgeList(submission);
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

  // initialize shared memory
  shmp->terminate = false;
  shmp->generator_counter = 0;
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

  // read shared memory
  while (!quit) {
    if (sem_wait(&shmp->num_used) == -1) {
      if ((errno == EINTR) && (shmp->generator_counter == 0)) {
        perror("Quit before running generators");
        break;
      }
      if (errno == EINTR) {
        perror("Quit while waiting for read");
        break;
      }
      error("sem_wait");
    }

    readSubmission(shmp);

    if (quit) {
      break;
    }
    if (sem_post(&shmp->num_free) == -1) {
      error("sem_post");
    }
  }

  // terminate generators
  printf("Currently %d generators are active...\n", shmp->generator_counter);
  printf("Terminating generators...\n");
  shmp->terminate = true;
  for (int i = 0; i < shmp->generator_counter; i++) {
    if (sem_post(&shmp->num_free) == -1) {
      perror("sem_post - error while freeing the waiting generators");
    }
  }

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