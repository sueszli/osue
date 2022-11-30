#ifndef COMMON
#define COMMON

#define errorHandler(msg) \
  do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct {
  char from;
  char to;
} Edge;

typedef Edge EdgeList[];

typedef char NodeList[];

struct shmbuf {
  EdgeList solution;
};

#endif