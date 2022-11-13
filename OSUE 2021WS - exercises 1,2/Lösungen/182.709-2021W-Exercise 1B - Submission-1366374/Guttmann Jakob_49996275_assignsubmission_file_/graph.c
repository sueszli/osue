/**
 * @file graph.c
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at>
 * @brief implentation of methods declared in graph.h, further description see in graph.h docu
 * @date 11.11.2021
 * 
 */
#include "graph.h"
#include <time.h>

void permutate(int *nodes, int length)
{

    for (int i = length - 1; i > 0; i--)
    {
        int j = (int)rand() % (i + 1);

        int temp = nodes[i];
        nodes[i] = nodes[j];
        nodes[j] = temp;
    }
}

int search(int *nodes, int node, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (nodes[i] == node)
            return i;
    }
    return -1;
}

int feedback_arc_set(int numEdges, int numNodes, int actualMin, int *permutNodes, Edge *edges, Edge *arcset)
{
    int count_arcset = 0;
    for (int i = 0; i < numEdges; i++)
    {
        int src = edges[i].src;
        int dest = edges[i].dest;
        int pos_src = search(permutNodes, src, numNodes);
        int pos_dest = search(permutNodes, dest, numNodes);
        if (pos_src == -1 || pos_dest == -1)
            return -2;

        if (pos_src > pos_dest)
        {
            arcset[count_arcset].src = src;
            arcset[count_arcset].dest = dest;
            count_arcset++;

            if (count_arcset >= actualMin)
            {
                return -1;
            }
        }
    }
    return count_arcset;
}

void printArcset(Edge *arcset, int size, char *caller)
{
    if (size == 0)
    {
        printf("[%s] The graph is acyclic\n", caller);
        return;
    }
    printf("[%s] Solution with %d edges:", caller, size);
    for (int i = 0; i < size; i++)
    {
        printf(" %d-%d", arcset[i].src, arcset[i].dest);
    }
    fflush(stdout);
}