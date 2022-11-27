/**
 * @file generator.c
 * @author Anni Chen
 * @date 08.11.2021
 * @brief Module to generate solutions
 * @details Genereates solutions for the 3-color Problem and submits them to the circular buffer
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <regex.h>
#include <signal.h>
#include "circular_buffer.h"

#define USAGE()                                               \
    {                                                         \
        fprintf(stdout, "[USAGE]: ./%s 0-1 0-2 1-2\n", name); \
        exit(EXIT_FAILURE);                                   \
    }

#define ERROR_MSG(...)                           \
    {                                            \
        fprintf(stderr, "[%s] [ERROR]: ", name); \
        fprintf(stderr, __VA_ARGS__);            \
        fprintf(stderr, "\n");                   \
    }

#define ERROR_EXIT(...)         \
    {                           \
        ERROR_MSG(__VA_ARGS__); \
        exit(EXIT_FAILURE);     \
    }

/**
 * Name of the current program.
 */
static char *name = "generator";

/**
 * @brief indicates whether the process should terminate
 */
static volatile sig_atomic_t quit = 0;

/**
 * Signal handler
 * @brief This function sets the global quit variable to 1.
 * @param signal the signal number
 */
static void handle_signal(int signal)
{
    quit = 1;
}

/**
 * validates input
 * @brief This function checks if the provided string satisfies the regex [0-9]+-[0-9]
 * @param str the string provided
 */
static int validate_input(const char *str)
{
    regex_t regex;
    int return_val;
    if (regcomp(&regex, "[0-9]+-[0-9]", REG_EXTENDED) != 0)
    {
        ERROR_EXIT("Failed to compile regular expression");
    }
    return_val = regexec(&regex, str, (size_t)0, NULL, 0);
    regfree(&regex);
    return return_val == 0 ? 0 : -1;
}

/**
 * @brief The entry point of the generator program.
 * @details Note that this function can only be executed if the supervisor is running.
 * This function starts with parsing the graph and storing its vertices and edges separately. It then assigns
 * random colors to the vertices and stores the edges which have vertices both containing the same color in an array.
 * This array sums up all edges that should be removed and is therefore a solution to the 3-color problem. After wrapping the
 * solution, the solution will be written to the circular buffer.
 * @param argc The argument counter.
 * @param argv The argument values.
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[])
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // Check if there are any arguments
    if (argc < 2)
    {
        fprintf(stderr, "[ERROR]: No edges provided\n");
        USAGE();
    }

    const char hyphen[2] = "-";
    char *pt1;
    char *pt2;
    int i;
    int maxVertices = (argc - 1) * 2;
    int numOfEdges = argc - 1;
    struct Edge edges[numOfEdges];
    struct Edge edgesToBeRemoved[numOfEdges];
    struct Vertex vertices[maxVertices];
    int currFreePosInVertices = 0;
    int currFreePosInEdges = 0;

    // initialize
    for (i = 0; i < numOfEdges; i++)
    {
        edges[i].v1 = NULL;
        edges[i].v2 = NULL;
    }

    //initialize
    for (i = 0; i < numOfEdges; i++)
    {
        edgesToBeRemoved[i].v1 = NULL;
        edgesToBeRemoved[i].v2 = NULL;
    }

    //initialize
    for (i = 0; i < maxVertices; i++)
    {
        vertices[i].color = -1;
        vertices[i].content = -1;
    }

    /********* parse graph and store edges*********/

    //iterate over all positional arguments
    for (i = 1; i < argc; i++)
    {
        size_t length = strlen(argv[i]);
        char *arg = malloc(sizeof(char) * (length + 1));
        if (arg == NULL)
        {
            ERROR_EXIT("Could not allocate memory");
        }

        //initialise arg
        memset(arg, 0, length + 1);
        //copy first positional argument to arg
        strncpy(arg, argv[i], length);

        //validate input
        if (validate_input(arg) != 0)
        {
            ERROR_EXIT("The edges provided are not valid. Vertices should only consist of positive numbers.");
            USAGE();
        }

        //get first token separated by hyphen
        pt1 = strtok(arg, hyphen);
        //get second token separated by hyphen
        pt2 = strtok(NULL, hyphen);

        size_t l1 = strlen(pt1);
        char *c1 = malloc(sizeof(char) * (l1 + 1));
        if (c1 == NULL)
        {
            free(arg);
            ERROR_EXIT("Could not allocate memory");
        }
        strcpy(c1, pt1);
        c1[l1] = 0;

        size_t l2 = strlen(pt2);
        char *c2 = malloc(sizeof(char) * (l2 + 1));
        if (c2 == NULL)
        {
            free(arg);
            ERROR_EXIT("Could not allocate memory");
        }
        strcpy(c2, pt2);
        c2[l2] = 0;

        //convert char to long
        long content1 = strtol(c1, NULL, 10);
        long content2 = strtol(c2, NULL, 10);

        free(c1);
        free(c2);

        //create new vertex out of first content if it does not exist
        struct Vertex v1 = {-1, content1};
        int added = addVertexIfExists(v1, vertices, maxVertices, currFreePosInVertices);

        if (added)
        {
            currFreePosInVertices++;
        }

        //create new vertex out of second content if it does not exist
        struct Vertex v2 = {-1, content2};
        added = addVertexIfExists(v2, vertices, maxVertices, currFreePosInVertices);

        if (added)
        {
            currFreePosInVertices++;
        }

        struct Vertex *v3 = getAdressOfVertex(vertices, maxVertices, content1);
        struct Vertex *v4 = getAdressOfVertex(vertices, maxVertices, content2);

        //create edge if it does not exist
        added = addEdgesIfExists(edges, v3, v4, numOfEdges, currFreePosInEdges);

        if (added)
        {
            currFreePosInEdges++;
        }

        free(arg);
    }

    // /********* store Graph end *********/

    if (open_circbuf('c') == -1)
    {
        ERROR_EXIT("Failed to open circular buffer. Supervisor may not be running.");
    }

    if (init_sem('c') == -1)
    {
        ERROR_EXIT("Failed to initialize semaphores");
    }

    struct Solution solution;

    while (get_state() && !quit)
    {

        struct timeval time;
        gettimeofday(&time, NULL);
        // different seed at every execution
        srand(((time.tv_sec * 1000) + (time.tv_usec / 1000)) * getpid());

        assignRandomColors(vertices, maxVertices);

        removeEdges(edges, edgesToBeRemoved, numOfEdges);

        int numOfEdgesToRemove = countEdges(edgesToBeRemoved, numOfEdges);

        //discard the solution
        if (numOfEdgesToRemove > MAX_EDGES || numOfEdgesToRemove == numOfEdges)
        {
            continue;
        }

        //copy edges to remove to solution
        solution = prepareSolution(numOfEdgesToRemove, edgesToBeRemoved);

        if (write_solution(solution) == -1)
        {
            ERROR_EXIT("Failed to write to buffer");
        }
    }

    if (close_circbuf('c') == -1)
    {
        ERROR_EXIT("Failed to close buffer");
    }

    return EXIT_SUCCESS;
}