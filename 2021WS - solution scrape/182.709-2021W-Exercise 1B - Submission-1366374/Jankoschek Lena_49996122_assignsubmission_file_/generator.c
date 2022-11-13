/**
 * @file generator.c
 * @author Lena Jankoschek - 12019852
 * @brief 
 * @version 0.1
 * @date 2021-11-12
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>

#include "circularbuffer.h"



/**
 * usage function
 * @brief this function prints the right usage of the program, its program name and exits the program with EXIT_FAILURE.
 * 
 * @param program_name - name of the program (argv[0])
 */
static void usage(const char *program_name) {
    fprintf(stderr, "[%s] Usage: EDGE1...\n", program_name);
    fprintf(stderr, "[%s] Example: 0-1 0-2 0-3 1-2 1-3 2-3\n", program_name);
    exit(EXIT_FAILURE);
}


/**
 * @brief this function parses the arguments of the program
 * @details this function checks if there is at least one edge given as an argument and checks if the edges are right formatted and matches the pattern
 * ^(0|([1-9][0-9]*))-(0|([1-9][0-9]*))$. If there's an invalid argument or the few arguments, an error message is printed and the usage function is called.
 * @param argc - number of arguments
 * @param argv - values of the arguments
 */
static void parse_arguments(int argc, char *argv[]) {
    //check if at least one edge is given
    if(argc < 2) {
        fprintf(stderr, "[%s] ERROR: Too few arguments\n", argv[0]);
        usage(argv[0]);
    }

    //check edges format with pattern matching
    int status;
    regex_t reg;
    //compile pattern
    if(regcomp(&reg, "^(0|([1-9][0-9]*))-(0|([1-9][0-9]*))$", REG_EXTENDED|REG_NOSUB) != 0) {
        fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }
    for(int i=1; i < argc; i++) {
        //check if edge matches pattern
        status = regexec(&reg, argv[i], 0, NULL, 0);
        if(status != 0) {
            //no match
            regfree(&reg);
            fprintf(stderr, "[%s] ERROR: %s is not an edge\n", argv[0], argv[i]);
            usage(argv[0]);
        }
    }
    //free space of reg
    regfree(&reg);
}



/**
 * @brief this function gets the index of the node in the nodes array
 * @details this function iterates through the array of nodes. If the node is in the array (node == nodes[i]), then its index is returned
 * and the function is stopped. Otherwise the integer -1 is returned, which symbolizes, that the node is not in the array
 * @param node - node which is index is looked for
 * @param nodes - array of nodes
 * @param size - number of nodes in nodes array
 * @return int - returns the index of the node if it's found, otherwise -1 is returned
 */
static int get_index(int node, int nodes[], int size) {
    for(int i=0; i < size; i++) {
        //node found
        if(nodes[i] == node) {
            return i;
        }
    }
    //node not in array
    return -1;
}



/**
 * @brief this function prints the edges of a solution
 * @details this function prints the edges of the given solution. It iterates through all edges[2] and prints them in the following pattern: 1-1.
 * @param s - instance of a struct solution, which is printed
 */
static void print_edges(struct solution s) {
    //iterate through edges
    for(int i=0; i < s.size; i++) {
        fprintf(stdout, "%d-%d ", s.edges[i][0], s.edges[i][1]);
    }
    fprintf(stdout, "\n");
}


/**
 * @brief entry point of the program generator
 * @details this function is the entry point of the program generator. It calls the function parse_arguments.
 * It creates a two-dimensional array of the program arguments which represents the edges of a graph. It then creates an int array 
 * which represents the nodes of the graph. If no error occured, the circularbuffer is openend by calling cb_coo.
 * It's main task is to create solutions of the 3coloring problem of the graph with a monte-carlo (random) algorithmn.
 * This algorithmn assigns random colors to the nodes and removes every edge, where the nodes have the same color.
 * If a solution with MAX_SIZE_SOLUTION or less is found, the generator writes the solution into the circularbuffer.
 * It repeats that task until the supervisor stops it by setting the state of the circularbuffer to 1.
 * @param argc - number of arguments of the program
 * @param argv - value of arguments of the program
 * @return int - exit code, 0 = everythings okay, != 0 = failure
 */
int main(int argc, char *argv[]){
    //parse arguments of the program
    parse_arguments(argc, argv);

    //set number of edges, edges array and nodes array
    int edges_count = argc - 1;
    int edges[edges_count][2];
    int nodes[edges_count*2]; //max number of nodes

    //fill nodes array an edges array
    int nodes_count = 0;
    char *ptr;
    int node;
    char *succ;
    //iterate through edes array
    for(int i=1; i <= edges_count; i++) {
        //split the string NODE1-NODE2 on "-" so that ptr is NODE1
        ptr = strtok(argv[i], "-");
        //make string to int
        node = strtol(ptr, &succ, 10);
        //set edge
        edges[i-1][0] = node;
        //add node to nodes array if not already in it
        if(get_index(node, nodes, nodes_count) < 0) {
            nodes[nodes_count++] = node;
        }
        //get the second part of the splitted string, so that ptr is NODE2
        ptr = strtok(NULL, "-");
        //make string to int
        node = strtol(ptr, &succ, 10);
        //set edge
        edges[i-1][1] = node;
        //add node to nodes array if not already in it
        if(get_index(node, nodes, nodes_count) < 0) {
            nodes[nodes_count++] = node;
        }
    }

    //open circular buffer
    struct circularbuffer *circbuf = cb_coo('g');
    if(circbuf == NULL) {
        fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
        if(errno == EEXIST) {
            fprintf(stderr, "[%s] HINT: Try 'rm -rf /dev/shm/*12019852*' for resolving the error\n", argv[0]);
        }
        exit(EXIT_FAILURE);
    }

    //monte-carlo-algorithmus
    char colors[] = {'r', 'g', 'b'};
    char color_nodes[nodes_count];
    int rand_int;
    int idx1;
    int idx2;
    int sol_size = 0;
    struct solution sol;
    struct timeval current_time;
    while(circbuf->shm->state == 0) {
        //assign random colors
        gettimeofday(&current_time, NULL);
        srand(current_time.tv_usec + getpid());
        for(int i=0; i < nodes_count; i++) {
            rand_int = rand() % 3;
            color_nodes[i] = colors[rand_int];
        }

        //check if two nodes of an edge have the same color
        sol_size = 0;
        for(int i=0; i < edges_count; i++) {
            idx1 = get_index(edges[i][0], nodes, nodes_count);
            idx2 = get_index(edges[i][1], nodes, nodes_count);
            if(idx1 == -1 || idx2 == -1) {
                 assert(0);
            }
            if(color_nodes[idx1] == color_nodes[idx2]) {
                 //add invalid edge to solution
                sol_size++;
                if(sol_size > MAX_SIZE_SOLUTION) {
                    break;
                }
                sol.edges[sol_size-1][0] = edges[i][0];
                sol.edges[sol_size-1][1] = edges[i][1];
            }
        }
        //check if number of removed edges is smaller than the MAX_SIZE_SOLUTION
        //only then the solution is written to the circularbuffer
        if(sol_size <= MAX_SIZE_SOLUTION) {
            sol.size = sol_size;
            fprintf(stdout, "[%s: %d] Solution with %d edges found: ", argv[0], getpid(), sol.size);
            print_edges(sol);
            int return_code = cb_write(circbuf, sol);
            if(return_code == -1) {
                fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
                //close the circularbuffer
                if(cb_close(circbuf, 'g') != 0) {
                    fprintf(stderr, "[%s] ERROR: %s\n", argv[0], strerror(errno));
                }
                return EXIT_FAILURE;
            }
            if(return_code == -2) {
                //the state of the circularbuffer was set to 1 -> the circularbuffer was closed
                return EXIT_SUCCESS;
            }
        }
    }

    return EXIT_SUCCESS;
}