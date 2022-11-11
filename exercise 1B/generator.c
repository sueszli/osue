#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void logEdgeList(EdgeList edgeList) {
  for (size_t i = 0; i < edgeList.numEdges; i++) {
    log("(%s-%s)\n", edgeList.fst[i].from, edgeList.fst[i].to);
  }
}

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

static void logNodeList(NodeList nodeList) {
  size_t len = 0;
  while (nodeList[len] != NULL) {
    len++;
  }

  char *msg = "Node list: ";
  uint8_t nodeNameSize = 128;  // assumption about max node name size
  char *out = malloc((len * nodeNameSize + strlen(msg) + 1) * sizeof(char *));
  if (out == NULL) {
    error("malloc failed");
  }
  strcpy(out, msg);
  for (size_t i = 0; i < len; i++) {
    strcat(out, nodeList[i]);
    strcat(out, " ");
  }

  log("%s\n", out);
  free(out);
}

static bool contains(NodeList nodeList, char *input) {
  size_t i = 0;
  while (nodeList[i] != NULL) {
    if (strcmp(nodeList[i], input) == 0) {
      return true;
    }
    i++;
  }
  return false;
}

static char **parseNodes(NodeList nodeList, EdgeList edgeList) {
  // @invariant nodeList is empty
  // @invariant nodeList can't contain duplicates
  char **top = nodeList;
  size_t size = 0;

  // add nodes from edges
  for (size_t i = 0; i < edgeList.numEdges; i++) {
    char *node1 = edgeList.fst[i].from;
    char *node2 = edgeList.fst[i].to;
    if (!contains(nodeList, node1)) {
      *top = node1;
      top++;
      size++;
    }
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

int main(int argc, char **argv) {
  // parse and validate args
  if (argc < 2) {
    argumentError("At least one argument required");
  }
  u_int64_t numEdges = argc - 1;
  EdgeList allEdges = {.numEdges = numEdges,
                       .fst = malloc(numEdges * sizeof(Edge))};
  if (allEdges.fst == NULL) {
    error("malloc failed");
  }
  parseEdges(allEdges, argc, argv);
  NodeList nodeList = calloc(2 * numEdges * sizeof(char *), sizeof(char *));
  if (nodeList == NULL) {
    error("calloc failed");
  }
  nodeList = parseNodes(nodeList, allEdges);
  logNodeList(nodeList);

  // solve problem
  // strfry to randomly swap characters in string (-> does it also work with
  // pointers?)

  free(allEdges.fst);
  free(nodeList);

  exit(EXIT_SUCCESS);
}
