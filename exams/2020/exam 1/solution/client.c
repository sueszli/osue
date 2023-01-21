#include "client.h"

char *program_name = NULL;

char *shmp = MAP_FAILED;
int shmfd;
sem_t *sem_request = SEM_FAILED;
sem_t *sem_response = SEM_FAILED;
sem_t *sem_client = SEM_FAILED;

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
  if (argc < 2) {
    usage("too few arguments");
  }
  if (argc > 3) {
    usage("too many arguments");
  }

  bool optP = false;

  int opt = 0;
  while ((opt = getopt(argc, argv, "p:")) != -1) {
    switch (opt) {
      case 'p':
        if (optP) {
          usage("repeated use of same option");
        }
        optP = true;
        if (optarg[0] == '-') {
          usage("missing argument for option p");
        }
        args->password = optarg;
        printf("-p %s\n", args->password);
        break;

      default:
        usage("illegal option");
        break;
    }
  }
  if (!optP) {
    usage("no p option");
  }
  if ((argc - optind) != 0) {
    usage("illegal positional argument");
  }
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
  shmfd = shm_open(SHM_NAME, PERMISSIONS, 0);
  if (shmfd == -1) {
    error_exit("shm_open");
  }

  shmp = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
  if (shmp == MAP_FAILED) {
    error_exit("mmap");
  }

  sem_request = sem_open(SEM_NAME_REQUEST, 0);
  if (sem_request == SEM_FAILED) {
    error_exit("sem_open");
  }

  sem_response = sem_open(SEM_NAME_RESPONSE, 0);
  if (sem_response == SEM_FAILED) {
    error_exit("sem_open");
  }

  sem_client = sem_open(SEM_NAME_CLIENT, 0);
  if (sem_client == SEM_FAILED) {
    error_exit("sem_open");
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
 * sem_wait(sem_client); init = 1
 *
 * write to shared memory;
 * sem_post(sem_request); init = 1
 * ___________________________________________________________
 *
 *                                  sem_wait(sem_request) init = 0
 *                                  server processes request;
 *                                  sem_post(sem_response)
 * ___________________________________________________________
 *
 * sem_wait(sem_response);
 * read response;
 *
 * sem_post(sem_client);
 *
 ******************************************************************************/
void process_password(const char *password, char *hash) {
  if (sem_wait(sem_client) == -1) {
    error_exit("sem_wait");
  }

  strcpy(shmp, password);

  if (sem_post(sem_request) == -1) {
    error_exit("sem_post");
  }

  if (sem_wait(sem_response) == -1) {
    error_exit("sem_wait");
  }

  strcpy(hash, shmp);

  if (sem_post(sem_client) == -1) {
    error_exit("sem_post");
  }
}

int main(int argc, char *argv[]) {
  args_t args;
  program_name = argv[0];
  char hash[SHM_SIZE] = {0};
  initialize_names();

  // first exercise
  parse_arguments(argc, argv, &args);

  // second exercise
  allocate_resources();

  // third exercise
  process_password(args.password, hash);

  printf("Hash: %s\n", hash);

  print_message("detach shared memory");
  free_resources();

  return EXIT_SUCCESS;
}
