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

// convenience macros ::
#define sizeOfMember(structName, member) sizeof(((structName *)0)->member)
// :: convenience macros

// graph types ::
typedef struct {
  char *from;
  char *to;
} Edge;

typedef struct {
  size_t numEdges;
  Edge *fst;
} EdgeList;

typedef char **NodeList;  // @invariant no duplicates
// :: graph types

void printEdgeList(EdgeList edgeList) {
  for (size_t i = 0; i < edgeList.numEdges; i++) {
    printf("(%s-%s)\n", edgeList.fst[i].from, edgeList.fst[i].to);
  }
}

void parseEdges(EdgeList edgeList, int argc, char **argv) {
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

void printNodeList(NodeList nodeList) {
  char **p = nodeList.fst;
  while (*p != NULL) {
    if (0 == strcmp(p *, input)) {
      return true;
    }
    p++;
  }
  return false;
}

bool contains(NodeList nodeList, char *input) {
  char **p = nodeList.fst;
  while (*p != NULL) {
    p++;
  }

  char *p = nodeList.fst;
  while (p != '\0') {
    if (0 == strcmp(p, input)) {
      return true;
    }
    p++;
  }
  return false;
}

void parseNodes(NodeList nodeList, EdgeList edgeList) {
  for (size_t i = 0; i < edgeList.numEdges; i++) {
    char *a = edgeList.fst[i].from;
    char *b = edgeList.fst[i].to;

    log("%s - %s\n", a, b);
  }

  // don't forget to realloc to remove unnecessary memory
}

int main(int argc, char **argv) {
  // parse and validate args
  if (argc < 2) {
    argumentError("At least one argument required");
  }
  EdgeList edgeList = {.numEdges = argc - 1,
                       .fst = malloc(sizeof(Edge) * (argc - 1))};
  parseEdges(edgeList, argc, argv);
  NodeList nodeList = malloc(sizeof(Edge) * (argc - 1) * 2);
  printNodeList(nodeList);
  // parseNodes(nodeList, edgeList);

  // solve problem

  // strfry to randomly swap characters in string (-> does it also work with
  // pointers?)

  free(edgeList.fst);
  // free(nodeList.fst);

  exit(EXIT_SUCCESS);
}
