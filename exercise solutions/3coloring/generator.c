#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (true)

#define usage(msg)                                                            \
  do {                                                                        \
    fprintf(stderr, "%s\n", msg);                                             \
    fprintf(                                                                  \
        stderr,                                                               \
        "SYNOPSIS\n\t./generator EDGE1...\nEXAMPLE\n\tgenerator 0-1 0-2 0-3 " \
        "1-2 1-3 2-3");                                                       \
    exit(EXIT_FAILURE);                                                       \
  } while (0)

#define MAX_NUM_EDGES (1024)  // we want to avoid using pointers in shm

enum Color { RED = 0, GREEN = 1, BLUE = 2 };

typedef struct Node {
  size_t name;
  enum Color color;
} Node;

typedef struct Edge {
  Node from;
  Node to;
} Edge;

typedef struct EdgeList {
  Edge edges[MAX_NUM_EDGES];
  size_t len;
} EdgeList;

static void printEdgeList(const char* name, EdgeList edgeList) {
  printf("%s: ", name);
  for (size_t i = 0; i < edgeList.len; i++) {
    printf("%ld-%ld ", edgeList.edges[i].from.name, edgeList.edges[i].to.name);
  }
  printf("\n");
}

static EdgeList parseInput(int argc, char* argv[]) {
  // validate
  if ((argc - 1) < 1) {
    usage("too few arguments");
  }
  if ((argc - 1) > MAX_NUM_EDGES) {
    usage("too many arguments");
  }

  for (int i = 1; i < argc; i++) {
    for (int j = 1; j < argc; j++) {
      if ((i != j) && (strcmp(argv[i], argv[j])) == 0) {
        usage("duplicate arguments");
      }
    }
  }

  for (int i = 1; i < argc; i++) {
    if (index(argv[i], '-') == NULL) {
      usage("no dash in argument");
    }
    if (index(argv[i], '-') != rindex(argv[i], '-')) {
      usage("more than one dash in argument");
    }
  }

  for (int i = 1; i < argc; i++) {
    char* str = strdup(argv[i]);
    if (str == NULL) {
      error("strdup");
    }

    char* saveptr;
    char* fst = strtok_r(str, "-", &saveptr);
    char* snd = strtok_r(NULL, "-", &saveptr);
    if ((fst == NULL) || (snd == NULL)) {
      usage("missing node name");
    }

    for (size_t j = 0; j < strlen(fst); j++) {
      if (!isdigit(fst[j])) {
        usage("non digit character in node name");
      }
    }
    for (size_t j = 0; j < strlen(snd); j++) {
      if (!isdigit(snd[j])) {
        usage("non digit character in node name");
      }
    }

    free(str);
  }

  // make struct
  EdgeList output;
  output.len = (size_t)argc - 1;

  for (int i = 1; i < argc; i++) {
    char* saveptr;
    char* fst = strtok_r(argv[i], "-", &saveptr);
    char* snd = strtok_r(NULL, "-", &saveptr);

    errno = 0;
    output.edges[i - 1].from.name = strtoul(fst, NULL, 10);
    output.edges[i - 1].to.name = strtoul(snd, NULL, 10);
    if (errno != 0) {
      error("strtoul");
    }
  }
  return output;
}

static void generateColoring(EdgeList edgeList) {
  // side-effect: adds color value to nodes in edgeList

  // get nodeList
  Node nodeArray[MAX_NUM_EDGES * 2];
  size_t counter = 0;

  for (size_t i = 0; i < edgeList.len; i++) {
    Node fst = edgeList.edges[i].from;
    bool foundFst = false;
    for (size_t i = 0; i < counter; i++) {
      if (nodeArray[i].name == fst.name) {
        foundFst = true;
      }
    }
    if (!foundFst) {
      nodeArray[counter++] = fst;
    }

    Node snd = edgeList.edges[i].to;
    bool foundSnd = false;
    for (size_t i = 0; i < counter; i++) {
      if (nodeArray[i].name == snd.name) {
        foundSnd = true;
      }
    }
    if (!foundSnd) {
      nodeArray[counter++] = snd;
    }
  }

  // color nodeList
  for (size_t i = 0; i < counter; i++) {
    nodeArray[i].color = rand() % 3;
  }

  // color edgeList
  for (size_t i = 0; i < edgeList.len; i++) {
    for (size_t j = 0; j < counter; j++) {
      if (edgeList.edges[i].from.name == nodeArray[j].name) {
        edgeList.edges[i].from.color = nodeArray[j].color;
        break;
      }
    }
    for (size_t j = 0; j < counter; j++) {
      if (edgeList.edges[i].to.name == nodeArray[j].name) {
        edgeList.edges[i].to.color = nodeArray[j].color;
        break;
      }
    }
  }

  for (size_t i = 0; i < edgeList.len; i++) {
    Edge e = edgeList.edges[i];
    printf("\t%ld [%d] - %ld [%d]\n", e.from.name, e.from.color, e.to.name,
           e.to.color);
  }
}

static EdgeList generateSolution(EdgeList edgeList) {
  generateColoring(edgeList);

  EdgeList output;
  size_t counter = 0;

  // add to output if not the same color

  output.len = counter;
  return output;
}

int main(int argc, char* argv[]) {
  EdgeList edgeList = parseInput(argc, argv);
  printEdgeList("input", edgeList);

  srand((unsigned int)time(NULL));

  EdgeList solution = generateSolution(edgeList);
  printEdgeList("solution", solution);

  exit(EXIT_SUCCESS);
}