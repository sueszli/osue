#include "common.h"

#pragma region "solving"
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

  nodeString = strfry(nodeString);

  EdgeList output;
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
  output.size = (counter - 1);
  return output;
}
#pragma endregion "solving"

int main(int argc, char* argv[]) {
  EdgeList edgeList = parseEdgeList(argc, argv);
  char* nodeString = parseNodeString(edgeList);

  int i = 100;
  while (i--) {
    EdgeList solution = generateSolution(edgeList, nodeString);
    printEdgeList(solution);
  }

  free(nodeString);
  return EXIT_SUCCESS;
}
