/**
 * @file        generatorfunctions.c
 * @author:     Atheer ELobadi
 * @date:       22.11.2020
 * @brief:      contains the helping functions which are needed be the generator to find feedback set arc
 */

#include "generatorfunctions.h"

void add_string_edge_to_graph(char *edge, graph_t *graph)
{
    uint32_t u = 0;
    uint32_t v = 0;
    parse_vertices(edge, &u, &v);

    graph->edge[graph->top].u = u;
    graph->edge[graph->top].v = v;

    graph->top++;
}

void init_graph(graph_t *graph)
{
    graph->top = 0;
    graph->edge = malloc(sizeof(edge_t) * graph->size);
    if (graph->edge == NULL)
    {
        fprintf(stderr, "[%s:%d]: Error allocating memory. %s\n",
                __FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < graph->size; i++)
    {
        graph->edge[i].u = 0;
        graph->edge[i].v = 0;
    }
}

void print_graph(graph_t graph)
{
    printf("--------------------------------\n");
    for (int i = 0; i < graph.size; i++)
    {
        printf("%d - %d\n", graph.edge[i].u, graph.edge[i].v);
    }
    printf("--------------------------------\n");
}

void parse_vertices(char *edge, uint32_t *u, uint32_t *v)
{
    char *endptr;

    *u = strtol(edge, &endptr, 10);
    while (*edge != *endptr)
    {
        edge++;
    }
    edge++;
    *v = strtol(edge, '\0', 10);
}

void random_peremutation(uint32_t *rndP, unsigned int size)
{
    for (unsigned int i = 0; i < size; i++)
        rndP[i] = i;

    unsigned int temp, d;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec * getpid());
    // srand(time(NULL));
    for (unsigned int i = 0; i < size; i++)
    {
        d = (rand() % size);
        temp = rndP[d];
        rndP[d] = rndP[i];
        rndP[i] = temp;
    }
}

int compare_order_of_u_and_v(uint32_t *rndP, int number_of_vertices, edge_t edge)
{
    for (int i = 0; i < number_of_vertices; i++)
    {
        if (rndP[i] == edge.u)
        {
            return 1; // u before v
        }
        if (rndP[i] == edge.v)
        {
            return 0; // v before u
        }
    }
    return -1;
}

int get_number_of_vertices(graph_t graph)
{
    int max = 0;
    for (int i = 0; i < graph.size; i++)
    {
        if (graph.edge[i].u > max)
        {
            max = graph.edge[i].u;
        }
        if (graph.edge[i].v > max)
        {
            max = graph.edge[i].v;
        }
    }
    return ++max;
}

set_t *get_feedback_arc_set(graph_t graph)
{
    int number_of_vertices = get_number_of_vertices(graph);

    uint32_t *rndP = malloc(sizeof(uint32_t) * number_of_vertices);
    if (rndP == NULL)
    {
        fprintf(stderr, "[%s:%d]: Error allocating memory. %s\n",
                __FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    random_peremutation(rndP, number_of_vertices);

    graph_t *temp_solution = malloc(sizeof(graph_t));
    if (temp_solution == NULL)
    {
        fprintf(stderr, "[%s:%d]: Error allocating memory. %s\n",
                __FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    temp_solution->edge = malloc(sizeof(edge_t));
    if (temp_solution->edge == NULL)
    {
        fprintf(stderr, "[%s:%d]: Error allocating memory. %s\n",
                __FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    temp_solution->size = -1;

    for (int i = 0; i < graph.size; i++)
    {
        if (compare_order_of_u_and_v(rndP, number_of_vertices, graph.edge[i]) == 0)
        {
            temp_solution->edge = realloc(temp_solution->edge, sizeof(edge_t) * (temp_solution->size + 1));
            // add edge to temp_solution
            temp_solution->edge[temp_solution->size] = graph.edge[i];
            ++temp_solution->size;
        }
    }
    set_t *solution;
    if ((solution = malloc(sizeof(set_t))) == NULL)
    {
        fprintf(stderr, "[%s:%d]: Error allocating memory. %s\n",
                __FILE__, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (temp_solution->size < MAX_SET_SIZE)
    {
        solution->size = temp_solution->size;
        for (int i = 0; i < solution->size; i++)
        {
            solution->edge[i] = temp_solution->edge[i];
        }
    }
    else
    {
        // make sure the size is not acceptable
        solution->size = MAX_SET_SIZE + 1;
    }

    //free(temp_solution);
    free(rndP);
    return solution;
}

void usage(char *program_name)
{
    printf(USAGE, program_name);
}