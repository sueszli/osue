/**
 * @file datatypes.c
 * @author Lukas Fink 11911069
 * @date 10.11.2021
 *  
 * @brief datatypes Module implementation
 *
 * implements methods required for using the datatypes of datatypes.h
 **/
 
 
#include "datatypes.h"

/**
 * construct an Edge
 * @brief construct a Edge from a String
 * @details the String has to be of the form [0..9]+[-][0..9]+. 
 * E.g.: 1-2 or 2345-5421 for the function to succeed
 * @param arr input String
 * @param programName name of the program calling the function
 * @return returns a new random vertice array if the String has the
 * required structure and contains no self-loops, otherwise return NULL
 **/
struct Edge * constructEdge(char *arr, char *programName) {
    if (arr == NULL || (sizeof(arr)/sizeof(char)) < 3) {
        return NULL;
    }

// read first didgit
    int sizeS = 8;
    char *startArr = malloc(sizeof(char) * sizeS);
    if (startArr == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        return NULL;
    }

    int nIn = 0;
    int nS = 0;
    while (arr[nIn] != '-') {
        if ((arr[nIn] == '\0') || (!isdigit(arr[nIn]))) {
            return NULL;
        }
        if (nS >= sizeS-1) {
            sizeS += 16;
            startArr = realloc(startArr, sizeof(char) * sizeS);
            if (startArr == NULL) {
                fprintf(stderr, "%s: out of memory!\n", programName);
                free(startArr);
                return NULL;
            }
        }
        startArr[nS] = arr[nIn];
        nIn++;
        nS++;
    }
    startArr[nS] = '\0';
    // jump to the next char after the seperator
    nIn++;

// read second didgit

    int sizeE = 8;
    char *endArr = malloc(sizeof(char) * sizeE);
    if (endArr == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        free(startArr);
        return NULL;
    }

    int nE = 0;
    while (arr[nIn] != '\0') {
        if (!isdigit(arr[nIn])) {
            return NULL;
        }
        if (nE >= sizeE-1) {
            sizeE += 16;
            endArr = realloc(endArr, sizeof(char) * sizeE);
            if (endArr == NULL) {
                fprintf(stderr, "%s: out of memory!\n", programName);
                free(startArr);
                free(endArr);
                return NULL;
            }
        }
        endArr[nE] = arr[nIn];
        nIn++;
        nE++;
    }
    endArr[nE] = '\0';

// convert input
    int start = 0;
    int rS = sscanf(startArr, "%d", &start);
    if (rS == EOF) {
        fprintf(stderr, "%s: sscanf: input failure on: %s\n", programName, startArr);
        free(startArr);
        free(endArr);
        return NULL;
    }
    int end = 0;
    int rE = sscanf(endArr, "%d", &end);
    if (rE == EOF) {
        fprintf(stderr, "%s: sscanf: input failure on: %s\n", programName, startArr);
        free(startArr);
        free(endArr);
        return NULL;
    }

    // no edges from and to the same vertice!
    if (start == end) {
        return NULL;
    }

    struct Edge transformed = {.start=start, .end=end};
    struct Edge *transP = &transformed;

    // free Memory
    free(startArr);
    free(endArr);

    return transP;
}

/**
 * print Edges
 * @brief print out an Edge array
 * @details prints Edges until an Edge with negaive value
 * @param edgeArr array of edges to be printed
 **/
void showEdges (struct Edge *edgeArr) {
    if (edgeArr == NULL) {
        return;
    }
    int n = 0;
    while (edgeArr[n].start >= 0) {
        fprintf(stdout, "%d-%d ", edgeArr[n].start, edgeArr[n].end);
        n++;
    }
}

/**
 * remove duplicate Edges function
 * @brief removes duplicate Edges
 * @details generates a new Edge array containing no duplicates
 * @param edgeArr input Edge Array to remove duplicates from
 * @param programName name of the program calling the function
 * @param eSize size of the input Edge Arrayy
 * @return returns a new Edge array without duplicates when succeeding, otherwise NULL
 **/
struct Edge * removeDuplicateEdges(struct Edge *edgeArr, char *programName, int eSize) {
    if (edgeArr == NULL) {
        return NULL;
    }

    struct Edge *withoutDupl = malloc(sizeof(struct Edge) * eSize);
    if (withoutDupl == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        return NULL;
    }
    int nW = 0;
    int n = 0;
    struct Edge nullEdge = {.start=-1, .end=-1};
    withoutDupl[nW] = nullEdge;
    while (edgeArr[n].start >= 0) {
        int localStart = edgeArr[n].start;
        int localEnd = edgeArr[n].end;
        int foundDupl = -1;
        int nLocal = 0;
        while (withoutDupl[nLocal].start >= 0) {
            if ((localStart == withoutDupl[nLocal].start) && (localEnd == withoutDupl[nLocal].end)) {
                foundDupl = 0;
                break;
            }
            nLocal++;
        }
        if (foundDupl != 0) {
            struct Edge new = {.start=edgeArr[n].start, .end=edgeArr[n].end};
            withoutDupl[nW] = new;
            nW++;
            struct Edge nullEdge = {.start=-1, .end=-1};
            withoutDupl[nW] = nullEdge;
        }
        n++;
    }

    return withoutDupl;
}

/**
 * make a vertice list (array)
 * @brief make a orderd Vertice list (array) from an Edge array
 * @details generates a new int array containing the vertices from the given Edge array
 * @param edgeArr input Edge Array to generate the vertice array from
 * MUST contain a edge with negative values at the end to terminate properly
 * @return returns a new vertice list when succeeding, otherwise NULL
 **/
int *makeVerticeList(struct Edge *edgeArr) {
    if (edgeArr == NULL) {
        return NULL;
    }

    int big = edgeArr[0].start;
    int small = edgeArr[0].start;
    int n = 0;
    while (edgeArr[n].start >= 0) {
        int localStart = edgeArr[n].start;
        int localEnd = edgeArr[n].end;
        if (localStart > big) {
            big = localStart;
        } else if (localStart < small) {
            small = localStart;
        }
        if (localEnd > big) {
            big = localEnd;
        } else if (localEnd < small) {
            small = localEnd;
        }
        n++;
    }

    int size = big-small+2;
    int *vertices = malloc(sizeof(int) * size);
    int nV = 0;
    while (small <= big) {
        vertices[nV] = small;
        small++;
        nV++;
    }
    vertices[nV] = -1;

    return vertices;
}

/**
 * print vertices
 * @brief print out a vertice array
 * @details print a vertice array to stdout
 * @param vertices array to print from
 * MUST contain a vertice with negative value to terminate properly
 **/
void showVertices (int *vertices) {
    if (vertices == NULL) {
        return;
    }
    int n = 0;
    while (vertices[n] >= 0) {
        fprintf(stdout, "%d, ", vertices[n]);
        n++;
    }
}

/**
 * Size function for Vertices
 * @brief the size of vertice list
 * @details the size of a vertice list excluding the terminator (negative value)
 * @param vertices array of vertices
 * MUST contain a vertice with negative value to terminate properly
 * @return size of vertice list excluding the end (-1)
 **/
int getSize (int *vertices) {
    if (vertices == NULL) {
        return 0;
    }
    int n = 0;
    while (vertices[n] >= 0) {
        n++;
    }
    return n;
}

/**
 * Size function for Edges
 * @brief the size of aedge list
 * @details the size of edge list excluding the terminator (negative value)
 * @param edges array of edges
 * MUST contain an edge with negative values to terminate properly
 * @return size of edge list excluding the terminator (edge with negative value)
 **/
int getSizeEdges (struct Edge *edges) {
    if (edges == NULL) {
        return 0;
    }
    int n = 0;
    while (edges[n].start >= 0) {
        n++;
    }
    return n;
}

/**
 * Index function
 * @brief first index of an entry of a vertice array
 * @details finds the first appearence of the given input in the given array
 * @param vertices array of vertices
 * @param entry entry of the vertice-array
 * @return index of vertice array or (-1) if the array doesn't contain the entry
 **/
int getIndex (int *vertices, int entry) {
    if (vertices == NULL) {
        return -1;
    }
    int index = -1;
    int n = 0;
    while (vertices[n] >= 0) {
        if (vertices[n] == entry) {
            index = n;
            break;
        }
        n++;
    }

    return index;
}