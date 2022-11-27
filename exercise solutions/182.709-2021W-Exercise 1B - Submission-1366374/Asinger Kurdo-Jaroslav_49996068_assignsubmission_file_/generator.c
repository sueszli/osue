#include "circbuffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

/**
 * @name generator
 * @author Kurdo-Jaroslav Asinger, 01300351
 * @brief generates possible solutions for the 3-coloring problem of graphs
 * @details open shared memory (with supervisor)
 *          input edges
 *          loop: vertices are colored randomly
 *                edges are removed until no adjacent vertices have the same color
 *                if solution is better than the previously defined worst accepted solution, it is passed to a shared buffer
 *          repeat loop until supervisor sends termination signal
 * @date Nov/11/2021
 */


static void usage(char *progname);

/**
 * @brief colors all vertices in the graph at random
 * 
 * @param colors an int array which has one position for each vertex
 * @param vertCount the number of vertices in the graph
 */
static void randomColor(int *colors, int vertCount);

/**
 * @brief Get a solution with the current vertex-coloring
 * 
 * @param base an "empty" solution
 * @param edges an array of edges (edge is a struct which holds two int variables)
 * @param edgeCount the number of edges in the graph
 * @param colors the colors of the vertices in the graph (realized as int values)
 * @param vertCount the number of vertices in the graph
 */
static void getSol(coloringSol *base, edge *edges, int edgeCount, int *colors, int vertCount);


int main(int argc, char **argv)
{

    if (setup(false, argv[0]) != 0)
    {
        exit(EXIT_FAILURE);
    }

    int from = 0;
    int to = 0;
    int vertCount = 0;

    edge *edges = malloc((argc - 1) * sizeof(edge));

    int i;
    // start at 1 because argv[0] holds program name
    for (i = 1; i < argc; i++)
    {
        int match = sscanf(argv[i], "%d-%d", &from, &to);

        // check if all arguments match the form "a-b" whereas a and b are integers
        if (match != 2)
        {
            usage(argv[0]);
        }

        // assumption: verteces count starts at 0 - therefore, negative integers are not allowed
        if ((from < 0) || (to < 0))
        {
            usage(argv[0]);
        }

        // assumption: the graph has as many verteces as the highest given integer value + 1 (for vertex 0)
        if (from > vertCount)
        {
            vertCount = from;
        }
        if (to > vertCount)
        {
            vertCount = to;
        }
        edges[i - 1].from = from;
        edges[i - 1].to = to;
    }

    vertCount++;

    int *colors = malloc(sizeof(int) * vertCount);
    srand(time(0));

    /**
     * @brief solutions are calculated in a loop.
     *        the loop stops once the supervisor terminated.
     * 
     */
    while (true)
    {

        randomColor(colors, vertCount);

        coloringSol solution;
        solution.deletedCount = 0;

        // if the graph has 1 or 0 vertices, it already is 3-colorable
        if (vertCount > 1)
        {
            getSol(&solution, edges, argc - 1, colors, vertCount);
        }

        /*for (i = 0; i < vertCount; i++){
            printf("%d ", colors[i]);
        }
        printf("Solution: %d\n", solution.deletedCount);
        for(i = 0; i < solution.deletedCount; i++){
            printf("%d-%d ", solution.deletedEdges[i].from, solution.deletedEdges[i].to);
        }
        printf("\n\n");*/

        // check if supervisor terminated
        if (isRunning() == false)
        {
            break;
        }
        // write the current soluion in the buffer if it is valid
        if (solution.deletedCount >= 0)
        {
            if (writeBuf(solution, argv[0]) != 0)
            {
                free(colors);
                free(edges);
                cleanup(false, argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }

    if (cleanup(false, argv[0]) != 0)
    {
        free(colors);
        free(edges);
        exit(EXIT_FAILURE);
    }
    free(colors);
    free(edges);
    exit(EXIT_SUCCESS);
}

static void getSol(coloringSol *base, edge *edges, int edgeCount, int *colors, int vertCount)
{
    int i;
    for (i = 0; i < edgeCount; i++)
    {
        int from = edges[i].from;
        int to = edges[i].to;
        if (colors[from] == colors[to])
        {
            // we do not consider solutions which need to remove more than MAX_SOLUTION_SIZE (8 in this case)
            if (base->deletedCount >= MAX_SOLUTION_SIZE)
            {
                // -1 as a identifier for an invalid solution
                base->deletedCount = -1;
                return;
            }
            base->deletedEdges[base->deletedCount] = edges[i];
            base->deletedCount = base->deletedCount + 1;
        }
    }
}

static void randomColor(int *colors, int vertCount)
{
    int i;
    for (i = 0; i < vertCount; i++)
    {
        // as we check for 3-coloring, we use randoms between 0 and 2
        colors[i] = rand() % 3;
    }
}

static void usage(char *progname)
{
    fprintf(stderr, "USAGE: %s [int-int] ...\n", progname);
    exit(EXIT_FAILURE);
}