#define _GNU_SOURCE
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#pragma region "done"
static EdgeList parseEdgeList(int argc, char* argv[]) {
  // post-condition: free returned EdgeList

  const int size = argc - 1;
  EdgeList output =
      (EdgeList){.edges = malloc((size_t)size * sizeof(Edge)), .size = size};
  if (output.edges == NULL) {
    errorHandler("malloc");
  }

  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) != 3) {
      errorUsage("argument can only have 3 characters");
    }
    output.edges[i - 1] = (Edge){.from = argv[i][0], .to = argv[i][2]};
  }

  // check for duplicates
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      Edge iEdge = output.edges[i];
      Edge jEdge = output.edges[j];
      if ((i != j) && (iEdge.from == jEdge.from) && (iEdge.to == jEdge.to)) {
        errorUsage("no duplicate arguments allowed");
      }
    }
  }

  return output;
}

static char* parseNodeString(EdgeList edgeList) {
  // post-condition: free returned string

  char* output = malloc((size_t)(edgeList.size) * sizeof(char));
  if (output == NULL) {
    errorHandler("malloc");
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
#pragma endregion "done"

static EdgeList generateSolution(EdgeList edgeList, char* nodeString) {
  // side-effect: randomizes nodeString
  // post-condition: free returned EdgeList

  EdgeList output =
      (EdgeList){.edges = malloc((size_t)edgeList.size * sizeof(Edge)),
                 .size = edgeList.size};
  if (output.edges == NULL) {
    errorHandler("malloc");
  }

  nodeString = strfry(nodeString);
  printf("Randomized nodes: %s\n", nodeString);

  // add
  int counter = 0;
  for (int i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;
    ptrdiff_t posFrom = (ptrdiff_t)index(nodeString, from);
    ptrdiff_t posTo = (ptrdiff_t)index(nodeString, to);

    if (posFrom > posTo) {
      output.edges[counter++] = (Edge){.from = from, .to = to};
    }
  }

  // update size
  output.size = counter;
  output.edges = realloc(output.edges, (size_t)counter * sizeof(Edge));
  if (output.edges == NULL) {
    errorHandler("realloc");
  }

  return output;
}

int main(int argc, char* argv[]) {
  printf("\n\n");

  EdgeList edgeList = parseEdgeList(argc, argv);
  char* nodeString = parseNodeString(edgeList);
  printEdgeList("Input", edgeList);
  printf("Node string: %s\n", nodeString);

  // make sure generated solutions really make sense before proceeding...
  EdgeList solution = generateSolution(edgeList, nodeString);
  printEdgeList("Solution", solution);

  free(edgeList.edges);
  free(nodeString);
  free(solution.edges);

  printf("\n\n");
  return EXIT_SUCCESS;
}
