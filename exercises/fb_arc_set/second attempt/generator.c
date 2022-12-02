#include "common.h"

#pragma region "solving"
static EdgeList parseEdgeList(int argc, char* argv[]) {
  // post-condition: free returned EdgeList

  const int size = argc - 1;
  EdgeList output =
      (EdgeList){.edges = malloc((size_t)size * sizeof(Edge)), .size = size};
  if (output.edges == NULL) {
    error("malloc");
  }

  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) != 3) {
      usage("argument can only have 3 characters");
    }
    output.edges[i - 1] = (Edge){.from = argv[i][0], .to = argv[i][2]};
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      Edge iEdge = output.edges[i];
      Edge jEdge = output.edges[j];
      if ((i != j) && (iEdge.from == jEdge.from) && (iEdge.to == jEdge.to)) {
        usage("no duplicate arguments allowed");
      }
    }
  }
  return output;
}

static char* parseNodeString(EdgeList edgeList) {
  // post-condition: free returned string

  char* output = malloc((size_t)(edgeList.size) * sizeof(char));
  if (output == NULL) {
    error("malloc");
  }

  int node_i = 0;
  output[node_i] = '\0';

  for (int i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;
    if (index(output, from) == NULL) {
      output[node_i++] = to;
    }
    if (index(output, to) == NULL) {
      output[node_i++] = to;
    }
    output[node_i] = '\0';  // required by index()
  }
  return output;
}

static EdgeList generateSolution(EdgeList edgeList, char* nodeString) {
  // side-effect: randomizes nodeString
  // post-condition: free returned EdgeList

  EdgeList output =
      (EdgeList){.edges = malloc((size_t)edgeList.size * sizeof(Edge)),
                 .size = edgeList.size};
  if (output.edges == NULL) {
    error("malloc");
  }

  nodeString = strfry(nodeString);
  int counter = 0;
  for (int i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;
    ptrdiff_t posFrom = (ptrdiff_t)index(nodeString, from);
    ptrdiff_t posTo = (ptrdiff_t)index(nodeString, to);

    // add if edge not in order
    if (posFrom > posTo) {
      output.edges[counter++] = (Edge){.from = from, .to = to};
    }
  }

  output.size = counter;
  output.edges = realloc(output.edges, (size_t)counter * sizeof(Edge));
  if (output.edges == NULL) {
    error("realloc");
  }
  return output;
}
#pragma endregion "solving"

static void writeSubmission(ShmStruct* shmp, EdgeList edgeList,
                            char* nodeString) {
  // side-effect: writes into shared memory (under mutual exclusion)
  // side-effect: may change state of static variable 'updatedCounter'

  static bool hasIncrementedCounter = false;  // for less required semaphores

  if ((sem_wait(&shmp->write_mutex) == -1) && (errno != EINTR)) {
    error("sem_wait");
  }

  if (!hasIncrementedCounter) {
    shmp->generator_counter++;
  }

  EdgeList solution = generateSolution(edgeList, nodeString);
  // printEdgeList(solution);
  if (solution.size <= MAX_SOLUTION_SIZE) {
    shmp->buf[shmp->write_index] = solution;
    memcpy(&shmp->buf[shmp->write_index], &solution, sizeof(solution));
    shmp->write_index = (shmp->write_index + 1) % BUF_SIZE;
  }
  free(solution.edges);

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
    if (errno == ENOENT) {
      fprintf(stderr, "Shm must be created by supervisor first\n");
    }
    error("shm_open");
  }
  ShmStruct* shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  while (!(shmp->terminate)) {
    if (sem_wait(&shmp->num_free) == -1) {
      if (errno != EINTR) {
        break;  // signal while waiting
      }
      error("sem_wait");
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

  free(edgeList.edges);
  free(nodeString);
  return EXIT_SUCCESS;
}
