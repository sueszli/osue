#include "server.h"

const char *pgm_name = NULL;

static int shmfd = -1;
static shm_data_t *shmp = MAP_FAILED;

static sem_t *sem_server = SEM_FAILED;
static sem_t *sem_ready = SEM_FAILED;
static sem_t *sem_client = SEM_FAILED;

static volatile sig_atomic_t quit = 0;

void task_1a(void);
void task_1b(void);
void task_2(void);
void task_3(shm_data_t *);
void free_resources(void);

static void signal_handler(int sig) { quit = 1; }

int main(int argc, char *argv[]) {
  pgm_name = argv[0];

  // Register signal handlers
  struct sigaction s;
  s.sa_handler = signal_handler;
  s.sa_flags = 0;  // no SA_RESTART!
  if (sigfillset(&s.sa_mask) < 0) {
    error_exit("sigfillset");
  }
  if (sigaction(SIGINT, &s, NULL) < 0) {
    error_exit("sigaction SIGINT");
  }
  if (sigaction(SIGTERM, &s, NULL) < 0) {
    error_exit("sigaction SIGTERM");
  }

  if (atexit(free_resources) == -1) {
    error_exit("atexit failed");
  }

  // allocate resources
  task_1a();
  task_1b();

  // service loop (calls task_3)
  task_2();

  fprintf(stderr, "server exiting regularly\n");
  return EXIT_SUCCESS;
}

/***********************************************************************
 * Task 1
 * ------
 * Allocate resources for interprocess communication.
 * General remarks:
 * - Creation of the shared resources should fail if they already exist!
 * - Use PERMISSIONS (defined in common.h), which grants read and write
 *   permissions to owner, group and others.
 * - You can use the provided function 'error_exit' to exit your program
 *   in case of an error.
 * - You *MUST* use the POSIX interfaces.
 ***********************************************************************/

/***********************************************************************
 * Task 1a
 * -------
 * Create and map a named POSIX shared memory object.
 *
 * Use global variable 'shmfd' for creation.
 * Let global variable 'shmp' point to the shared memory area.
 *
 * Hints:
 * - Use SHM_NAME as name for the shared memory object (defined in common.h).
 * - Do NOT set 'shmfd' back to -1 (if you do, unlinking will fail in
 *   'free_resources')! You do not need to close the file descriptor after
 *   mapping.
 * - Don't forget to set an appropriate size for the shared memory object.
 *
 * See also: shm_overview(7), ftruncate(2), mmap(2)
 ***********************************************************************/

void task_1a(void) {
  int shmfd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, PERMISSIONS);
  if (shmfd == -1) {
    error_exit("shm_open");
  }

  if (ftruncate(shmfd, sizeof(*shmp)) == -1) {
    error_exit("ftruncate");
  }

  shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (shmp == MAP_FAILED) {
    error_exit("mmap");
  }
}

/***********************************************************************
 * Task 1b
 * -------
 * Allocate the POSIX semaphores.
 *
 * Let global variables 'sem_server', 'sem_ready', and 'sem_client' point
 * to the created semaphores.
 *
 * Hints:
 * - Use SEM_NAME_SERVER, SEM_NAME_CLIENT and SEM_NAME_READY as names
 *   for the semaphores (defined in common.h).
 *
 * Note: In the demo solution, sem_client is initialized with 1.
 *
 * See also: sem_overview(7)
 ***********************************************************************/

void task_1b(void) {
  // client ready, free server to read
  sem_server = sem_open(SEM_NAME_READY, O_CREAT | O_EXCL, PERMISSIONS, 0);
  if (sem_server == SEM_FAILED) {
    error_exit("sem_open");
  }

  // server ready, free client to read
  sem_ready = sem_open(SEM_NAME_READY, O_CREAT | O_EXCL, PERMISSIONS, 0);
  if (sem_ready == SEM_FAILED) {
    error_exit("sem_open");
  }

  // client mutex
  sem_client = sem_open(SEM_NAME_CLIENT, O_CREAT | O_EXCL, PERMISSIONS, 1);
  if (sem_client == SEM_FAILED) {
    error_exit("sem_open");
  }
}

/***********************************************************************
 * Task 2
 * -------
 * Handle a request from a client in a synchronized way.
 * Call 'task_3(shmp);' and enclose it by correct calls of
 * sem_post and sem_wait with the available semaphores.
 *
 * Hints:
 * - the synchronization protocol is already defined by the client
 *   (see pseudocode in 'clientps.c')
 * - don't forget that signals can interrupt certain blocking
 *   synchronization calls!
 * - the 'quit' flag is set by the signal handler
 *
 * See also: sem_overview(7)
 ***********************************************************************/

void task_2(void) {
  while (!quit) {
    // task_2_DEMO(sem_server, sem_ready, sem_client, shmp);
    if (sem_wait(sem_server) == -1) {
      error_exit("sem_wait");
    }
    task_3(shmp);
    if (sem_post(sem_ready) == -1) {
      error_exit("sem_post");
    }
  }
}

/***********************************************************************
 * Task 3
 * -------
 *
 * Iterate through bank_accounts and adjust balance based on the command.
 * If a bank account is not found, set 'amount' to '-1'.
 *
 * Hints:
 * - the structures that store the bank accounts can be found in
 *   'server.h'
 *
 ***********************************************************************/

void task_3(shm_data_t *shmp) {
  bool found = false;
  for (int i = 0; i < num_bank_accounts; ++i) {
    if (strcmp(bank_accounts[i].iban, shmp->iban) == 0) {
      if (shmp->cmd == DEPOSIT) {
        bank_accounts[i].balance += shmp->amount;
      } else {
        bank_accounts[i].balance -= shmp->amount;
      }
      shmp->amount = bank_accounts[i].balance;
      found = true;
      break;
    }
  }
  if (!found) {
    shmp->amount = -1;
  }
}

void free_resources(void) {
  if (shmp != MAP_FAILED) {
    if (munmap(shmp, sizeof *shmp) == -1) {
      print_error("Could not unmap shared memory");
    }
    shmp = MAP_FAILED;
  }

  if (shmfd != -1) {
    (void)close(shmfd);
    if (shm_unlink(SHM_NAME) == -1) {
      print_error("Could not remove shared memory");
    }
    shmfd = -1;
  }

  if (sem_server != SEM_FAILED) {
    (void)sem_close(sem_server);
    if (sem_unlink(SEM_NAME_SERVER) == -1) {
      print_error("Could not remove semaphore");
    }
    sem_server = SEM_FAILED;
  }

  if (sem_client != SEM_FAILED) {
    (void)sem_close(sem_client);
    if (sem_unlink(SEM_NAME_CLIENT) == -1) {
      print_error("Could not remove semaphore");
    }
    sem_client = SEM_FAILED;
  }

  if (sem_ready != SEM_FAILED) {
    (void)sem_close(sem_ready);
    if (sem_unlink(SEM_NAME_READY) == -1) {
      print_error("Could not remove semaphore");
    }
    sem_ready = SEM_FAILED;
  }
}
