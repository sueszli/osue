#include "common.h"

static EdgeList parseEdgeList(int argc, char* argv[]) {
  // read man -k string | grep string

  // use delimiter
  // getdelim()
  // getline()

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
      usage("argument must have exactly 3 characters");
    }
    output.edges[i - 1] = (Edge){.from = argv[i][0], .to = argv[i][2]};
  }

  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      Edge iEdge = output.edges[i];
      Edge jEdge = output.edges[j];
      if ((i != j) && (iEdge.from == jEdge.from) && (iEdge.to == jEdge.to)) {
        usage("duplicate arguments");
      }
    }
  }
  return output;
}

static char* parseNodeString(EdgeList edgeList) { return "nothing"; }

int main(int argc, char* argv[]) { return EXIT_SUCCESS; }
