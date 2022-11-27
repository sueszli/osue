#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <time.h>
#include "error.h"

static char *prg_name;
typedef struct graph
{
    unsigned int v_top;
    unsigned int v_cap;
    unsigned int e_top;
    unsigned int e_cap;
    int *vertices;
    int *edges; // saves one edge: [from1, to1, from2, to2, ...]
} graph_t;

static void get_vertices_from_str(int *from, int *to, char *str);
static int graph_contains_vertex(graph_t *g, int vertex);
static void insert_vertex(graph_t *g, int vertex);
static int is_valid_argument(char *arg);
static void argument_handler(int argc, char **argv, graph_t *g);
static void insert_edge(graph_t *g, int from, int to);
static void shuffle_vertices(graph_t *g);
static int random_int(int min, int max);
static void swap_vertices(graph_t *g, int i, int j);
static void print_vertecies(graph_t *g);
static void print_edges(graph_t *g);
static void gen_arcset(graph_t *input, graph_t *output);
static int index_of_vertex(graph_t *g, int vertex);
static void usage(void);

int main(int argc, char **argv)
{
    srand(time(0));

    prg_name = argv[0];
    graph_t graph = {0, 0, 0, 0, NULL, NULL};
    graph_t arc_set = {0, 0, 0, 0, NULL, NULL};

    argument_handler(argc, argv, &graph);
    gen_arcset(&graph, &arc_set);
    print_edges(&arc_set);
    return 0;
}

/**
 * @brief checks if all input arguments are valid and puts them into data structure
 * 
 * @param argc argument counter
 * @param argv argument vector
 */
static void argument_handler(int argc, char **argv, graph_t *g)
{
    if (argc - optind < 1)
        usage();

    int i;
    for (i = optind; i < argc; i++)
    {
        int from, to;
        get_vertices_from_str(&from, &to, argv[i]);

        insert_vertex(g, from);
        insert_vertex(g, to);
        insert_edge(g, from, to);
    }
}

/**
 * @brief generates a heuristic arcset from input and saves it into output
 * 
 * @param input input graph
 * @param output output graph (heuristic arcset)
 */
static void gen_arcset(graph_t *input, graph_t *output)
{
    int i, from, to;
    shuffle_vertices(input);
    for (i = 0; i < input->e_top; i++)
    {
        from = input->edges[i];
        to = input->edges[++i];

        if (index_of_vertex(input, from) > index_of_vertex(input, to))
        {
            insert_vertex(output, from);
            insert_vertex(output, to);
            insert_edge(output, from, to);
        }
    }
}

/**
 * @brief gets index of vertex in graph -> vertices
 * 
 * @param g graph
 * @param vertex vertex
 * @return int returns the index of vertex in graph.vertices, if vertex does not exist in vertices set returns -1
 */
static int index_of_vertex(graph_t *g, int vertex)
{
    int i;
    for (i = 0; i < g->v_top; i++)
    {
        if (g->vertices[i] == vertex)
            return i;
    }
    return -1;
}

/**
 * @brief Takes an argument string (eg. "1-3") and extracts the vertecies. 
 * This functions throws an error if the sting is an illigal argument.
 * Saves the result in from and to (eg. from = 1, to = 3)
 * 
 * @param from vertex from
 * @param to vertex to
 * @param str input argument [0..n]-[0..m] n,m are natural numbers
 */
static void get_vertices_from_str(int *from, int *to, char *str)
{
    if (!is_valid_argument(str))
        ERROR_MSG("illigal argument format", prg_name);

    int index = 0;
    for (; *(str + index) != '\0'; index++)
    {
        if (*(str + index) == '-')
        {
            *(str + index) = '\0';
            *from = atoi(str);
            *to = atoi((str + index) + 1);
        }
    }
}

/**
 * @brief shuffles all vertices in graph randomly
 * 
 * @param g graph
 */
static void shuffle_vertices(graph_t *g)
{
    int i, r;

    for (i = g->v_top - 1; i >= 1; i--)
    {
        r = random_int(0, i);
        swap_vertices(g, i, r);
    }
}

/**
 * @brief prints all vertecies in graph g
 * ( for debugging ) 
 * @param g graph
 */
static void print_vertecies(graph_t *g)
{
    int *p;

    for (p = g->vertices; p < g->vertices + g->v_top; ++p)
        printf("%d, ", *p);
    printf("\n");
}

/**
 * @brief prints all edges in graph g
 * ( for debugging ) 
 * @param g graph
 */
static void print_edges(graph_t *g)
{
    int i, from, to;

    for (i = 0; i < g->e_top; i++)
    {

        from = g->edges[i];
        to = g->edges[++i];

        printf("%d-%d ", from, to);
    }
    printf("\n");
}

/**
 * @brief swaps position of two vertices in graph
 * 
 * @param g graph
 * @param i vertex1 index
 * @param j vertex2 index
 */
static void swap_vertices(graph_t *g, int i, int j)
{
    if (i < 0 || j < 0 || i >= g->v_top || j >= g->v_top)
        ERROR_MSG("index out of bound", prg_name);

    int tmp = g->vertices[i];
    g->vertices[i] = g->vertices[j];
    g->vertices[j] = tmp;
}

static int random_int(int min, int max)
{
    return (rand() % (max - min + 1)) + min;
}

/**
 * @brief checks if a graph already contains the vertex
 * 
 * @param g graph
 * @param vertex vertex to check
 * @return int returns 1 if the graph already contains the vertex, returns 0 if not
 */
static int graph_contains_vertex(graph_t *g, int vertex)
{
    int i;
    for (i = 0; i < g->v_top; i++)
    {
        if (g->vertices[i] == vertex)
            return 1;
    }
    return 0;
}

/**
 * @brief Insert a vertex in the graphs vertex set (no duplicates)
 * 
 * @param g graph
 * @param vertex vertex to add
 */
static void insert_vertex(graph_t *g, int vertex)
{
    if (!graph_contains_vertex(g, vertex))
    {
        if (g->v_top == g->v_cap)
        {
            int newcap = g->v_cap + 10;
            int *newptr = realloc(g->vertices, sizeof(int) * newcap);
            if (newptr == NULL)
                ERROR_MSG("realloc error", prg_name);
            g->vertices = newptr;
            g->v_cap = newcap;
        }
        g->vertices[g->v_top++] = vertex;
    }
}

/**
 * @brief insertes a edge into a graph
 * 
 * @param g graph
 * @param from vertex from
 * @param to vertex to
 */
static void insert_edge(graph_t *g, int from, int to)
{
    if (g->e_top == g->e_cap)
    {
        int newcap = g->e_cap + 10;
        int *newptr = realloc(g->edges, sizeof(int) * newcap);
        if (newptr == NULL)
            ERROR_MSG("realloc error", prg_name);
        g->edges = newptr;
        g->e_cap = newcap;
    }
    g->edges[g->e_top++] = from;
    g->edges[g->e_top++] = to;
}

/**
 * @brief checks if string matches [0..n]-[0..m]
 *  examples: 1-2 3-4
 * 
 * @param arg 
 * @return int returns 1 for match and 0 for no match
 */
static int is_valid_argument(char *arg)
{
    regex_t regex;
    int reti;

    /* Compile regular expression */
    reti = regcomp(&regex, "^[0-9][0-9]*-[0-9][0-9]*$", 0);
    if (reti)
    {
        ERROR_MSG("Regex compile error", prg_name);
        exit(EXIT_FAILURE);
    }

    reti = regexec(&regex, arg, 0, NULL, 0);
    return !reti;
}

/**
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void)
{
    (void)fprintf(stderr, "USAGE: %s EDGE1...\n", prg_name);

    exit(EXIT_FAILURE);
}
