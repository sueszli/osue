/**
* @file supervisor.c
* @author Laurenz Gaisch <e11808218@student.tuwien.ac.at>
* @date 14.11.2021
*
* @brief Prints edges that need to be removed to get an acyclic graph
* @details Checks the shared memory for solutions. If there is a new one posted by a generator, print it in the console.
*/


#include <stdio.h>
#include <bits/types/sig_atomic_t.h>
#include <signal.h>
#include "circular-buffer.h"

/**
 * Startpoint of the program
 * @param argc EMPTY
 * @param argv Only program name
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[]) {
    //initialize the signal handler, that closes the generator on supervisor exit.
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    int min = MAX_EDGES;
    int state = 1;
    createMemory();

    while(state<2) {
        shmp = readBuffer();

        state = shmp->state;

        //Only print if the state is 1 (running) and the count is the same or less than previous iteration
        if(state == 1 && shmp->count <= min) {
            min = shmp->count;
            printf("[%s] Solution with %d edges: ", argv[0], shmp->count);
            for(int i = 0; i<min;i++){
                printf("%d-%d",shmp->firstNodeOfEdge[i],shmp->secondNodeOfEdge[i]);
                if(i<min - 1) printf(", ");
            }
            printf("\n");
        }
    }
    printf("[%s] The graph is acyclic!\n",argv[0]);
    disposeSuper();
    return EXIT_SUCCESS;
}
