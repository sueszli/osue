#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "client.h"
#include "common.h"

/** Name of the executable (for printing messages). */
char *program_name = "<not yet set>";

/** Shared memory and semaphores. */
char *shmp = MAP_FAILED;
int shmfd;
sem_t *sem_request = SEM_FAILED;
sem_t *sem_response = SEM_FAILED;
sem_t *sem_client = SEM_FAILED;

FILE *inputFile = NULL;

// Incomplete names for semaphores and shared memory.
char SHM_NAME[256] = "/osue_shm_";
char SEM_NAME_REQUEST[256] = "/osue_request_";
char SEM_NAME_CLIENT[256] = "/osue_client_";
char SEM_NAME_RESPONSE[256] = "/osue_response_";

void initialize_names(void) {
  strcat(SHM_NAME, getlogin());
  strcat(SEM_NAME_REQUEST, getlogin());
  strcat(SEM_NAME_CLIENT, getlogin());
  strcat(SEM_NAME_RESPONSE, getlogin());
}

/************************************************************************
 * Task 1 - Argument parsing
 *
 * Implement argument parsing for the client.
 * Synopsis:
 *   ./client -p PASSWORD
 * Examples:
 * ./client -p password1234
 *
 * Read the password from the option argument of option -p.
 *
 * Call usage() if you encounter any invalid options or arguments. There is no
 * need to print a description of the problem.
 *
 * The parsed arguments should be stored in the args struct.
 * See client.h for the definition.
 *
 * @param argc Number of elements in argv
 * @param argv Array of command line arguments
 * @param args Struct with the parsed arguments
 *
 * Hints: getopt(3)
 ************************************************************************/
void parse_arguments(int argc, char *argv[], args_t *args) {
  int opt;
  int p_flag = 0;

  while ((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
      case 'p':
        args->password = optarg;
        ++p_flag;
        break;
      case '?':
        usage("err");
    }
  }

  if (argc != 4 || p_flag != 1) {
    usage("err");
  }

  char *input = argv[argc - 1];
}

/*******************************************************************************
 * Task 2 - Allocate shared resources
 *
 * Open a named POSIX shared memory object with the name SHM_NAME and map it.
 * Save the file descriptor in the global variable 'shmfd' and the address of
 * the mapping in 'shmp'.
 *
 * Open three POSIX named semaphores with the names SEM_NAME_REQUEST,
 * SEM_NAME_CLIENT and SEM_NAME_RESPONSE. Save their addresses in the global
 * variables 'sem_request', 'sem_client' and 'sem_response' respectively.
 *
 * Hints: shm_overview(7), ftruncate(2), mmap(2), sem_overview(7)
 ******************************************************************************/
void allocate_resources(void) {
  shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, PERMISSIONS);

  if (shmfd == -1) {
    usage("err");
  }

  shmp = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

  if (shmp == MAP_FAILED) {
    usage("err");
  }

  sem_request = sem_open(SEM_NAME_REQUEST, 0);

  if (sem_request == SEM_FAILED) {
    usage("err");
  }

  sem_response = sem_open(SEM_NAME_RESPONSE, 0);

  if (sem_response == SEM_FAILED) {
    usage("err");
  }

  sem_client = sem_open(SEM_NAME_CLIENT, 0);

  if (sem_client == SEM_FAILED) {
    usage("err");
  }
}

/*******************************************************************************
 * Task 3 - Process password
 *
 * Write the password to the shared memory, then instruct the server to
 * generate the hash and finally read back the resulting hash. The result has
 * to be copied to the buffer 'hash' provided by the caller. Add the missing
 * synchronization code to notify the server and ensure that only one client
 * can access the server at the same time.
 *
 * Pseudocode:
 *
 *      Client                      Server
 *
 * ???
 * write to shared memory;
 * ???
 * ___________________________________________________________
 *
 *                                  sem_wait(sem_request)
 *                                  server processes request;
 *                                  sem_post(sem_response)
 * ___________________________________________________________
 *
 * ???
 * read response;
 * ???
 *
 ******************************************************************************/
void process_password(const char *password, char *hash) {
  sem_wait(sem_client);
  strcpy(shmp, password);
  sem_post(sem_request);

  sem_wait(sem_response);
  strcpy(hash, shmp);
  sem_post(sem_client);
}

int main(int argc, char *argv[]) {
  args_t args;
  program_name = argv[0];
  char hash[SHM_SIZE] = {0};
  initialize_names();

  parse_arguments(argc, argv, &args);
  // DEMO_parse_arguments(argc, argv, &args);

  allocate_resources();
  // DEMO_allocate_resources();

  process_password(args.password, hash);
  // DEMO_process_password(args.password, hash);

  printf("Hash: %s\n", hash);

  // Free resources.
  print_message("detach shared memory");
  free_resources();

  return 0;
}
