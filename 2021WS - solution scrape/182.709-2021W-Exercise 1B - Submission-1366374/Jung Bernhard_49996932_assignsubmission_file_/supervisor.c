#include "myutil.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

/**
 * @brief Variable for the signal handler so it can be determined if SIGINT or SIGTERM has been received.
 */
volatile sig_atomic_t sig_atom_quit = 0;

/**
 * @brief This Semaphore is declared globally, so it can be posted in case of SIGINT or SIGTERM signal
 */
sem_t * sem_used;

/**
 * @brief Custom signal handler function for the signal SIGINT and SIGTERM
 * @param singal unused variable required by system 
 * @return void
 */
static void handle_signal(int signal);

/**
 * @file supervisor.c
 * @author Bernhard Jung 12023965
 * @brief  My Implementtation of Exercise 1B: The Supervisor process for reading the results from the shared memory ringbuffer
 * @details The supervisors mains task is to coordinate the generator processes via shared memory and semaphores
 * it achieves this goal, by intializing semaphores and shared memory. The results are read from the shared memory ringbuffer
 * Each time an element has been read from the ringbuffer, a semaphore is being posted to enable generator processes to writer another result into the buffer. 
 * @date 2021.11.14
 */
int main(int argc, char ** argv) {

  // Decleration later needed variables, struct and signals
  int shm_fd;
  shmRegion_t * shm_struct;
  int exit_state = EXIT_SUCCESS;

  signal(SIGTERM, handle_signal);
  signal(SIGINT, handle_signal);


  //Setting up the shared memory on serverside + error handeling
  shm_fd = shm_open(MEMREGION, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if (shm_fd == -1) {
    fprintf(stderr, "%s Error @ shm_open ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (ftruncate(shm_fd, sizeof(shmRegion_t)) == -1) {
    fprintf(stderr, "%s Error @ ftruncate ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  shm_struct = mmap(NULL, sizeof(shmRegion_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (shm_struct == MAP_FAILED) {
    fprintf(stderr, "%s Error @ mmap ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }

  //Unlinking semaphores in case for some unknown reason that the semaphores might still be linked from a previous execution
  sem_unlink(SEM_1_FREE);
  sem_unlink(SEM_2_USED);
  sem_unlink(SEM_3_CNT);
  sem_unlink(SEM_4_WRITE);

  //Declaring/Initializing/Setting up smeaphores for later use
  sem_t * sem_free = sem_open(SEM_1_FREE, O_CREAT | O_EXCL, 0660, RINGBUFFERSIZE);
  sem_used = sem_open(SEM_2_USED, O_CREAT | O_EXCL, 0660, 0);

  int sem_open_error_flag = 0;
  if(mySemOpen(sem_free, argv[0]) != 0)
  {
    sem_open_error_flag = 1;
  }
  if(mySemOpen(sem_used, argv[0]) != 0)
  {
    sem_open_error_flag = 1;
  }
  if(sem_open_error_flag != 0)
  {
    munmap(shm_struct, sizeof(shmRegion_t));
    close(shm_fd);
    shm_unlink(MEMREGION);
    exit(EXIT_FAILURE);
  }

  //Initializing shared memory struct so every value is defined
  int read_head = 0;
  shm_struct -> write_head = 0;
  shm_struct -> process_count = 0;
  shm_struct -> end = 0;
  shm_struct -> running = 1;
  for (int i = 0; i < RINGBUFFERSIZE; i++) {
    shm_struct -> result_length[i] = __INT_MAX__;
  }


  int minimum = __INT_MAX__;

  while (minimum > 0 && sig_atom_quit == 0 && shm_struct -> end == 0) { //Main loop for reading the ringbuffer
    if (mySemWait(sem_used, argv[0]) == -1) {
      exit_state = EXIT_FAILURE;
      break;
    }
    if (sig_atom_quit == 1) { //Break if Signal interrupt
      break;
    }
    //Updating the Ringbuffer's read_head position
    read_head += 1;  
    read_head %= RINGBUFFERSIZE;

    /*If a read solution is the smallest one yet:
    * 1: set minimum's value to the new solution's length
    * 2: Output that solution
    */ 
    if (shm_struct -> result_length[read_head] < minimum || minimum == 0) {
      minimum = shm_struct -> result_length[read_head];
      if (minimum == 0) {
        printf("The graph is acyclic!\n");
      } else {
        printf("Solution with %d edges:", minimum);
        for (int i = 0; i < minimum; i++) {
          printf(" %d-%d", shm_struct -> result_from[read_head][i], shm_struct -> result_to[read_head][i]);
        }
        printf("\n");
      }
    }
    if (mySemPost(sem_free, argv[0]) == -1) {
      exit_state = EXIT_FAILURE;
      break;
    }
  }

  shm_struct -> end = 1; //signaling to the generator processes to shut down

  //Making sure no generator is stuck in a semaphore for program termination
  for (int i = 0; i < shm_struct -> process_count; i++) {
    sem_post(sem_free);
  }

  //Cleaning up shared memory recources + error handling
  if (munmap(shm_struct, sizeof(shmRegion_t)) == -1) {
    fprintf(stderr, "%s Error @ munmap ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (close(shm_fd) != 0) {
    fprintf(stderr, "%s Error @ close(shm_fd) ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (shm_unlink(MEMREGION) == -1) {
    fprintf(stderr, "%s Error @ shm_unlink ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }

  //Closing and freeing Semaphores + Error handling
  if(mySemClose(sem_free, argv[0]) == -1)
  {
    exit_state = EXIT_FAILURE;
  }
  if(mySemClose(sem_used, argv[0]) == -1)
  {
    exit_state = EXIT_FAILURE;
  }

  //Unlinking semaphores + Error handling
  if (sem_unlink(SEM_1_FREE) != 0) {
    fprintf(stderr, "%s Error @ sem_unlink(SEM_1_FREE) ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (sem_unlink(SEM_2_USED) != 0) {
    fprintf(stderr, "%s Error @ sem_unlink(SEM_2_USED) ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }

  //Program termination
  return exit_state;
}

void handle_signal(int signal) {
  sig_atom_quit = 1;
  sem_post(sem_used);
}