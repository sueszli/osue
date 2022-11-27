#include <stdlib.h>
#include <string.h>

#define SHM_NAME "fb_arc_set"
#define MAX_EDGE_NUMBER (20)

#define SEM_SUPERVISOR_STARTS "/sem_supervisor_starts"
#define SEM_ALT_G_C_1 "/sem_alt_g_c_1"
#define SEM_ALT_G_C_2 "/sem_alt_g_c_2"

struct edge {
    unsigned int start_node;
    unsigned int end_node;
};

typedef struct edge edge_t;

/**
 * @brief struct for the shared memory
*/
struct fb_shm {
    unsigned int state;
    edge_t feedback_arcs[MAX_EDGE_NUMBER];
};

typedef struct fb_shm fb_shm_t;

/**
 * @brief Program name as a static string variable
 */
static char *program_name;

/**
 * @brief Variable that saves the best feedback arc
 */
edge_t best_feedback_arc[0];
/**
 * @brief Variable that saves the best feedback arc
 */
edge_t *current_graph;
int current_graph_size;

/**
 * @brief Program name as a static string variable
 */
static char *program_name;

edge_t* string_to_graph(edge_t graph[]);
void append_string_edge_to_graph(edge_t graph[], char* s, int index);
void print_graph(edge_t graph[], int graph_size);
int* randomPermutation(int size);
int get_index_and_shift(int array[], int index, int size);
int get_highest_edge(edge_t graph[], int graph_size);

int main(int argc, char *argv[]) {
    printf("generator\n");

    program_name = argv[0];

    //generate semaphore
    //sem_generator = Init(1)

    //TODO: Error handling wrong output

    current_graph_size = (argc - 1);
    current_graph = malloc(sizeof(edge_t) * (argc - 1));

    for (int i = 1; i < argc; i++)
    {
        append_string_edge_to_graph(current_graph, argv[i], i-1);
    }

    int rand_per_size = get_highest_edge(current_graph, current_graph_size)+1;
    int* rand_per = randomPermutation(rand_per_size);

    for(int i = 0; i < rand_per_size; i++)
    {
        printf("Permutation %i\n",rand_per[i]);
    }

    edge_t *fed_arc = malloc(sizeof(edge_t) * (argc - 1));

    //TODO: Dealloc
    print_graph(current_graph, current_graph_size);

    exit (EXIT_SUCCESS);
}

void append_string_edge_to_graph(edge_t graph[], char* s, int index){
    //The graph has one more edge
    int seperator_index = 0;

    //edge_t return_graph[size_of_old_graph + 1];

    for(int i = 0; s[i] != '\0'; i++){
        if(s[i] == '-'){
            s[i] = '\0';
            seperator_index = i;
        }
    }

    graph[index].start_node = atoi(s);
    graph[index].end_node = atoi(s+seperator_index+1);
}

void print_graph(edge_t graph[], int graph_size)
{
    char* s_return = "";
    for(int i = 0; i < graph_size; i++)
    {
        printf("%i-%i ", graph[i].start_node, graph[i].end_node);
        //strcat(s_return, "atoi(graph[i].start_node)");
        //strcat(s_return, "-");
        //strcat(s_return, "atoi(graph[i].end_node)");
        //strcat(s_return, " ");
    }
    printf("\n");
}

int* randomPermutation(int size)
{
    int return_permutation[size];
    int num_inc[size];

    for(int i = 0; i < size; i++)
    {
        num_inc[i] = i;
    }

    for(int i = 0; i < size; i++)
    {
        return_permutation[i] = get_index_and_shift(num_inc, rand()%(size-i), size);
    }

    return return_permutation;
}

int get_index_and_shift(int array[], int index, int size)
{
    int i_return = array[index];

    for(int i = index; i < size - 1; i++)
    {
        array[i] = array[i+1];
    }

    return i_return;
}

int get_highest_edge(edge_t graph[], int graph_size)
{
    unsigned int i_return = 0;

    for (int i = 0; i < graph_size; i++)
    {
        if (graph[i].start_node > i_return)
        {
            i_return = graph[i].start_node;
        }

        if (graph[i].end_node > i_return)
        {
            i_return = graph[i].end_node;
        }
    }

    return i_return;
}

void generate_feedback_arc_set(edge_t *fed_arc, int* rand_per)
{
    
}