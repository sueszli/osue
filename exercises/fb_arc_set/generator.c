#define _GNU_SOURCE
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

#pragma region "common.h"
#define errorUsage(msg)                                                        \
  do {                                                                         \
    fprintf(stderr,                                                            \
            "Wrong usage: %s\nSYNOPSIS:\n\tgenerator "                         \
            "EDGE1...\nEXAMPLE:\n\tgenerator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 " \
            "6-0\n",                                                           \
            msg);                                                              \
    exit(EXIT_FAILURE);                                                        \
  } while (0);

#define errorHandler(msg) \
  do {                    \
    perror(msg);          \
    exit(EXIT_FAILURE);   \
  } while (0);

typedef struct {
  char from;
  char to;
} Edge;

typedef struct {
  Edge* edges;
  size_t size;
} EdgeList;

static void printEdgeList(EdgeList edgeList) {
  printf("Edge list: ");
  for (size_t i = 0; i < edgeList.size; i++) {
    Edge e = edgeList.edges[i];
    printf("%c-%c  ", e.from, e.to);
  }
  printf("\n");
}
#pragma endregion "common.h"

static void parseEdgeList(EdgeList edgeList, char* arguments[],
                          size_t edgeListSize) {
  // side effect: initiates edgeList
  // invariant: edgeList.edges size == arguments size
  for (size_t i = 0; i < edgeListSize; i++) {
    if (strlen(arguments[i]) != 3) {
      errorUsage("argument can only have 3 characters");
    }
    edgeList.edges[i] = (Edge){.from = arguments[i][0], .to = arguments[i][2]};
  }

  // check for duplicate edges
  for (size_t i = 0; i < edgeListSize; i++) {
    for (size_t j = 0; j < edgeListSize; j++) {
      Edge iEdge = edgeList.edges[i];
      Edge jEdge = edgeList.edges[j];
      if ((i != j) && (iEdge.from == jEdge.from) && (iEdge.to == jEdge.to)) {
        errorUsage("no duplicate arguments allowed");
      }
    }
  }
}

static void parseNodeString(char* nodeString, EdgeList edgeList) {
  // side effect: initiates nodeString
  // invariant: nodeString size + 1 == edgeList size
  int node_i = 0;
  nodeString[node_i] = '\0';

  for (size_t i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;
    if (index(nodeString, from) == NULL) {
      nodeString[node_i++] = to;
    }
    if (index(nodeString, to) == NULL) {
      nodeString[node_i++] = to;
    }

    nodeString[node_i] = '\0';  // required by index()
  }
}

static void generateSolution(EdgeList* solution, EdgeList edgeList,
                             char* nodeString) {
  // side effect: initiates solution
  // invariant: solution size + 1 = solution size == edgeList size
  printEdgeList(edgeList);
  nodeString = strfry(nodeString);
  printf("Randomized nodes: %s\n", nodeString);

  size_t sEdges_i = 0;

  // add
  for (size_t i = 0; i < edgeList.size; i++) {
    char from = edgeList.edges[i].from;
    char to = edgeList.edges[i].to;
    ptrdiff_t posFrom = index(nodeString, from);
    ptrdiff_t posTo = index(nodeString, to);

    if (posFrom > posTo) {
      (*solution).edges[sEdges_i++] = (Edge){.from = from, .to = to};
    }
  }

  // update size
  (*solution).size = sEdges_i;
  (*solution).edges =
      realloc((*solution).edges, (*solution).size * sizeof(Edge));
  if ((*solution).edges == NULL) {
    errorHandler("realloc");
  }
}

int main(int argc, char* argv[]) {
  printf("\n\n");

  // parse edges
  const size_t numEdges = (size_t)argc - 1;
  EdgeList edgeList = {.edges = malloc(numEdges * sizeof(Edge)),
                       .size = numEdges};
  if (edgeList.edges == NULL) {
    errorHandler("malloc");
  }
  parseEdgeList(edgeList, argv + 1, numEdges);

  // parse nodes
  char* nodeString = malloc((numEdges + 1) * sizeof(char));
  if (nodeString == NULL) {
    errorHandler("malloc");
  }
  parseNodeString(nodeString, edgeList);

  // ---------------------------

  // generate solution
  EdgeList solution = {.edges = malloc(numEdges * sizeof(Edge)),
                       .size = numEdges};
  if (solution.edges == NULL) {
    errorHandler("malloc");
  }

  generateSolution(&solution, edgeList, nodeString);
  printEdgeList(solution);

  // send to server ...

  free(solution.edges);

  // ---------------------------

  free(edgeList.edges);
  free(nodeString);

  printf("\n\n");
  return EXIT_SUCCESS;
}
