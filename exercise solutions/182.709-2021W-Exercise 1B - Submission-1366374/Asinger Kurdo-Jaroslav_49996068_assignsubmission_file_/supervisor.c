#include "circbuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

/**
 * @name supervisor
 * @author Kurdo-Jaroslav Asinger, 01300351
 * @brief acts as a server - extracts 3-coloring solutions of graphs found by generator
 * @details open shared memory (with supervisor)
 *          loop: read a solution from a circular buffer
 *                compare it with the currently best foun solution - save as new best and print if it is better
 *          repeat loop until either a solution without edge removal is found, or an interruption signal is received
 *          communicate termination to generator so it terminates too
 * @date Nov/11/2021
 */

volatile sig_atomic_t quit = 0;

void handle_signal(int signal)
{
    quit = 1;
}

int main(int argc, char **argv)
{
    /**
     * @brief signal handling
     *        terminate with interrupt signal
     * 
     */
    struct sigaction sa = {
        .sa_handler = handle_signal};

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // setup shared memory
    setup(true, argv[0]);

    // initialize best solution with a value higher than the worst accepted solution.
    coloringSol bestSol;
    bestSol.deletedCount = MAX_SOLUTION_SIZE + 1;

    while (!quit)
    {

        /**
         * @brief read a solution from the circular buffer.
         *        if read failed, an invalid solution is returned.
         *        the program will terminate in case of an invalid solution
         * 
         */
        coloringSol newSol = readBuf(argv[0]);
        if (newSol.deletedCount == -1)
        {
            stopSol();
            cleanup(true, argv[0]);
            exit(EXIT_FAILURE);
        }

        if (bestSol.deletedCount > newSol.deletedCount)
        {
            bestSol = newSol;
            if (bestSol.deletedCount == 0)
            {
                printf("The graph is 3-colorable!\n");

                /* assumption: if we already found a solution without removing
                any edges, we can stop trying to find better ones, as this is impossible*/
                quit = 1;
            }
            else
            {
                printf("Solution with %d edges: ", bestSol.deletedCount);
                int i;
                for (i = 0; i < bestSol.deletedCount; i++)
                {
                    printf("%d-%d ", bestSol.deletedEdges[i].from, bestSol.deletedEdges[i].to);
                }
            }
            printf("\n");
            fflush(stdout);
        }
    }
    /**
     * @brief set the state of the circular buffer to -1
     *        this signals generator to stop the iterations and to terminate as well
     */
    stopSol();
    if (cleanup(true, argv[0]) != 0)
    {
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}