/**
 * @file algorithms.c
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *
 * @brief algorithms Module implementation
 * 
 * @details implements a version of the algorithm fisher_yates_shuffle
 * and an algorithm that generates feedback-arcs.
 * 
 **/

#include "algorithms.h"
#include "datatypes.h"

static int randomInRange(int low, int high);
static int * duplIntArr(int *arr, int size);

/**
 * implementation of the fisher_yates_shuffle algorithm
 * @brief permutation of vertice array
 * @details uses a version of the fisher_yates_shuffle algorithm to
 * generate a new randomly orderd array of vertices
 * @param vertices input array of vertices
 * @return returns a new random vertice array
 **/
int * fisher_yates_shuffle(int *vertices) {
    if (vertices == NULL) {
        return NULL;
    }
    int n = getSize(vertices);  // TODO getSize returnd bei input 1-2 3-4 n = 4
    int *shuffeld = duplIntArr(vertices, n+1);  // n+1 because we want to also copy the end (-1)
    for (int i = 0; i <= n-2; i++) {
        int j = randomInRange(i, n-1);  // n-1 because j = random integer such that i <= j < n
        int temp = shuffeld[i];
        shuffeld[i] = shuffeld[j];
        shuffeld[j] = temp;
    }
    shuffeld[n] = -1;

    return shuffeld;
}

/**
 * duplicate function
 * @brief duplicates an int array
 * @details duplicates an int array with given size
 * @param arr int array
 * @param size size of the array
 * @return duplicate of int array
 **/
static int * duplIntArr(int *arr, int size) {
    int *duplicate = malloc(sizeof(int) * size);
    int n = 0;
    while (n < size) {
        duplicate[n] = arr[n];
        n++;
    }
    return duplicate;
}

/**
 * random number generator with given range
 * @brief generates a random number
 * @details random number in range low<=number<=high
 * @param low low border
 * @param high high border
 * @return random number in range
 **/
static int randomInRange(int low, int high) {
    return (rand() % (high-low+1)) + low;
}

/**
 * feedback arc set algorithm
 * @brief feedback arc set algorithm
 * @details Select all edges (u, v) for which u > v in the ordering. These edges form a feedback arc set.
 * @param vertices vertices of the graph, randomized.
 * @param edges edges without duplicates.
 * @param programName name of the program
 * @return feedback arc set pointer when solution contains less or equal to size Edges, otherwise NULL
 **/
struct FeedbackArc * fas_algorithm(int *vertices, struct Edge *edges, char *programName) {
    int maxSize = MAX_EDGES;
    struct FeedbackArc *arc_fas = malloc(sizeof(struct FeedbackArc));
    if (arc_fas == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        return NULL;
    }

    int selected = 0;
    int n = 0;
    while (edges[n].start >= 0) {
        int u = edges[n].start;
        int v = edges[n].end;
        int uIndex = getIndex(vertices, u);
        if (uIndex < 0) {
            fprintf(stderr, "%s: fas_algorithm: vertice '%d' is not in the list: ", programName, u);
            showVertices(vertices);
            fprintf(stderr, "!\n");
            free(arc_fas);
            return NULL;
        }
        int vIndex = getIndex(vertices, v);
        if (vIndex < 0) {
            fprintf(stderr, "%s: fas_algorithm: vertice '%d' is not in the list: ", programName, u);
            showVertices(vertices);
            fprintf(stderr, "!\n");
            free(arc_fas);
            return NULL;
        }

        if (uIndex > vIndex) {
            struct Edge chosen = {.start = u, .end = v};
            arc_fas->feedback[selected] = chosen;
            selected++;
        }

        if (selected > maxSize) {
            free(arc_fas);
            return NULL;
        }
        n++;
    }
    struct Edge nullEdge = {.start=-1, .end=-1};
    arc_fas->feedback[selected] = nullEdge;

    return arc_fas;
}