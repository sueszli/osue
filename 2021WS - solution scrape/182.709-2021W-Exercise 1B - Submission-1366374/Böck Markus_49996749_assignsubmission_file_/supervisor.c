/**
 * supervisor
 * 
 * @author Markus Böck, 12020632
 * @date 2021.11.10
 * @brief Supervisor responsible for collecting solutions of generators
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#include "API.h"
#include "CircularBuffer.h"

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
 * @brief global variable set by the signal handler to indicate a 
 * termination request
 */
volatile sig_atomic_t quit = 0;

/**
 * @brief signal handler for SIGINT and SIGTERM. Simply sets quit to 1
 * @param signal Number of the signal received
 */
static void signalHandler(int signal)
{
    quit = 1;
}

/**
 * @brief main function of supervisor
 * @param argc argument count
 * @param argv argument data
 * @return exit code
 */
int main(int argc, char* argv[])
{
    executableName = argv[0];
    if (argc > 1)
    {
        error("No command line arguments are allowed");
        return EXIT_FAILURE;
    }
    
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signalHandler;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    CircularBuffer buffer;
    // Server has exclusive ownership and is responsible for unlinking
    // Hence O_CREAT | O_EXCL
    CircularBufferErrors errors = CircularBufferCreate(&buffer, API_KEY_STRING, SharedMemorySize, O_CREAT | O_EXCL | O_RDWR);
    switch (errors)
    {
        case CBSharedMemoryError:
            error("Error creating shared memory /dev/shm/%s: %s", API_KEY_STRING, strerror(errno));
            return EXIT_FAILURE;
        case CBSemaphoreError:
            error("Error creating semaphore: %s", strerror(errno));
            return EXIT_FAILURE;
        case CBMemoryAllocationError:
            error("Out of memory");
            return EXIT_FAILURE;
        default:break;
    }
    
    size_t solutionCapacity = 0;
    Solution* solution = malloc(sizeof(Solution));
    if (!solution)
    {
        error("Out of memory");
        CircularBufferRequestClose(&buffer);
        CircularBufferDestroy(&buffer);
        return EXIT_FAILURE;
    }
    size_t bestSolutionSoFar = -1;
    while(!quit)
    {
        // Reads into Solution have to be done in two parts. First
        // the count to get the amount of edges and potentially resize
        // the solution. 
        // Second to read in the actual edges
        CircularBufferRead(&buffer, &solution->count, sizeof(size_t));
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
                CircularBufferRequestClose(&buffer);
                CircularBufferDestroy(&buffer);
                return EXIT_FAILURE;
            }
            solution = newSolution;
        }
        CircularBufferRead(&buffer, ((char*)solution) + offsetof(Solution, edges), sizeof(Edge) * solution->count);
        if (solution->count >= bestSolutionSoFar)
        {
            continue;
        }
        bestSolutionSoFar = solution->count;
        printf("[%s] Solution with %zu edges:", executableName, solution->count);
        for (size_t i = 0; i < solution->count; i++)
        {
            printf(" %lu-%lu", solution->edges[i].first, solution->edges[i].second);
        }
        printf("\n");
        fflush(stdout);
        if (bestSolutionSoFar == 0)
        {
            break;
        }
    }
    free(solution);
    CircularBufferRequestClose(&buffer);
    CircularBufferDestroy(&buffer);
    return EXIT_SUCCESS;
}
