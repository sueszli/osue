#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// print macros ::
#ifdef DEBUG
#define log(fmt, ...) \
  fprintf(stderr, "[%s:%d] " fmt, __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define log(fmt, ...) /* NOP */
#endif

#define error(s)                                   \
  fprintf(stderr, "%s: %s\n", s, strerror(errno)); \
  exit(EXIT_FAILURE);

#define argumentError(s)                                                       \
  fprintf(stderr, "%s\n", s);                                                  \
  fprintf(stderr, "%s\n", "USAGE:");                                           \
  fprintf(stderr, "\t%s\n", "generator EDGE1...");                             \
  fprintf(stderr, "%s\n", "EXAMPLE:");                                         \
  fprintf(stderr, "\t%s\n", "# Each 'x-y' argument below is a directed edge"); \
  fprintf(stderr, "\t%s\n", "# from vertice 'x' to vertice 'y' in a graph.");  \
  fprintf(stderr, "\t%s\n", "generator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0");  \
  exit(EXIT_FAILURE);
// :: print macros

// types ::
#define MAX_NAME_SIZE (1024)  // max name size for an edge or node

typedef struct {
  char *from;
  char *to;
} Edge;

typedef struct {
  size_t numEdges;
  Edge *fst;
} EdgeList;

typedef char **NodeList;  // @invariant no duplicates
// :: types

// logging ::
static void logEdgeList(EdgeList edgeList) {
  char msg[] = "Edge list: ";
  size_t edgeSize = MAX_NAME_SIZE * 2 + 4;
  char *out =
      malloc((edgeList.numEdges * edgeSize + strlen(msg) + 1) * sizeof(char *));
  if (out == NULL) {
    error("malloc failed");
  }
  strcpy(out, msg);
  for (size_t i = 0; i < edgeList.numEdges; i++) {
    char *left = edgeList.fst[i].from;
    char *right = edgeList.fst[i].to;
    strcat(out, "(");
    strcat(out, left);
    strcat(out, "-");
    strcat(out, right);
    strcat(out, ")");
    strcat(out, " ");
  }
  strcat(out, "\0");

  log("%s\n", out);
  free(out);
}

static void logNodeList(NodeList nodeList) {
  size_t len = 0;
  while (nodeList[len] != NULL) {
    len++;
  }

  char msg[] = "Node list: ";
  char *out = malloc((len * MAX_NAME_SIZE + strlen(msg) + 1) * sizeof(char *));
  if (out == NULL) {
    error("malloc failed");
  }
  strcpy(out, msg);
  for (size_t i = 0; i < len; i++) {
    strcat(out, nodeList[i]);
    strcat(out, " ");
  }
  strcat(out, "\0");

  log("%s\n", out);
  free(out);
}
// :: logging

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
    edgeList.fst[i - 1].from = left;
    edgeList.fst[i - 1].to = right;
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

bool contradictsOrder(char *from, char *to, NodeList nodeList) {
  size_t fromIndex = 0;
  size_t toIndex = 0;
  for (size_t i = 0; nodeList[i] != NULL; i++) {
    if (strcmp(nodeList[i], from) == 0) {
      fromIndex = i;
    }
    if (strcmp(nodeList[i], to) == 0) {
      toIndex = i;
    }
  }
  return fromIndex > toIndex;
}

static void genSolution(EdgeList solution, EdgeList allEdges,
                        NodeList nodePermutation) {
  log("Generating solution:\n", NULL);
  log("\tAll edges:\n", NULL);

  for (size_t i = 0; i < allEdges.numEdges; i++) {
    Edge edge = allEdges.fst[i];
    if (!contradictsOrder(edge.from, edge.to, nodePermutation)) {
      solution.fst[i] = edge;
    }
  }
}

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
  logEdgeList(allEdges);

  // parse nodes
  NodeList nodePermutation =
      calloc(2 * numEdges * sizeof(char *), sizeof(char *));
  if (nodePermutation == NULL) {
    error("calloc failed");
  }
  nodePermutation = parseNodes(nodePermutation, allEdges);
  logNodeList(nodePermutation);

  // generate feedback arc sets
  uint8_t iterations = 1;
  while (iterations > 0) {
    shuffle(nodePermutation);

    EdgeList solution = {.numEdges = numEdges,
                         .fst = malloc(numEdges * sizeof(Edge))};
    if (solution.fst == NULL) {
      error("malloc failed");
    }
    // genSolution(solution, allEdges, nodePermutation);

    free(solution.fst);
    iterations--;
  }

  free(allEdges.fst);
  free(nodePermutation);

  exit(EXIT_SUCCESS);
}
