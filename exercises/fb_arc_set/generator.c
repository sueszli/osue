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

int main(int argc, char* argv[]) { return EXIT_SUCCESS; }
