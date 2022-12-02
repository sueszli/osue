#ifndef COMMON
#define COMMON

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