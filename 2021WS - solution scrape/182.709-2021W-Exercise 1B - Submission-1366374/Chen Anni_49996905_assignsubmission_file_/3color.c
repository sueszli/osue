#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "3color.h"

//returns 1 if vertex is added, 0 if it already exists
int addVertexIfExists(struct Vertex v, struct Vertex vertices[], int numOfVertices, int freePos)
{
    int i;
    for (i = 0; i < numOfVertices; i++)
    {
        //check if vertex already exist, if so return
        if (vertices[i].content == v.content)
        {
            return 0;
        }
    }

    vertices[freePos] = v;

    return 1;
}

struct Vertex *getAdressOfVertex(struct Vertex vertices[], int numOfVertices, long content)
{
    int i;

    for (i = 0; i < numOfVertices; i++)
    {
        if (vertices[i].content == content)
        {
            return &vertices[i];
        }
    }

    return NULL;
}

// //returns 1 if edge is added, 0 if it already exists
int addEdgesIfExists(struct Edge edges[], struct Vertex *newV1, struct Vertex *newV2, int numOfEdges, int freePos)
{

    int i;
    struct Vertex *v1;
    struct Vertex *v2;

    for (i = 0; i < numOfEdges; i++)
    {
        v1 = edges[i].v1;
        v2 = edges[i].v2;

        if (v1 != NULL && v2 != NULL)
        {
            if (v1->content == newV1->content && v2->content == newV2->content)
            {
                return 0;
            }
        }
    }

    edges[freePos].v1 = newV1;
    edges[freePos].v2 = newV2;

    return 1;
}

void printAllVertices(struct Vertex vertices[], int numOfVertices)
{
    //print all vertices
    int i;
    for (i = 0; i < numOfVertices; i++)
    {
        printf("Vertex %d:\n Content is: %ld\n Color is: %d\n", i, vertices[i].content, vertices[i].color);
    }
}


void printAllEdges(struct Edge edges[], int numOfEdges)
{
    int i;
    //print all edges
    for (i = 0; i < numOfEdges; i++)
    {
        // printf("Edge %d:\n Content of first vertex is: %s\n Content of second vertex is: %s\n", i, edges[i].v1.content, edges[i].v2.content);
        if (edges[i].v1 != NULL && edges[i].v2 != NULL)
        {
            printf("Edge %d:\n Content of first vertex is: %ld\n Color is: %d \n Content of second vertex is: %ld\n Color is: %d\n", i, edges[i].v1->content, edges[i].v1->color, edges[i].v2->content, edges[i].v2->color);
        }
    }
}

void assignRandomColors(struct Vertex vertices[], int numOfVertices)
{

    int i;

    //reset all colors
    for (i = 0; i < numOfVertices; i++)
    {
        vertices[i].color = -1;
    }

    for (i = 0; i < numOfVertices; i++)
    {
        if (vertices[i].content != -1 && vertices[i].color == -1)
        {
            //assign random color
            vertices[i].color = (rand() % 3) + 1;
        }
    }
}

void removeEdges(struct Edge edges[], struct Edge edgesToBeRemoved[], int numOfEdges)
{
    int i;
    struct Vertex *v1;
    struct Vertex *v2;
    int color1;
    int color2;
    int currEmptyPos = 0;

    //reset
    for (i = 0; i < numOfEdges; i++)
    {
        edgesToBeRemoved[i].v1 = NULL;
        edgesToBeRemoved[i].v2 = NULL;
    }

    for (i = 0; i < numOfEdges; i++)
    {

        v1 = edges[i].v1;
        v2 = edges[i].v2;

        if (v1 != NULL && v2 != NULL)
        {
            if (v1->content != -1 && v2->content != -1)
            {
                color1 = v1->color;
                color2 = v2->color;
                if (color1 == color2)
                {
                    edgesToBeRemoved[currEmptyPos] = edges[i];
                    currEmptyPos++;
                }
            }
        }
    }
}

int countEdges(struct Edge edges[], int numOfEdges)
{
    int i;
    struct Vertex *v1;
    struct Vertex *v2;
    int count = 0;

    for (i = 0; i < numOfEdges; i++)
    {

        v1 = edges[i].v1;
        v2 = edges[i].v2;

        if (v1 != NULL && v2 != NULL)
        {
            if (v1->content != -1 && v2->content != -1)
            {
                count++;
            }
        }
    }

    return count;
}

struct Solution prepareSolution(int numOfEdgesToRemove, struct Edge removedEdges[])
{

    int i;

    struct Solution sol;
    sol.numOfRemovedEdges = numOfEdgesToRemove;
    sol.status = 0;

    long c1;
    long c2;

    //initialize
    for (i = 0; i < MAX_EDGES; i++)
    {
        sol.edges[i].content1 = -1;
        sol.edges[i].content2 = -1;
    }

    for (i = 0; i < numOfEdgesToRemove; i++)
    {
        c1 = removedEdges[i].v1->content;
        c2 = removedEdges[i].v2->content;

        sol.edges[i].content1 = c1;
        sol.edges[i].content2 = c2;
    }

    return sol;
}

void printSolution(struct Solution solution)
{
    int edgesToRemove = solution.numOfRemovedEdges;
    int i;
    long c1;
    long c2;

    if (edgesToRemove == 0)
    {
        printf("The graph is 3-colorable!\n");
    }
    else
    {

        printf("Solution with %d edge(s):", edgesToRemove);

        for (i = 0; i < edgesToRemove; i++)
        {
            c1 = solution.edges[i].content1;
            c2 = solution.edges[i].content2;

            printf(" %ld-%ld", c1, c2);
        }

        printf("\n");
    }
}