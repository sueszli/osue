
/**
 * generator
 * 
 * @author Markus Böck, 12020632
 * @date 2021.11.10
 * @brief Generator responsible for creating solutions and submitting
 * them to the supervisor
 */

#include "CircularBuffer.h"
#include "API.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>

/**
 * @brief points to argv[0], the executable name of the program
 */
static const char* executableName = NULL;

#ifdef __GNUC__
#define FORMAT_FUNC __attribute__((format(printf, 1, 2)))
#else
#define FORMAT_FUNC
#endif

/**
 * @brief utility function for outputting errors
 * 
 * This function is used as a utility throughout the program to ease outputting errors.
 * All output is done to stderr with the value of the global 'executableName' preprended to the output.
 * It's signature is identical to printf, and its format strings + varargs allow the same values and options as printf.
 * @param format Format string as defined by printf & friends
 */
FORMAT_FUNC static void error(const char* format,...) 
{
    (void)fprintf(stderr, "%s: ", executableName);
    va_list list;
    va_start(list, format);
    (void)vfprintf(stderr, format, list);
    va_end(list);
    (void)fprintf(stderr, "\n");
}

/**
 * @brief displays the usage of the executable
 */
static void displayUsage()
{
    (void)printf("SYNOPSIS\n"
                 "\t%s EDGE1...\n"
                 "EXAMPLE\n"
                 "\t%s 0-1 0-2 0-3\n", executableName, executableName);
}

/**
 * @brief Graph of the graph colouring problem
 * 
 * size are the amount of edges in edges.
 * verticeCount is the amount of vertices in the graph. 
 */
typedef struct EdgeList
{
    size_t size;
    Edge* edges;
    size_t verticeCount;
} EdgeList;

/**
 * @brief Parses the command line arguments into the edgeList
 * @param edgeList EdgeList to write the results into.
 * @param argc Argument count from main
 * @param argv Argument strings from main
 * @return true if successful, false on errors
 */
static bool readEdgesFromCLI(EdgeList* edgeList, int argc, char* argv[])
{
    assert(edgeList);
    edgeList->size = argc - 1;
    edgeList->edges = malloc(edgeList->size * sizeof(Edge));
    if (!edgeList->edges)
    {
        error("Out of memory");
        return false;
    }
    size_t verticeCount = 0;
    for (int i = 1; i < argc; i++)
    {
        char* separator = argv[i];
        for(;*separator && *separator != '-'; separator++);
        if (*separator == '\0')
        {
            error("Expected edge instead of %s", argv[i]);
            goto error;
        }
        // Special case, someone thought they were funny and put a 
        // - infront of the first number
        if (separator == argv[i])
        {
            error("Value in edges must be positive: %s", argv[i]);
            goto error;
        }
        *separator = '\0';
        errno = 0;
        char* endPtr = NULL;
        unsigned long firstNum = strtoul(argv[i], &endPtr, 10);
        if (errno)
        {
            error("Expected integer as first component of edge: %s", strerror(errno));
            goto error;
        }
        if (endPtr != argv[i] + strlen(argv[i]))
        {
            error("Expected integer as first component of edge. Starting at: %s", endPtr);
            goto error;
        }
        errno = 0;
        endPtr = NULL;
        unsigned long secondNum = strtoul(separator + 1, &endPtr, 10);
        if (errno)
        {
            error("Expected integer as second component of edge: %s", strerror(errno));
            goto error;
        }
        if (endPtr != separator + 1 + strlen(separator + 1))
        {
            error("Expected integer as second component of edge. Starting at: %s", endPtr);
            goto error;
        }
        if (firstNum > verticeCount)
        {
            verticeCount = firstNum;
        }
        if (secondNum > verticeCount)
        {
            verticeCount = secondNum;
        }
        edgeList->edges[i-1] = (Edge){.first = firstNum, .second = secondNum};
    }
    edgeList->verticeCount = verticeCount + 1;
    return true;
    
error:
    free(edgeList->edges);
    return false;
} 

/**
 * @brief main program of generator
 * @param argc argument count
 * @param argv argument content
 * @return exit code
 */
int main(int argc,char* argv[])
{
    executableName = argv[0];
    if (argc == 1)
    {
        displayUsage();
        return EXIT_FAILURE;
    }
    EdgeList edgeList;
    if (!readEdgesFromCLI(&edgeList, argc, argv))
    {
        // readEdgesFromCLI already emitted an error message
        return EXIT_FAILURE;
    }
    CircularBuffer buffer;
    CircularBufferErrors errors = CircularBufferCreate(&buffer, API_KEY_STRING, SharedMemorySize, O_RDWR);
    switch (errors)
    {
        case CBSharedMemoryError:
            error("Error open shared memory /dev/shm/%s: %s\n"
                  "Is the server running?", API_KEY_STRING, strerror(errno));
            free(edgeList.edges);      
            return EXIT_FAILURE;
        case CBSemaphoreError:
            error("Error creating semaphore: %s", strerror(errno));
            free(edgeList.edges);
            return EXIT_FAILURE;
        case CBMemoryAllocationError:
            error("Out of memory");
            free(edgeList.edges);
            return EXIT_FAILURE;
        default:break;
    }
    
    // Seed randon number generator
    srand(time(NULL));
    
    enum Colours
    {
        Red = 0,
        Green = 1,
        Blue = 2,
    };

    enum Colours* verticeColours = malloc(sizeof(enum Colours) * edgeList.verticeCount);
    if (!verticeColours)
    {
        error("Out of memory");
        CircularBufferDestroy(&buffer);
        free(edgeList.edges);
        return EXIT_FAILURE;
    }
    Solution* solution = malloc(sizeof(Solution));
    if (!solution)
    {
        error("Out of memory");
        free(verticeColours);
        CircularBufferDestroy(&buffer);
        free(edgeList.edges);
        return EXIT_FAILURE;
    }
    size_t solutionCapacity = 0;
    while (true)
    {
        for (size_t i = 0; i < edgeList.verticeCount; i++)
        {
            verticeColours[i] = rand() % 3;
        }
        solution->count = 0;
        for (size_t i = 0; i < edgeList.size; i++)
        {
            Edge edge = edgeList.edges[i];
            if (verticeColours[edge.first] != verticeColours[edge.second])
            {
                continue;
            }
            solution->count++;
            if (solution->count > solutionCapacity)
            {
                // To avoid frequent reallocations grow the capacity 
                // by a factor of 1.5. If that is less than the needed
                // count, resize to count
                solutionCapacity = solutionCapacity + (solutionCapacity >> 1);
                if (solutionCapacity < solution->count)
                {
                    solutionCapacity = solution->count;
                }
                Solution* newSolution = realloc(solution, sizeof(Solution) + sizeof(Edge) * solutionCapacity);
                if (!newSolution)
                {
                    error("Out of memory");
                    free(solution);
                    free(verticeColours);
                    CircularBufferDestroy(&buffer);
                    free(edgeList.edges);
                    return EXIT_FAILURE;
                }
                solution = newSolution;
            }
            solution->edges[solution->count-1] = edge;
        }
        if (CircularBufferCloseRequested(&buffer))
        {
            break;
        }
        CircularBufferWrite(&buffer, solution, sizeof(Solution) + sizeof(Edge) * solution->count);
    }
    
    free(solution);
    free(verticeColours);
    CircularBufferDestroy(&buffer);
    free(edgeList.edges);
    return EXIT_SUCCESS;
}
