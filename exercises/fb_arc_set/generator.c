#include "common.h"

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

static unsigned long int parseStrToLong(char* str) {
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
    unsigned long int fstLong = parseStrToLong(fst);
    unsigned long int sndLong = parseStrToLong(snd);
    output.edges[i - 1] = (Edge){.from = fstLong, .to = sndLong};
  }

  return output;
}

static unsigned long int* parseNodeList(EdgeList edgeList) {
  // post-condition: free returned pointer

  unsigned long int* output = malloc(edgeList.size * sizeof(*output));
  size_t counter = 0;

  unsigned long int* reallocOut = realloc(output, counter * sizeof(*output));
  if (reallocOut == NULL) {
    error("realloc");
  }

  return reallocOut;
}

int main(int argc, char* argv[]) {
  validateInput(argc, argv);
  EdgeList edgeList = parseEdgeList(argc, argv);
  unsigned long int* nodeList = parseNodeList(edgeList);

  // can i malloc a long list and then read its size like strlen()?

  return EXIT_SUCCESS;
}
