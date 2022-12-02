#include "common.h"

#define usage(msg)                                                             \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "Wrong usage: %s\nSYNOPSIS:\n\tgenerator "                         \
            "EDGE1...\nEXAMPLE:\n\tgenerator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 " \
            "6-0\n",                                                           \
            msg);                                                              \
    exit(EXIT_FAILURE);                                                        \
  } while (0);

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

static void writeSubmission(ShmStruct* shmp, EdgeList edgeList,
                            char* nodeString) {
  // side-effect: writes into shared memory (under mutual exclusion)

  if (sem_wait(&shmp->write_mutex) == -1) {
    error("sem_wait");
  }

  EdgeList solution = generateSolution(edgeList, nodeString);
  shmp->buf[shmp->read_index] = solution;
  shmp->read_index = (shmp->read_index + 1) % BUF_SIZE;
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
    error("shm_open");
  }
  ShmStruct* shmp =
      mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    error("mmap");
  }

  while (!(shmp->terminate)) {
    if (sem_wait(&shmp->num_free) == -1) {
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
