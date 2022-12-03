#include "common.h"

//#region "solving"
static EdgeList parseEdgeList(int argc, char* argv[]) {
  const int size = argc - 1;
  if (size == 0) {
    usage("no arguments");
  }
  if (size > MAX_EDGELIST_SIZE) {
    usage("too many arguments");
  }

  EdgeList output;
  output.size = size;

  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) != 3) {
      usage("argument does not have exactly 3 characters");
    }
    if ((argv[i][0] == '-') || (argv[i][1] != '-') || (argv[i][2] == '-')) {
      usage("argument syntax is invalid");
    }
    output.edges[i - 1] = (Edge){.from = argv[i][0], .to = argv[i][2]};
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      Edge iEdge = output.edges[i];
      Edge jEdge = output.edges[j];
      if ((i != j) && (iEdge.from == jEdge.from) && (iEdge.to == jEdge.to)) {
        usage("duplicate argument");
      }
    }
  }
  return output;
}

static char* parseNodeString(EdgeList edgeList) {
  // post-condition: free returned string

  char* output = malloc((size_t)((2 * edgeList.size) + 1) * sizeof(char));
  if (output == NULL) {
    error("malloc");
  }

  int counter = 0;
  output[counter] = '\0';

  for (int i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;
    if (index(output, from) == NULL) {
      output[counter++] = to;
    }
    if (index(output, to) == NULL) {
      output[counter++] = to;
    }
    output[counter] = '\0';  // required by index()
  }

  char* reallocedOutput = realloc(output, (size_t)(counter + 1) * sizeof(char));
  if (reallocedOutput == NULL) {
    error("realloc");
  }
  return reallocedOutput;
}

static EdgeList generateSolution(EdgeList edgeList, char* nodeString) {
  // side-effect: randomizes nodeString

  nodeString = strfry(nodeString);

  EdgeList output;
  int counter = 0;
  for (int i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;

    // add if edge not in order
    if (index(nodeString, from) > index(nodeString, to)) {
      output.edges[counter++] = (Edge){.from = from, .to = to};
    }
  }
  output.size = counter;
  return output;
}
//#endregion "solving"

static void writeSubmission(ShmStruct* shmp, EdgeList edgeList,
                            char* nodeString) {
  // side-effect: writes into shared memory (under mutual exclusion)
  // side-effect: may change state of static variable 'hasIncrementedCounter'

  if ((sem_wait(&shmp->write_mutex) == -1) && (errno != EINTR)) {
    error("sem_wait");
  }

  // increment generator counter with atomic fetch-and-add (only once)
  static bool hasIncrementedCounter = false;
  if (!hasIncrementedCounter) {
    shmp->generator_counter++;
    hasIncrementedCounter = true;
  }

  // write into shared memory
  EdgeList solution = generateSolution(edgeList, nodeString);
  if (solution.size < MAX_SOLUTION_SIZE) {
    shmp->buf[shmp->write_index] = solution;
    shmp->write_index = (shmp->write_index + 1) % BUF_SIZE;
  }

  if (sem_post(&shmp->write_mutex) == -1) {
    error("sem_post");
  }
}

int main(int argc, char* argv[]) {
  EdgeList edgeList = parseEdgeList(argc, argv);
  char* nodeString = parseNodeString(edgeList);

  // open shared memory
  int fd = shm_open(SHM_PATH, O_RDWR, 0);
  if (fd == -1) {
    error("shm_open");
  }
  ShmStruct* shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  // write into shared memory
  while (!(shmp->terminate)) {
    if ((sem_wait(&shmp->num_free) == -1) && (errno != EINTR)) {
      error("sem_wait");
    }
    if (shmp->terminate) {
      // if supervisor called sem_post() to free generators for shutdown
      break;
    }

    writeSubmission(shmp, edgeList, nodeString);

    if (sem_post(&shmp->num_used) == -1) {
      error("sem_post");
    }
  }

  // close shared memory
  if (munmap(shmp, sizeof(*shmp)) == -1) {
    error("munmap");
  }
  if (close(fd) == -1) {
    error("close");
  }

  free(nodeString);
  return EXIT_SUCCESS;
}
