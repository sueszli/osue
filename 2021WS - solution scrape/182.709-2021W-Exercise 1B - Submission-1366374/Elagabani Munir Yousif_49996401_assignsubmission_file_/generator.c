/**
 * @file generator.c
 * @author Munir Yousif Elagabani <e12022518@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief program that generates random feedback arc solutions and writes the
 * solution to a circular buffer.
 **/

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "circularbuffer.h"

static char *pgm_name;

/** Mandatory usage function.
 *  @brief This function writes helpful usage information about the program to
 *stderr.
 *  @details global variables: pgm_name
 **/
static void usage() {
  fprintf(stderr, "Usage: %s EDGE1 ...\n", pgm_name);
  fprintf(stderr, "Example: %s 0-1 1-2 1-3 1-4 \n", pgm_name);
  exit(EXIT_FAILURE);
}

/**
 * @brief This function generates random permutations
 *
 * @param size size of the permutation array
 * @param permutation pre-allocated array where the permutation will be written
 * to
 */
static void generateRandomPermutation(size_t size, int *permutation) {
  int startIndex = 0;
  for (int i = 0; i < size; i++) {
    int randIndex = rand() % size;
    int tmp = permutation[startIndex];
    permutation[startIndex] = permutation[randIndex];
    permutation[randIndex] = tmp;
    startIndex++;
  }
}

/**
 * @brief This function calculates which edges need to be removed in order to
 * get an acyclic graph.
 *
 * @param numEdges size of edgeList
 * @param edgeList array of edge_t with all edges
 * @param numVertices number of implicitly given vertices
 * @param permutation permutation array that has the same size as numVertices
 * @return fbArc_t solution with the edges and correct number of edges that have
 * to be removed. If the number of edges exceed MAX_EDGES_FB_ARC the number of
 * edges is set to -1.
 */
static fbArc_t generateSolution(size_t numEdges, edge_t *edgeList,
                                size_t numVertices, int *permutation) {
  fbArc_t result;
  result.numEdges = 0;

  for (int i = 0; i < numEdges; i++) {
    edge_t current = edgeList[i];
    bool foundU = false;
    for (int j = 0; j < numVertices; j++) {
      if (result.numEdges == MAX_EDGES_FB_ARC) {
        result.numEdges = -1;
        return result;
      }

      int vertex = permutation[j];
      if (!foundU && vertex == current.to) {
        foundU = true;
      } else if (foundU && vertex == current.from) {
        result.selection[result.numEdges] = current;
        result.numEdges++;
        break;
      }
    }
  }

  return result;
}

/**
 * @brief Main entry point for the progam.
 *
 * @param argc the number of arguments
 * @param argv the array of arguments
 * @return Returns exit code EXIT_SUCCESS OR EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
  pgm_name = argv[0];

  // at least 2 egdes need to be provided
  if (argc < 2) {
    usage();
  }

  // -1 because the first argument is the program name
  size_t numEdges = (argc - 1);
  edge_t *edges = malloc(sizeof(edge_t) * numEdges);

  edge_t *currentEdge = edges;

  size_t maxVertexIndex = 0;

  for (int i = 1; i < argc; i++) {
    char *string = argv[i];

    // sscanf returns the number of correctly pattern matched variables and we
    // need 2 for v1 and v2
    if (sscanf(string, "%d-%d", &currentEdge->from, &currentEdge->to) == 2 &&
        currentEdge->from >= 0 && currentEdge->to >= 0) {
      printf("%d-%d\n", currentEdge->from, currentEdge->to);

      if (maxVertexIndex < currentEdge->from) {
        maxVertexIndex = currentEdge->from;
      }
      if (maxVertexIndex < currentEdge->to) {
        maxVertexIndex = currentEdge->to;
      }
    } else {
      fprintf(stderr, "[%s] Wrong input: edge #%d is malformed: \"%s\" \n",
              pgm_name, i - 1, string);
      usage();
    }
    currentEdge++;
  }
  printf("Number of edges: %zu\n", numEdges);

  printf("\n");
  load_buffer(false, pgm_name);

  srand(time(0));

  // the number of vertices is implicitly provided by the maximum index of
  // vertices
  size_t numVertices = maxVertexIndex + 1;
  int permutation[numVertices];
  for (int i = 0; i < numVertices; i++) {
    permutation[i] = i;
  }

  while (!shouldTerminate()) {
    generateRandomPermutation(numVertices, permutation);
    fbArc_t result =
        generateSolution(numEdges, edges, numVertices, permutation);

    // fbarc set is too big
    if (result.numEdges == -1) {
      continue;
    }

    // fbarc set is not better than the current best solution
    if (result.numEdges >= get_min_solution_size()) {
      continue;
    }
    write_buffer(result);
  }

  free(edges);
  cleanup_buffer();
  return EXIT_SUCCESS;
}