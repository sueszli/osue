#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <regex.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include "shared_mem.h"
#include "circ_buff.h" 

const char *myprog; /** The program name.*/
volatile sig_atomic_t quit = 0;
void handle_signal(int signal) { quit = 1; }

static int match_regexp(char *input)
{
    int    status;
    regex_t    re;

    if (regcomp(&re, "[0-9]+-[0-9]+$", REG_EXTENDED|REG_NOSUB) != 0) {
        return -1;      /* Report error. */
    }
    status = regexec(&re, input, (size_t) 0, NULL, 0);
    regfree(&re);
    if (status != 0) {
        return -1;      /* Report error. */
    }
    return 0;
}

static int is_input_valid(int argc, char *argv[])
{
    if (argc <= 2)
	{
        fprintf(stderr, "%s: At least two edges are necessary\n", myprog);
		return -1;
    }

    for(int i=1; i < argc; i++)
    {
        if(match_regexp(argv[i]) == -1)
        {
            fprintf(stderr, "%s: Input does not match regexp: [0-9]+-[0-9]+$\n", myprog);   
            return -1;
        }
    }
    return 0;
}

static bool node_ind_exists(struct node *nodes, int n, int nodes_size)
{
    for(int i = 0; i < nodes_size; i++)
    {
        if (nodes[i].node_ind == n){
            return true;
        }
    }
    return false;
}

static int parse_input(int argc, char *argv[], struct graph *graph)
{
    graph->nodes_size = 1;

    for(int i = 0; i + 1 < argc; i++) 
    {
        char *token = strtok(argv[i + 1], "-");
        int n1 = strtol(token, NULL, 10);

        if(!(node_ind_exists(graph->nodes, n1, graph->nodes_size)))
        {
            graph->nodes[graph->nodes_size - 1].node_ind = n1;
            graph->nodes[graph->nodes_size - 1].color = -1;
            graph->nodes_size++;
            graph->nodes = realloc(graph->nodes, sizeof(struct node) * (graph->nodes_size));
            if (graph->nodes == NULL)
            {
                fprintf(stderr, "%s: realloc failed: %s\n", myprog, strerror(errno));
                return 1;
            }
        }

        graph->edges[i].node_ind_1 = n1;
        token = strtok(NULL, "-");
        int n2 = strtol(token, NULL, 10);

        if(!(node_ind_exists(graph->nodes, n2, graph->nodes_size)))
        {
            graph->nodes[graph->nodes_size - 1].node_ind = n2;
            graph->nodes[graph->nodes_size - 1].color = -1;
            graph->nodes_size++;
            graph->nodes = realloc(graph->nodes, sizeof(struct node) * (graph->nodes_size));
            if (graph->nodes == NULL)
            {
                fprintf(stderr, "%s: realloc failed: %s\n", myprog, strerror(errno));
                return 1;
            }
        }
        graph->edges[i].node_ind_2 = n2;
    }
    return 0;
}

static void paint_nodes(struct graph *graph)
{
    for(int i = 0; i < graph->nodes_size; i++)
    {
        graph->nodes[i].color = rand() % 3;
    }
}

int main(int argc, char *argv[])
{
    myprog = argv[0];
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (is_input_valid(argc, argv) == -1)
    {
        fprintf(stderr, "%s: Input not valid\n", myprog); 
        exit(EXIT_FAILURE);
    }

    struct graph *graph = malloc(sizeof(struct graph));
    graph->edges = malloc(sizeof(struct edge) * (argc - 1));
    graph->nodes = malloc(sizeof(struct node));
    if (parse_input(argc, argv, graph) == -1)
    {
        fprintf(stderr, "%s: Input not valid\n", myprog); 
        exit(EXIT_FAILURE);
    }
    
    for(int i = 0; i < (argc - 1); i++)
    {
        printf("Edge Index: %i-%i\n", graph->edges[i].node_ind_1, graph->edges[i].node_ind_2);
    }

    time_t t;
    struct circ_buff *buffer = create_buff(false);
    srand(((unsigned) time(&t)) * getpid());
    paint_nodes(graph);
    for(int i = 0; i < graph->nodes_size; i++)
    {
        printf("Node Index: %i\n", graph->nodes[i].node_ind);
        printf("Node Color: %i\n", graph->nodes[i].color);
    }

    char input[40];
    printf("Insert an input:\n");
    scanf("%s", input);
    while((quit == 0) && (buffer->sv_stopped == false))
    {
        write_circ_buff(buffer, input);
    }
    free(graph->nodes);
    free(graph->edges);
    free(graph);
    close_buff(false, buffer);
    return 0;
}