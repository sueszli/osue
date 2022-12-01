#include <stdbool.h>
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
  int size;
} EdgeList;

static void printEdgeList(EdgeList edgeList) {
  for (int i = 0; i < edgeList.size; i++) {
    Edge e = *(edgeList.edges + i);
    printf("%c-%c  ", e.from, e.to);
  }
  printf("\n");
}

static EdgeList parseEdgeList(int argc, char* argv[]) {
  if (argc <= 1) {
    errorUsage("no arguments given");
  }

  Edge edges[argc - 1];
  for (int i = 1; i < argc; i++) {
    if (strlen(argv[i]) != 3) {
      errorUsage("an edge can only consist of 3 characters");
    }
    char* str = argv[i];
    edges[i - 1] = (Edge){.from = str[0], .to = str[2]};
  }

  return (EdgeList){.edges = edges, .size = argc - 1};
}

static char* parseNodeString(EdgeList edgeList) { return "NULL"; }

int main(int argc, char* argv[]) {
  printf("\n\n");

  EdgeList edgeList = parseEdgeList(argc, argv);
  // char* nodeString = parseNodeString(edgeList);

  printf("\n\n");
  return EXIT_SUCCESS;
}
