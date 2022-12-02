#define _GNU_SOURCE
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#define errorUsage(msg)                                                        \
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
    errorHandler("malloc");
  }

  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) != 3) {
      errorUsage("argument can only have 3 characters");
    }
    output.edges[i - 1] = (Edge){.from = argv[i][0], .to = argv[i][2]};
  }

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

static EdgeList generateSolution(EdgeList edgeList, char* nodeString) {
  // side-effect: randomizes nodeString
  // post-condition: free returned EdgeList

  EdgeList output =
      (EdgeList){.edges = malloc((size_t)edgeList.size * sizeof(Edge)),
                 .size = edgeList.size};
  if (output.edges == NULL) {
    errorHandler("malloc");
  }

  // add if edge not in order in randomized nodeString
  nodeString = strfry(nodeString);
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

  int i = 100;
  while (i-- >= 0) {
    EdgeList solution = generateSolution(edgeList, nodeString);
    printEdgeList("Solution", solution);
    free(solution.edges);
  }

  free(edgeList.edges);
  free(nodeString);

  printf("\n\n");
  return EXIT_SUCCESS;
}
