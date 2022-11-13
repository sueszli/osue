/**
 * @file generator.c
 * @author Michael Bumbar <e11775807@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Main program module of the generator process.
 * 
 * This program receives a arbitrary number of edges through stdin and opens a already created shared memory and POSIX semaphores. As long as a supervisor process is running it randomly creates feedback arc sets
 * that are written to the circular buffer. If the supervisor terminates this program will close its shared memory and semaphores and terminate.
 **/

#include "circularbuffer.h"

extern circbuffer* buffer; /**< The name of the circular buffer. It is declared in circularbuffer.h. */



/**
 * Checks wether a String constitutes an edge and if so saves it in result.
 * @brief The program receives a char pointer and an pointer to an edge. If the contents of the String correspond to the edge pattern, the edge is saved in the second argument.
 * @details This function calls sscanf and parses c with a pattern. The first two numbers in c are saved in variables. If sscanf returns less than to numbers the string did not correspond
 * to the pattern of an edge and errorHandling is called. The program exits with EXIT_FAILURE.
 * Otherwise the program saves the first number in result->start and the second one in result->end and returns with EXIT_SUCCESS.
 * constants: GEN
 * @param c A string to be parsed
 * @param result A pointer to the resulting edge.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t get_edge(char* c, edge* result);


/**
 * Creates a graph from the command line input.
 * @brief Iterates over the elements of the command line and creates edges which are saved in an array. Additionally the start points and end points of these edges are saved in an array vertices. No duplicates are saved.
 * @details This function starts by iterating over the input creating edges from the strings by calling get_edge. Afterwards it iterates over vertices and edges to check if the edge and its vertices
 * have already been saved. If not they are saved to the arrays.
 * After the function finishes iterating over all inputs we free the space and return.
 * constants: GEN
 * @param argv The argument vector.
 * @param argc The argument counter.
 * @param arg The index of the first positional argument.
 * @param vertices The vertices of the input graph.
 * @param last_vertex The number of elements in vertices.
 * @param edges The edges of the input graph.
 * @param last_edge The number of elements in edges.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t get_graph(char** argv, int argc, int32_t arg, int32_t* vertices, int32_t* last_vertex, edge* edges, int32_t* last_edge);


/**
 * Creates a random permutation of an int32_t array.
 * @brief This function is an implementation of the Fisher Yates shuffle it creates a random permutation of an int32_t array.
 * @details This function starts by iterating over elements of vertices starting with the last and finishing with the second. For every element a random index in the array is generated and the element
 * is swapped with the one on the created index. 
 * @param vertices The input file from which the program reads.
 * @param last_vertex The number of consumed spaces in vertices. last_vertex -1 is the index of the last element.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t permutation(int32_t *vertices,int32_t last_vertex);


/**
 * Generates a random feedback arc set.
 * @brief Removes edges from the input graph to create a topological ordering. The removed edges are saved as a feedback arc set.
 * @details This function starts by iterating over the edges. For every edge it then iterates again over the vertices of the input. Postion of the starting point of an edge in the vertices array and the
 * position of the ending point are saved. If the start point is greater than the end point then the edge is added to the result and the result length is incremented. If the result length is greater 
 * than 8 the function returns -1.
 * Otherwise it returns EXIT_SUCCESS.  
 * @param s The arc_set used to store the solution.
 * @param last_edge The number of elements in edges.
 * @param last_vertex The number of elements in vertices.
 * @param vertices The vertices of the input graph.
 * @param edges The edges of the input graph.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t calculate_solution(arc_set* s,int32_t last_edge, int32_t last_vertex, int32_t* vertices, edge* edges);


/**
 * Frees resources.
 * @brief Closes the allocated memory and the shared memory.
 * @details This function calls free for the allocated memory in result, edges and vertices. Afterwards it calls free_loaded_buffer to close the shared memory and the POSIX semaphores.
 * @param result Used to temporarily store solutions.
 * @param edges The edges of the input graph.
 * @param vertices The vertices of the input graph.
 * @return Returns EXIT_SUCCESS.
 */
static int32_t free_res(arc_set* result, edge* edges, int32_t* vertices);


/**
 * Program entry point.
 * @brief The program starts here. This function calls functions to open the shared memory object and runs as long a supervisor process is active. It generates random feedback arc sets and writes
 * them to the circular buffer if theire length is smaller than 8. If no supervisor is active the generator frees all allocated resources and closes the shared memory.
 * @details This function calls getopt and parses the command line arguments. If an option is received a the usage function is called. If the function received no positional arguments it writes a solution
 * with length 0 to the circular buffer and terminates with EXIT_SUCCESS. 
 * If not the function opens the shared memory and the POSIX semaphores and allocates memory vor vertices and edges with length argc. Afterwards it calls get_graph to save the input to the previously created
 * arrays.
 * As long as a supervisor is active the generator will call permutaition to shuffle the vertices in place, calculate_solution to generate a feedback arc set and write, to write the solution to the
 * circular buffer if warranted. 
 * If there is no active supervisor the generator calls free_res to free all allocated memory and close the shared memory and semaphores. It then returns EXIT_SUCCESS.
 * constants: GEN
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char** argv){
    int buffer_state = load_buffer();
    if(buffer_state == -1){
        return EXIT_SUCCESS;
    } else if(buffer_state == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    int32_t pos_arg = 0;
    int32_t* vertices;
    edge* edges;
    arc_set* result;
    int32_t last_vertex = 0; //number of elements in vertices
    int32_t last_edge = 0; //number of elements in edges
    srand(getpid());
    int c;
    while((c = getopt(argc,argv,"")) != -1){
        switch (c)
        {
        case '?': usage("generator EDGE1 ...", SUP); break; 
        default:  usage("generator EDGE1 ...", SUP); break; 
            break;
        }
    }
    pos_arg = optind;
    vertices = malloc(sizeof(int32_t)*(argc));
    if(vertices == NULL){
        free_loaded_buffer();
        errorHandling(GEN,"malloc failed",strerror(errno));
    }
    edges = malloc(sizeof(edge)*(argc));
    if(edges == NULL){
        free_loaded_buffer();
        free(vertices);
        errorHandling(GEN,"malloc failed",strerror(errno));
    }
    result = malloc(sizeof(arc_set));
    if(result == NULL){
        free_loaded_buffer();
        free(vertices);
        free(edges);
        errorHandling(GEN,"malloc failed",strerror(errno));
    }
    if((argc) == 1){
        result->length = 0;
        if(write_buffer(*result) == EXIT_FAILURE){
            free_res(result,edges,vertices);
            errorHandling(GEN,"write_buffer failed",strerror(errno));
        }
        free_res(result,edges,vertices);
        return EXIT_SUCCESS;
    }
    if(get_graph(argv,argc,pos_arg,vertices,&last_vertex,edges,&last_edge) == EXIT_FAILURE){
            free_res(result,edges,vertices);
            errorHandling(GEN,"get_graph failed",strerror(errno));
    }
    while(buffer->running == ACTIVE){
        if(permutation(vertices,last_vertex) == EXIT_FAILURE){
            free_res(result,edges,vertices);
            errorHandling(GEN,"permutation failed",strerror(errno));
        }
        int solution_length;
        if((solution_length = calculate_solution(result,last_edge,last_vertex,vertices,edges)) == EXIT_FAILURE){
            free_res(result,edges,vertices);
            errorHandling(GEN,"calculate_solution failed",strerror(errno));
        } else if(solution_length == -1){
            continue;
        }
        if(write_buffer(*result) == EXIT_FAILURE){
            free_res(result,edges,vertices);
            errorHandling(GEN,"write_buffer failed",strerror(errno));
        }
    }
    free_res(result,edges,vertices);
    return EXIT_SUCCESS;
}


static int32_t get_edge(char* c, edge* result){
    if(sscanf(c,"%d-%d",&(result->start),&(result->end)) < 2){
        errorHandling(GEN, "get_edge failed", "the format of one or more edges was incorrect");
    }
    return EXIT_SUCCESS;
}


static int32_t get_graph(char** argv, int argc, int32_t arg, int32_t* vertices, int32_t* last_vertex, edge* edges, int32_t* last_edge){
    edge tmp = {.start = 0, .end = 0};
    for(int i = arg; i < argc; i++){
        if(get_edge(argv[i],&tmp) == EXIT_FAILURE){
            errorHandling(GEN,"getedge failed",strerror(errno));
        }
        int j = 0;
        for(;j <= *last_vertex; j++){
            if(vertices[j] == tmp.start){
                break;
            }
        }
        if(*last_vertex == 0){
            vertices[*last_vertex] = tmp.start;
            *last_vertex = *last_vertex+1;
        } else if(j > *last_vertex-1){
            vertices[*last_vertex] = tmp.start;
            *last_vertex = *last_vertex+1;
        }
        j = 0;
        for(;j <= *last_vertex; j++){
            if(vertices[j] == tmp.end){
                break;
            }
        }
        if(*last_vertex == 0){
            vertices[*last_vertex] = tmp.end;
            *last_vertex = *last_vertex+1;
        } else if(j > *last_vertex-1){
            vertices[*last_vertex] = tmp.end;
            *last_vertex = *last_vertex+1;
        }
        j = 0;
        for(;j <= *last_edge;j++){
            if(cmp_edge(&tmp, &edges[j]) == 0){
                break;
            }
        }
        if(*last_edge == 0){
            (edges[*last_edge]).end = tmp.end;
            (edges[*last_edge]).start = tmp.start;
             *last_edge = *last_edge+1;
        } else if(j > *last_edge-1){
            (edges[*last_edge]).end = tmp.end;
            (edges[*last_edge]).start = tmp.start;
             *last_edge = *last_edge+1;
        }
    }
    return EXIT_SUCCESS;
}


static int32_t permutation(int32_t *vertices,int32_t last_vertex){
    int32_t swap;
    int32_t tmp;
     for (int i = last_vertex-1; i > 0; i--) {
         tmp = rand() % (i + 1);
         swap = vertices[tmp];
         vertices[tmp] = vertices[i];
         vertices[i] = swap;
     }
     return EXIT_SUCCESS;
}


static int32_t calculate_solution(arc_set* s,int32_t last_edge, int32_t last_vertex, int32_t* vertices, edge* edges){
    int pos_end;
    int pos_start;
    s->length = 0;
    for(int i = 0;i < last_edge; i++){
        for(int j= 0; j< last_vertex; j++){
            if (vertices[j] == edges[i].start){
                pos_start = j;
            }
            if (vertices[j] ==  edges[i].end){
                pos_end = j;
            }
        }
        if(pos_start > pos_end){
            int c = (s-> length) + 1;
            if(c > 8){
                return -1;
            }
            s->result[c-1].start = edges[i].start;
            s->result[c-1].end = edges[i].end;
            s->length = c;
        }
    }
    return EXIT_SUCCESS;
}


static int32_t free_res(arc_set* result, edge* edges, int32_t* vertices ){
    free(result);
    free(vertices);
    free(edges);
    return  free_loaded_buffer();
}