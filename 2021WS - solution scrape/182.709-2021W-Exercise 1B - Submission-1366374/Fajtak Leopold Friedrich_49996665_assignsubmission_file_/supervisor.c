/**
 * @file supervisor.c
 * @author Leopold Fajtak (e0152033@student.tuwien.ac.at)
 * @brief See header
 * @version 0.2
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "supervisor.h"

/**
 * @brief initialize variables, allocate space, create shared memory and semaphores
 * 
 */
static void init(void);

/**
 * @brief free allocated space, unlink shared memory and semaphores
 * 
 */
static void cleanup(void);

/**
 * @brief print an error message
 * 
 * @param msgStart Start of error message
 */
static void handle_error(char* msgStart);

/**
 * @brief Compares g to the current best solution and sets bestSolution if g is better.
 * The other pointer is freed
 * 
 * @param g Solution to be compared with bestSolution
 */
static void compareSolution(graph* g);

/**
 * @brief Print current best solution to stdout
 * 
 */
static void printBestSolution(void);

static bool quit; /*! variable is set in signal handlers. If it is true, the
                    process will terminate - initialized in init() */
static graph* bestSolution; /*! The currenty best soultion found - initialized in
                       init() */
static char *myprog; /*! argv[0] */

int main(int argc, char **argv){
    myprog = argv[0];
    init();
    graph* g=NULL;
    while(!quit){
        g = readBuf();
        if (g==NULL) {
            if(errno == EINTR)
                continue;
            handle_error("Failure while reading free_sem");
            cleanup();
            exit(EXIT_FAILURE);
        }
        compareSolution(g);
    }
    cleanup();
    return EXIT_SUCCESS;
}

void handle_sigterm(int signal){
    quit=true;
}

static void init(void){
    quit=false;
    bestSolution = NULL;
    //Init signal handling
    struct sigaction sa_term;
    memset(&sa_term, 0, sizeof(sa_term));
    sa_term.sa_handler = handle_sigterm;
    sigaction(SIGINT, &sa_term, NULL);
    sigaction(SIGTERM, &sa_term, NULL);
    sigaction(SIGQUIT, &sa_term, NULL);

    //Init circular buffer
    if(init_circ_buf()!=0){
        handle_error("unable to initialize shared memory");
        exit(EXIT_FAILURE);
    }
}

static void cleanup(void){
    free(bestSolution);
    if(cleanup_circ_buf()!=0){
        handle_error("unable to cleanup shared memory");
        exit(EXIT_FAILURE);
    }
}

static void handle_error(char* msgStart){
    fprintf(stderr, "%s: %s: %s\n", myprog, msgStart, strerror(errno));
}

static void compareSolution(graph* g){
    assert(g!=NULL);
    if(bestSolution == NULL){
        bestSolution = g;
        printBestSolution();
        return;
    }
    if((g->numEdges) < (bestSolution->numEdges)){
        free(bestSolution);
        bestSolution = g;
        printBestSolution();
    } else {
        free(g);
    }
}

static void printBestSolution(void){
    if(bestSolution->numEdges==0){
        printf("The graph is acyclic!\n");
        quit=true;
    }
    printf("Soultion with %d edges: \n", bestSolution->numEdges);
    printGraph(bestSolution);
}
