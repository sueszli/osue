#include "myutil.h"

#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <regex.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/**
 * @file generator.c
 * @author Bernhard Jung: 12023965
 * @brief  My Implementtation of Exercise 1B: The Generator process for writing results into the shared memory ringbuffer
 * @details At frist the generator links up with the shared memory and semaphores provided by the supervisor.
 *          If that was successful the input arguments are being parsed into generatorGraph_t.edgeFrom/edgeTo.
 *          The resulting graph is then being analyced if it includes loops via the Monte-Carlo-Algorithm. 
 *          The algorithm's result are then being written into the shared memory ringbuffer and read by the supervisor.
 * @date 2021.11.14
 */

/**
 * @brief Searches threw an int[] if a needle (int) is found within a haystack (int[])
 * @param graph_struct Graph-struct which includes an int[] which is being searched
 * @param needle the value who's index is being searched.
 * @return returns the index the needle was found; -1 if not found
 */
static int getIndex(generatorGraph_t graph_struct, int needle);

/**
 * @brief Calculates the number ob vertices within the given graph
 * @param graph_struct Graph-struct which is being analyced
 * @return Number of vertices
 */
static int getNumVertices(generatorGraph_t graph_struct);

/**
 * @brief Initializes the int[] vertices within the generatorGraph_t struct
 * @param graph_struct Graph-struct who's int[] vertices is being initialized
 * @return void
 */
static void initialzeGraphVertices(generatorGraph_t graph_struct);

/**
 * @brief Frees arrays withing the Graph-Struct
 * @param graph_strcut Graph-struct who's arrays are being freed
 * @param mode decides of only the arrays edge_from[] & edge_to[] (!= 1) should be freed or vertices[] should also get freed (==1)
 * @return void
 */
static void myFree(generatorGraph_t graph_strcut, int mode);

/**
 * @brief Helper function for shuffeling arrays with qSort
 * @param a void Pointer 1 
 * @param b boid Pointer 2
 * @return Returns randomly -1 or 1
 */
static int myRand(const void * a,
  const void * b);

int main(int argc, char ** argv) {

  // Decleration later needed variables and structs
  int shm_fd;
  int count = 0;
  char * token = NULL;
  char key[2] = "-";
  int exit_state = EXIT_SUCCESS;

  shmRegion_t * shm_struct;
  generatorGraph_t graph_struct;
  regex_t regex_pattern;

  //Setting up the shared memory on clientside + error handeling
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

  //Exit with failure condition if the supervisor is not active
  if (shm_struct -> running != 1) {
    fprintf(stderr, "%s Supervisor not active ! \n", argv[0]);
    munmap(shm_struct, sizeof(shmRegion_t));
    close(shm_fd);
    exit(EXIT_FAILURE);
  }

  //Declaring/Initializing/Setting up smeaphores for later use
  sem_t * sem_free = sem_open(SEM_1_FREE, O_CREAT, 0660, RINGBUFFERSIZE);
  sem_t * sem_used = sem_open(SEM_2_USED, O_CREAT, 0660, 0);
  sem_t * sem_thread_count = sem_open(SEM_3_CNT, O_CREAT, 0660, 1);
  sem_t * sem_write = sem_open(SEM_4_WRITE, O_CREAT, 0660, 1);

  //Opening Semaphores + Error handling
  int sem_open_error_flag = 0;
  if (mySemOpen(sem_free, argv[0]) != 0) {
    sem_open_error_flag = 1;
  }
  if (mySemOpen(sem_used, argv[0]) != 0) {
    sem_open_error_flag = 1;
  }
  if (mySemOpen(sem_thread_count, argv[0]) != 0) {
    sem_open_error_flag = 1;
  }
  if (mySemOpen(sem_write, argv[0]) != 0) {
    sem_open_error_flag = 1;
  }
  if (sem_open_error_flag != 0) {
    munmap(shm_struct, sizeof(shmRegion_t));
    close(shm_fd);
    exit(EXIT_FAILURE);
  }

  //counting how many generator Processes are running
  sem_wait(sem_thread_count);
  shm_struct -> process_count += 1;
  srand(shm_struct -> process_count + (int) time(NULL));
  sem_post(sem_thread_count);

  //Initializing RegEx Pattern + Error handling
  if (regcomp( & regex_pattern, "[0-9]+-[0-9]+", REG_EXTENDED) != 0) {
    fprintf(stderr, "%s regcomp(regex_pattern) ! Errno: %s\n", argv[0], strerror(errno));
    regfree( & regex_pattern);
    mySemClose(sem_free, argv[0]);
    mySemClose(sem_used, argv[0]);
    mySemClose(sem_thread_count, argv[0]);
    mySemClose(sem_write, argv[0]);
    munmap(shm_struct, sizeof(shmRegion_t));
    close(shm_fd);
    exit(EXIT_FAILURE);
  }

  //Allocation for dynamic number of input Edges
  graph_struct.edge_from = (int * ) malloc(argc * sizeof(int));
  graph_struct.edge_to = (int * ) malloc(argc * sizeof(int));
  graph_struct.num_edges = argc - 1;

  //Parsing Input parameters + Error handling
  if (argc <= 1) { //No parameters given
    fprintf(stderr, "%s Missing arguments !\n", argv[0]);
    myFree(graph_struct, 0);
    regfree( & regex_pattern);
    mySemClose(sem_free, argv[0]);
    mySemClose(sem_used, argv[0]);
    mySemClose(sem_thread_count, argv[0]);
    mySemClose(sem_write, argv[0]);
    munmap(shm_struct, sizeof(shmRegion_t));
    close(shm_fd);
    exit(EXIT_FAILURE);
  } else { //Parsing parameters
    for (int i = 1; i < argc; i++) {
      if (regexec( & regex_pattern, argv[i], 0, NULL, 0) == 0) {
        token = strtok(argv[i], key);
        char * end_ptr = NULL;
        graph_struct.edge_from[i - 1] = strtol(token, & end_ptr, 10);
        token = strtok(NULL, key);
        graph_struct.edge_to[i - 1] = strtol(token, & end_ptr, 10);
      } else {
        fprintf(stderr, "%s Illegal arguments !\n", argv[0]);
        myFree(graph_struct, 0);
        regfree( & regex_pattern);
        mySemClose(sem_free, argv[0]);
        mySemClose(sem_used, argv[0]);
        mySemClose(sem_thread_count, argv[0]);
        mySemClose(sem_write, argv[0]);
        munmap(shm_struct, sizeof(shmRegion_t));
        close(shm_fd);
        exit(EXIT_FAILURE);
      }
    }
  }

  //Allocation for dynamic number of input vertices
  graph_struct.num_vertices = getNumVertices(graph_struct);
  graph_struct.vertices = (int * ) malloc(graph_struct.num_vertices * sizeof(int));

  //Allocation and initialization for dynamic number of matching vertices
  int * pos = NULL;
  pos = (int * ) malloc(graph_struct.num_edges * sizeof(int));
  initialzeGraphVertices(graph_struct);

  while (shm_struct -> end == 0) { //Main loop until the generator receives a message to stop; writing found results to shared memory ringbuffer
    qsort(graph_struct.vertices, graph_struct.num_vertices, sizeof(int), myRand); //randomizing vertices

    //Implementtation + Application of the Monte-Carlo-Algorith
    count = 0;
    for (int i = 0; i < graph_struct.num_edges; i++) {
      if (getIndex(graph_struct, graph_struct.edge_from[i]) > getIndex(graph_struct, graph_struct.edge_to[i])) {
        pos[count] = i;
        count += 1;
      }
    }

    if (mySemWait(sem_free, argv[0]) == -1) {
      shm_struct -> end = 1;
      exit_state = EXIT_FAILURE;
      break;
    }
    if (shm_struct -> end == 0) {
      if (mySemWait(sem_write, argv[0]) == -1) {
        shm_struct -> end = 1;
        exit_state = EXIT_FAILURE;
        break;
      }

      //Updating the Ringbuffer's write_head position
      shm_struct -> write_head += 1;
      shm_struct -> write_head %= RINGBUFFERSIZE;

      //Writing found result to the shared memory ringbuffer
      shm_struct -> result_length[shm_struct -> write_head] = count;
      for (int i = 0; i < shm_struct -> result_length[shm_struct -> write_head]; i++) {
        shm_struct -> result_from[shm_struct -> write_head][i] = graph_struct.edge_from[pos[i]];
        shm_struct -> result_to[shm_struct -> write_head][i] = graph_struct.edge_to[pos[i]];
      }
      if (mySemPost(sem_write, argv[0]) == -1) {
        shm_struct -> end = 1;
        exit_state = EXIT_FAILURE;
        break;
      }
    }

    if (mySemPost(sem_used, argv[0]) == -1) {
      shm_struct -> end = 1;
      exit_state = EXIT_FAILURE;
      break;
    }
  }

  //Freeing allocated recources and closing semaphores + Error handling
  free(pos);
  myFree(graph_struct, 1);
  regfree( & regex_pattern);

  if (mySemClose(sem_free, argv[0]) == -1) {
    exit_state = EXIT_FAILURE;
  }
  if (mySemClose(sem_used, argv[0]) == -1) {
    exit_state = EXIT_FAILURE;
  }
  if (mySemClose(sem_thread_count, argv[0]) == -1) {
    exit_state = EXIT_FAILURE;
  }
  if (mySemClose(sem_write, argv[0]) == -1) {
    exit_state = EXIT_FAILURE;
  }

  //Unlinking semaphores + Error handling
  if (munmap(shm_struct, sizeof(shmRegion_t)) == -1) {
    fprintf(stderr, "%s Error @ munmap ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (close(shm_fd) != 0) {
    fprintf(stderr, "%s Error @ close(shm_fd) ! Errno: %s\n", argv[0], strerror(errno));
    exit(EXIT_FAILURE);
  }
  //Program termination
  return exit_state;
}

int getIndex(generatorGraph_t graph_struct, int needle) {
  for (int i = 0; i < graph_struct.num_vertices; i++) {
    if (needle == graph_struct.vertices[i]) {
      return i;
    }
  }
  return -1;
}

int getNumVertices(generatorGraph_t graph_struct) {
  int maximum = 0;
  for (int i = 0; i < graph_struct.num_edges; i++) {
    if (graph_struct.edge_from[i] > maximum) {
      maximum = graph_struct.edge_from[i];
    }
    if (graph_struct.edge_to[i] > maximum) {
      maximum = graph_struct.edge_to[i];
    }
  }
  maximum += 1;
  return maximum;
}

void initialzeGraphVertices(generatorGraph_t graph_struct) {
  for (int i = 0; i < graph_struct.num_vertices; i++) {
    graph_struct.vertices[i] = i;
  }
}

int myRand(const void * a,
  const void * b) {
  (void) a;
  (void) b;
  if ((rand() % 2) == 1) {
    return 1;
  } else {
    return -1;
  }
}

void myFree(generatorGraph_t graph_struct, int mode) {
  if (mode == 1) {
    free(graph_struct.vertices);
  }
  free(graph_struct.edge_from);
  free(graph_struct.edge_to);
}