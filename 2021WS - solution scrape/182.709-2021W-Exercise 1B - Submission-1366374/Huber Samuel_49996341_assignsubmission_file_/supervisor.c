/**
 * module name: supervisor.c
 * @author      Huber Samuel 11905181
 * @brief       supervisor chooses best 3 coloring problem solution from generated ones
 * @details     reads in generator.c generated solutions from circular_buffer.c
 *              remembers solutions with least edges deleted and writes them to stdout
 *              if solution with no edges deleted was found, program will terminate
 * @date        07.11.2021
**/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <limits.h>

#include "circular_buffer.h"

char *prog; // name of the called program

volatile sig_atomic_t quit = 0; // flag if atomic signal was set

/**
 * @brief prints out usage message
 * @details printed message advises the user, on how to call program properly \n program exits afterwards\n
 *          global variables used: 'prog'
 */
static void usage(void);

/**
 * @brief sets flag for SIGINT & SIGTERM
 * @details if flag is set program will terminate\n
 *          global variables used: 'quit'
 * @param signal: signal being handled
 */
static void signalHandling(int signal);


static void usage(void) {
    fprintf(stderr, "[%s] USAGE: %s \n", prog, prog);
    exit(EXIT_FAILURE);
}

static void signalHandling(int signal){
    quit = 1;
}

/**
 * @brief handles generated solutions
 * @details compares generated solutions given via the circular buffer,\n
 *          saves current best in structure,\n
 *          prints out newest best solution,\n
 *          terminates if solution without deleting edges was found\n
 *          global variables used: 'prog'
 * @param argc: amount of given arguments
 * @param argv: list of given arguments
 * @return if program run successful or with errors
 */
int main(int argc, char *argv[]) {

    // name of program
    prog = argv[0];

    edgeData_t currentBest;
    currentBest.size = INT_MAX;

    // if argc == 1, only name of program was given
    if (argc > 1) {
        fprintf(stderr, "[%s] ERROR: No arguments allowed!\n", prog);
        usage();
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); // initialize sa to 0
    sa.sa_handler = signalHandling;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    initiateBuffer();

    edgeData_t readData;
    readData.size = -1;

    while (!quit) {

        readBuffer(&readData);

        if (readData.size < currentBest.size) {
            if(readData.size == 0){
                printf("[%s] INFO: Solution with 0 edges found, graph is acyclic!\n",prog);
                quit = 1;
            } else if (readData.size == -1) {
                printf("[%s] INFO: No valid solution found yet!\n",prog);
                quit = 1;
            } else {
                memcpy(&currentBest,&readData,sizeof(edgeData_t));

                printf("[%s] INFO: Solution with %d edges: ",prog,currentBest.size);
                for (int i = 0; i < currentBest.size; ++i) {
                    printf("%d-%d ",currentBest.list[i].from,currentBest.list[i].to);
                }
                printf("\n");
            }
        }
    }

    setTermination(1);

    removeBuffer();

    exit(EXIT_SUCCESS);
}