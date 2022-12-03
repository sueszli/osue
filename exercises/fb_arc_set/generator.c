#include "common.h"

//#region "solving"
typedef struct {
  unsigned long* nodes;  // can be pointer because not written into shm
  int size;
} NodeList;

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

static void printNodeList(NodeList nodeList) {
  for (int i = 0; i < nodeList.size; i++) {
    printf("%ld ", nodeList.nodes[i]);
  }
  printf("\n");
}

static bool nodeListContains(NodeList nodeList, unsigned long elem) {
  for (int i = 0; i < nodeList.size; i++) {
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

  int counter = 0;
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
      realloc(output.nodes, (size_t)counter * sizeof(*output.nodes));
  if (reallocedOutput.nodes == NULL) {
    error("realloc");
  }
  return reallocedOutput;
}

static void shuffleNodeList(NodeList nodeList) {
  // side-effect: shuffles nodeList.nodes

  unsigned long* nodes = nodeList.nodes;
  int size = nodeList.size;
  for (int i = size - 1; i >= 1; i--) {
    int j = rand() % (i + 1);
    unsigned long t = nodes[j];
    nodes[j] = nodes[i];
    nodes[i] = t;
  }
}

static void generateSolution(EdgeList edgeList, NodeList nodeList) {
  printNodeList(nodeList);
  shuffleNodeList(nodeList);
  printNodeList(nodeList);
}
//#endregion "solving"

int main(int argc, char* argv[]) {
  srand((unsigned int)time(NULL));

  validateInput(argc, argv);
  EdgeList edgeList = parseEdgeList(argc, argv);
  NodeList nodeList = parseNodeList(edgeList);

  generateSolution(edgeList, nodeList);

  free(nodeList.nodes);
  return EXIT_SUCCESS;
}
