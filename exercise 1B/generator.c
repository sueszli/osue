#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

// parsing ::
static void parseEdges(EdgeList edgeList, int argc, char **argv) {
  for (size_t i = 1; i < argc; i++) {
    char *argument = argv[i];

    // validate
    size_t numDashes = 0;
    for (size_t j = 0; j < strlen(argument); j++) {
      if (argument[j] == '-') {
        numDashes++;
      }
    }
    if (numDashes != 1) {
      argumentError("Each argument must have exactly one '-' character");
    }

    // parse
    char *left = strtok(argument, "-");
    char *right = strtok(NULL, "-");
    edgeList.fst[i - 1] = (Edge){.from = left, .to = right};
  }
}

static bool contains(NodeList nodeList, char *input) {
  for (size_t i = 0; nodeList[i] != NULL; i++) {
    if (strcmp(nodeList[i], input) == 0) {
      return true;
    }
  }
  return false;
}

static char **parseNodes(NodeList nodeList, EdgeList edgeList) {
  char **top = nodeList;
  size_t size = 0;

  // add nodes from edges
  for (size_t i = 0; i < edgeList.numEdges; i++) {
    char *node1 = edgeList.fst[i].from;
    if (!contains(nodeList, node1)) {
      *top = node1;
      top++;
      size++;
    }
    char *node2 = edgeList.fst[i].to;
    if (!contains(nodeList, node2)) {
      *top = node2;
      top++;
      size++;
    }
  }

  // realloc
  char **newNodeList = realloc(nodeList, (size + 1) * sizeof(char *));
  if (newNodeList == NULL) {
    error("realloc failed");
  }
  newNodeList[size] = NULL;
  return newNodeList;
}
// :: parsing

// solving ::
static void shuffle(NodeList list) {
  size_t len = 0;
  while (list[len] != NULL) {
    len++;
  }

  srand(time(NULL));  // seed
  for (size_t i = 0; i < len; i++) {
    size_t j = i + rand() / (RAND_MAX / (len - i) + 1);
    // swap
    char *tmp = list[j];
    list[j] = list[i];
    list[i] = tmp;
  }
}

bool contradictsOrder(Edge edge, NodeList nodeList) {
  size_t fromIndex = 0;
  size_t toIndex = 0;
  for (size_t i = 0; nodeList[i] != NULL; i++) {
    if (strcmp(nodeList[i], edge.from) == 0) {
      fromIndex = i;
    }
    if (strcmp(nodeList[i], edge.to) == 0) {
      toIndex = i;
    }
  }
  return fromIndex > toIndex;
}

static EdgeList genSolution(EdgeList allEdges, NodeList nodePermutation) {
  // malloc
  EdgeList solution = {.numEdges = allEdges.numEdges,
                       .fst = malloc(allEdges.numEdges * sizeof(Edge))};
  if (solution.fst == NULL) {
    error("malloc failed");
  }

  // shuffle
  shuffle(nodePermutation);

  // get contradicting edges
  size_t solutionCounter = 0;
  for (size_t i = 0; i < allEdges.numEdges; i++) {
    Edge e = allEdges.fst[i];
    if (contradictsOrder(e, nodePermutation)) {
      solution.fst[solutionCounter] = e;
      solutionCounter++;
    }
  }
  solution.numEdges = solutionCounter;
  solution.fst[solutionCounter] = (Edge){.from = NULL, .to = NULL};

  // realloc
  EdgeList rSolution = {
      .numEdges = solution.numEdges,
      .fst = realloc(solution.fst, solution.numEdges * sizeof(Edge) + 1)};
  if (rSolution.fst == NULL) {
    error("realloc failed");
  }
  return rSolution;
}
// :: solving

int main(int argc, char **argv) {
  if (argc < 2) {
    argumentError("At least one argument required");
  }

  // parse edges
  const int numEdges = argc - 1;
  EdgeList allEdges = {.numEdges = numEdges,
                       .fst = malloc(numEdges * sizeof(Edge))};
  if (allEdges.fst == NULL) {
    error("malloc failed");
  }
  parseEdges(allEdges, argc, argv);

  // parse nodes
  NodeList allNodes = calloc(2 * numEdges * sizeof(char *), sizeof(char *));
  if (allNodes == NULL) {
    error("calloc failed");
  }
  allNodes = parseNodes(allNodes, allEdges);

  logEdgeList("All edges", allEdges);
  log("Num of all edges: %zu\n", allEdges.numEdges);
  logNodeList("All nodes", allNodes);

  // solve
  size_t iterations = 100;
  while (iterations > 0) {
    // log("\n\n// Iteration %zu\n", iterations);
    EdgeList solution = genSolution(allEdges, allNodes);  // malloc

    if (solution.numEdges < MAX_SOLUTION_SIZE) {
      log("Solution is right: %zu\n", solution.numEdges);
      logEdgeList("Solution", solution);
      log("\n");
    }

    free(solution.fst);
    iterations--;
  }

  free(allEdges.fst);
  free(allNodes);

  exit(EXIT_SUCCESS);
}
