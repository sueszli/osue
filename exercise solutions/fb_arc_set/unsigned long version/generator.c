#include "common.h"

//#region "solving"
static void validateInput(int argc, char* argv[]) {
  if ((argc - 1) == 0) {
    usage("no arguments");
  }
  if ((argc - 1) > MAX_EDGELIST_SIZE) {
    usage("too many arguments");
  }
  for (int i = 1; i < argc; i++) {
    for (int j = 1; j < argc; j++) {
      if ((i != j) && (strcmp(argv[i], argv[j]) == 0)) {
        usage("duplicate argument");
      }
    }
  }

  for (int i = 1; i < argc; i++) {
    if (index(argv[i], '-') == NULL) {
      usage("missing '-' character in an argument");
    }
    if (index(argv[i], '-') != rindex(argv[i], '-')) {
      usage("more than one '-' character in an argument");
    }
  }

  char* saveptr;
  for (int i = 1; i < argc; i++) {
    char* copy = strdup(argv[i]);
    if (copy == NULL) {
      error("strdup");
    }
    char* fst = strtok_r(copy, "-", &saveptr);
    char* snd = strtok_r(NULL, "-", &saveptr);
    if ((fst == NULL) || (snd == NULL)) {
      usage("no node name in an argument");
    }
    for (size_t i = 0; i < strlen(fst); i++) {
      if (!isdigit(fst[i])) {
        usage("node name does not only consist of digits");
      }
    }
    for (size_t i = 0; i < strlen(snd); i++) {
      if (!isdigit(snd[i])) {
        usage("node name does not only consist of digits");
      }
    }
    free(copy);
  }
}

static unsigned long parseStrToLong(char* str) {
  errno = 0;  // make errors visible
  unsigned long val = strtoul(str, NULL, 10);
  if ((errno == ERANGE) || (val == ULONG_MAX)) {
    error("strtoul");
  }
  if ((errno != 0) && (errno != EINVAL)) {
    error("strtoul");
  }
  return val;
}

static EdgeList parseEdgeList(int argc, char* argv[]) {
  EdgeList output;
  output.size = argc - 1;

  char* saveptr;
  for (int i = 1; i < argc; i++) {
    char* fst = strtok_r(argv[i], "-", &saveptr);
    char* snd = strtok_r(NULL, "-", &saveptr);
    unsigned long fstLong = parseStrToLong(fst);
    unsigned long sndLong = parseStrToLong(snd);
    output.edges[i - 1] = (Edge){.from = fstLong, .to = sndLong};
  }
  return output;
}

typedef struct {
  unsigned long* nodes;  // can be pointer because not written into shm
  unsigned int size;     // must be able to store twice the amount of int
} NodeList;

static bool nodeListContains(NodeList nodeList, uint64_t elem) {
  for (unsigned int i = 0; i < nodeList.size; i++) {
    if (nodeList.nodes[i] == elem) {
      return true;
    }
  }
  return false;
}

static NodeList parseNodeList(EdgeList edgeList) {
  // post-condition: free returned list

  NodeList output;
  output.nodes = malloc((size_t)(2 * edgeList.size) * sizeof(*output.nodes));
  if (output.nodes == NULL) {
    error("malloc");
  }

  unsigned int counter = 0;
  for (int i = 0; i < edgeList.size; i++) {
    unsigned long from = edgeList.edges[i].from;
    unsigned long to = edgeList.edges[i].to;
    output.size = counter;
    if (!nodeListContains(output, from)) {
      output.nodes[counter++] = from;
    }
    if (!nodeListContains(output, to)) {
      output.nodes[counter++] = to;
    }
  }
  output.size = counter;

  NodeList reallocedOutput;
  reallocedOutput.size = counter;
  reallocedOutput.nodes =
      realloc(output.nodes, counter * sizeof(*output.nodes));
  if (reallocedOutput.nodes == NULL) {
    error("realloc");
  }
  return reallocedOutput;
}

static void shuffleNodeList(NodeList nodeList) {
  // side-effect: shuffles nodeList.nodes

  unsigned long* nodes = nodeList.nodes;
  unsigned int size = nodeList.size;
  for (unsigned int i = size - 1; i >= 1; i--) {
    unsigned long j = (unsigned int)(rand()) % (i + 1);
    unsigned long t = nodes[j];
    nodes[j] = nodes[i];
    nodes[i] = t;
  }
}

static unsigned int indexOf(NodeList nodeList, unsigned long elem) {
  for (unsigned int i = 0; i < nodeList.size; i++) {
    if (nodeList.nodes[i] == elem) {
      return i;
    }
  }
  error("called indexOf although elem was not in nodeList");
}

static EdgeList generateSolution(EdgeList edgeList, NodeList nodeList) {
  shuffleNodeList(nodeList);

  EdgeList output;
  int counter = 0;
  for (int i = 0; i < edgeList.size; i++) {
    unsigned long from = edgeList.edges[i].from;
    unsigned long to = edgeList.edges[i].to;
    if (indexOf(nodeList, from) > indexOf(nodeList, to)) {
      output.edges[counter] = edgeList.edges[i];
      counter++;
    }
  }
  output.size = counter;
  return output;
}
//#endregion "solving"

static void writeSubmission(ShmStruct* shmp, EdgeList edgeList,
                            NodeList nodeList) {
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
  EdgeList solution = generateSolution(edgeList, nodeList);
  if (solution.size < MAX_SOLUTION_SIZE) {
    shmp->buf[shmp->write_index] = solution;
    shmp->write_index = (shmp->write_index + 1) % BUF_SIZE;
  }

  if (sem_post(&shmp->write_mutex) == -1) {
    error("sem_post");
  }
}

int main(int argc, char* argv[]) {
  srand((unsigned int)time(NULL));

  validateInput(argc, argv);
  EdgeList edgeList = parseEdgeList(argc, argv);
  NodeList nodeList = parseNodeList(edgeList);

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

    writeSubmission(shmp, edgeList, nodeList);

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

  free(nodeList.nodes);
  return EXIT_SUCCESS;
}
