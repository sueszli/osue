/**
 * @file supervisor.c
 * @author Michael Bumbar <e11775807@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Main program module of the supervisor.
 * 
 * This program creates a shared memory object containing a circular buffer and opens three POSIX semaphores. It waits for a generator process to write solutions to the circular buffer, reads them
 * and prints them to stdout.
 * The program terminates when it receives SIGINT or SIGTERM.
 * The program terminates if a solution read from the buffer has zero edges.
 **/

#include "circularbuffer.h"


extern circbuffer* buffer; extern circbuffer* buffer; /**< The name of the circular buffer. It is declared in circularbuffer.h. */
volatile sig_atomic_t quit = 0; /**< This variable shows the program wether it received SIGINT or SIGTERM. */


/**
 * Handles signals.
 * @brief Changes the value of quit to 1.
 * @param signal The signal this handler reacts to.
 */
void handle_signal(int signal){
    quit = 1;
}


/**
 * Program entry point.
 * @brief The program starts here. This function opens calls functions to create the shared memory object and waits for solutions in the circular buffer. If it reads a solution that is shorter then the
 * shortest solution yet the new solution is saved and printed to stdout. If a solution has length 0 the program prints a message and terminates. It also terminates with SIGINT and SIGTERM. Whenever
 * this function terminates it frees all resources and unlinks the shared memory object.
 * @details This function calls getopt and parses the command line arguments. If an option is received a the usage function is called. If the function received any positional arguments it terminates 
 * and calls errorHandling.
 * Whenever the function receives SIGINT or SIGTERM the signal handler handle_signal is invoked and quit is set to 1.
 * The function creates and opens shared memory and POSIX semaphores. Then it waits for solutions as long as quit is not set to 1. Every is solution is read and checked. If the length of a solution is
 * smaller than the length of the shortest solution until now the solution is printed to stdout and saved. If a solution has length 0 the function prints a success message unlinks and closes the shared
 * memory and exits with EXIT_SUCCESS. In cas of an error errorHandling is called, the ressources are freed and the shared memory is unlinked.
 * constants: SUP
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv){
    int c;
    while((c = getopt(argc,argv,"")) != -1){
        switch (c)
        {
        case '?': usage("", SUP); break; 
        default:  usage("", SUP); break; 
            break;
        }
    }
    if((argc - optind) != 0){
        errorHandling(SUP, "getopt failed", strerror(errno));
    }
    struct sigaction sa_sigint;
    struct sigaction sa_sigterm;
    memset(&sa_sigint,0,sizeof(sa_sigint));
    memset(&sa_sigterm,0,sizeof(sa_sigterm));
    sa_sigint.sa_handler = handle_signal;
    sa_sigterm.sa_handler = handle_signal;
    sigaction(SIGINT, &sa_sigint, NULL);
    sigaction(SIGTERM, &sa_sigterm, NULL);
    arc_set* result = malloc(sizeof(arc_set));
    int32_t currLen = INT32_MAX;
    if(setup_buffer() != EXIT_SUCCESS){
        free(result);
        errorHandling(SUP,"setup_buffer failed",strerror(errno));
    }
    while(quit != 1){
        int c;
        if((c = read_buffer(result)) == -1){
            break;
        } else if(c == EXIT_FAILURE){
            free(result);
            errorHandling(SUP,"sem_wait failed",strerror(errno));
        }
        int32_t cnt = result->length;
        if(cnt == 0){
            fprintf(stdout,"The graph is acyclic!");
            quit = 1;
        } else if(cnt < currLen){
            currLen = cnt;
            fprintf(stdout,"[%s] Solution with %d edges: ", SUP, currLen);
            show_solution(result);
            fprintf(stdout,"\n");
        }
    }
    activity_change();
    free(result);
    if(free_original_buffer() == EXIT_FAILURE){
        errorHandling(SUP,"free_original_buffer failed",strerror(errno));
    }
    return EXIT_SUCCESS;
}