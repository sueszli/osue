#ifndef COMMON
#define COMMON

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

#define printEdgeList(msg, edgeList)          \
  {                                           \
    printf("%s: ", msg);                      \
    for (int i = 0; i < edgeList.size; i++) { \
      Edge e = edgeList.edges[i];             \
      printf("%c-%c ", e.from, e.to);         \
    }                                         \
    printf("\n");                             \
  }

typedef struct {
  char from;
  char to;
} Edge;

typedef struct {
  Edge* edges;
  int size;
} EdgeList;

#endif