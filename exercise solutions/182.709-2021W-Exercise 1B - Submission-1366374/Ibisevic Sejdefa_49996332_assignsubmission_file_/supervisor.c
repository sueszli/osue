/**
 * @file supervisor.c
 * @author Sejdefa Ibisevic <e11913116@student.tuwien.ac.at>
 * @brief The supervisor is called to analyze the circular buffer content
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 * @details 
 * 
 */

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>

#include "circBuff.h"

static char *myprogram_sup;

/**
 * @brief Usage function called in case of wrong input, decsribes correct input parameters
 * @param none
 * @return none
 */
static void usage (void);
/**
 * @brief Signal handler function that sets quit to 1
 * 
 * @param signal 
 */
static void handle_signal(int signal);
/**
 * @brief Signal processing function
 * @param none
 * @return none
 */
void process_signal(void);
/**
 * @brief Function that counts number of edges provided in data parameter
 * 
 * @details Function accepts data pointer to the input data containing edges separated by '-' sign.
 *          The character c represents '-' and function counts number of provided edges in u-v form.
 * @param data - pointer to array of characters that contains all input edges
 * @param c - character '-' that represents separation between vertices of edge
 * @return edges - returns number of edges in data array
 */
int count_edges(const char *data, const char c);

volatile sig_atomic_t quit = 0;

static void usage (void) {
    fprintf(stderr, "[%s] Usage: %s\n", myprogram_sup, myprogram_sup);
    exit(EXIT_FAILURE);
}

static void handle_signal(int signal) { 
    quit = 1; 
}

void process_signal(void)
{
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
}

int count_edges(const char *data, const char c) {

    int edges = 0;

    for (int i = 0; data[i]; i++)
    {
        if (data[i] == c)
        {
            edges++;
        }
    }
    return edges;
}

int main(int argc, char **argv) {

    myprogram_sup = argv[0];

    if (argc != 1) usage();    //supervisor takes no arguments

    process_signal();

    char *buff_data;

    //create new buffer
    buffer *buff = open_buff(1);

    if (buff == NULL)
    {
        fprintf(stderr, "[%s] Error: buffer open failed: %s\n", myprogram_sup, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int min = MAX_EDGE + 1;

    while(!quit){

        buff_data = read_buff(buff);
        
        if (buff_data == NULL) break;

        int edges_num = count_edges(buff_data, '-');

        if (edges_num == 0) {
            printf("[%s] The graph is acyclic!\n", myprogram_sup);
            free(buff_data);
            break;
        }

        //Solution with less edges found
        if(edges_num < min) {
            min = edges_num;
            printf("[%s] Solution with %d edges: %s\n", myprogram_sup, edges_num, buff_data);
        }

        free(buff_data);
    }
    
    
    if (close_buff(buff, 1) == -1)
    {
        fprintf(stderr, "[%s] Error: buffer closing failed: %s\n", myprogram_sup, strerror(errno));
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
