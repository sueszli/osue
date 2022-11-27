/**
 * @file generator.c
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief Implementation of the generator module (part of the fb_arc_set functionality)
 * 
 * @details The generator parses the via CLI delivered edges and generates the respective graph (represented as 'struct graph'). After
 *          that the generator generates various fb_arc_sets for the given graph in a loop. Every solution (i.e. fb_arc_set) is written 
 *          to the shared memory which was set up by the supervisor. The generator terminates when the quit-flag in the shared memory is
 *          set by the supervisor. 
 *      
 **/


#include <time.h>

#include "fb_arc_set.h"

/**
 * @brief This static function prints information about the correct use of the generator program.
**/
void static usage_message(void){
    fprintf(stderr, "Usage: generator edge...\n");
}

/**
 * @brief This static function checks whether an int-array contains a certain number.
 * 
 * @param array                 The pointer to the int-array which is checked upon.  
 * @param length_of_array       The argument indicating the length of the array.
 * @param number_to_be_checked  The number for which the function performs the check in the provided int-array.
 * *
 * @return Returns true, if 'number_to_be_checked' is present in 'array' and false if it is not.
 */
bool static int_array_contains(int *array, int length_of_array, int number_to_be_checked){
    int index = 0;
    while(index < length_of_array){
        if( *(array+index) == number_to_be_checked){
            return true;
        }
        index++;
    }
    return false;
}

/**
 * @brief This static function creates a random fb_arc_set of the presented graph
 *
 * @param fb_arc_set             The pointer to the memory area, where the calculated fb_arc_set is stored after creation. 
 * @param graph                  The pointer to the graph, for which the fb_arc_set should be calculated.
 * @param vertex_array           The pointer to the array containing all the vertices of the provided graph.  
 * @param inverted_vertex_array  The pointer to the array which saves the index of each vertex in 'vertex_array'.
 * 
 */
void static create_fb_arc_set(struct edge_set *fb_arc_set, struct graph *graph, int *vertex_array, int *inverted_vertex_array){
    
    int k;
    int vertex_out;
    int vertex_in;
    for(k = 0; k < (*graph).length; k++){
        vertex_out = ((*graph).edge_array[k]).node_out;
        vertex_in = ((*graph).edge_array[k]).node_in;
        if(inverted_vertex_array[vertex_out] > inverted_vertex_array[vertex_in]){ 
            (fb_arc_set->edge_array[fb_arc_set->length]).node_out = vertex_out;
            (fb_arc_set->edge_array[fb_arc_set->length]).node_in = vertex_in;
            fb_arc_set->length++;
        } 
    }
}


/**
 * @brief   The main function of the generator program.
 * @details The main function is responsible for the execution of the inherent functionality of the program.
 *          It calls other functions to open the shared memory and the semaphores. 
 *          Furthermore it opens a loop, in which random fb_arc_sets are created continously. The created solutions (fb_arc_sets) 
 *          are written to the circular buffer after creation. 
 *
 * @param argc The argument counter. Its value must be greater than 2, but is not restricted to the upside, as any number of edges
 *             >=1 must be accepted
 * @param argv The array/pointer to the argument values. It contains the program name and the various edges of the graph to be processed
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char* argv[]){

    if(getopt(argc, argv, "") != -1){
        fprintf(stderr, "'generator' does not allow any options\n");
        usage_message();
        exit(EXIT_FAILURE);
    }

    if(argc == 1){
        fprintf(stderr, "'generator' needs to have at least one operand\n");
        usage_message();
        exit(EXIT_FAILURE);
    }

    int i = optind;
    struct graph graph;

    graph.length = 0;

    int vertex_out;
    int vertex_in;

    char *remaining_arg_string;
    const char *curr_edge;

    /* NOTE: loop for creating the graph */
    while (i < argc){
        curr_edge = argv[i];
        vertex_out = (int) strtol(curr_edge, &remaining_arg_string, 10); 
        vertex_in = (int) strtol((remaining_arg_string+1), NULL, 10);

        (graph.edge_array[i-optind]).node_out = vertex_out;
        (graph.edge_array[i-optind]).node_in = vertex_in;
        graph.length++;

        i++;
    }
    
    // NOTE: the maximum number of vertices in a graph with x edges is x+1
    int max_number_of_vertices = graph.length+1;

    int prelim_vertex_array[max_number_of_vertices];
    int j;

    // NOTE: initialize the prelim_vertex_array
    for(j = 0; j < max_number_of_vertices; j++){
        prelim_vertex_array[j] = -1;
    }

    int curr_vertex_out;
    int curr_vertex_in;
    int curr_index_of_prelim_vertex_array = 0;

    // NOTE: set up the _prelim_vertex_array
    for(j = 0; j < graph.length; j++){
        curr_vertex_out = (*(graph.edge_array+j)).node_out;
        curr_vertex_in = (*(graph.edge_array+j)).node_in;

        if(int_array_contains(prelim_vertex_array, sizeof(prelim_vertex_array)/sizeof(int), curr_vertex_out) == false){
            prelim_vertex_array[curr_index_of_prelim_vertex_array++] = curr_vertex_out;
        }
        
        if(int_array_contains(prelim_vertex_array, sizeof(prelim_vertex_array)/sizeof(int), curr_vertex_in) == false){
            prelim_vertex_array[curr_index_of_prelim_vertex_array++] = curr_vertex_in;
        }
    }

    int prelim_vertex_array_length = curr_index_of_prelim_vertex_array;

    // NOTE: set up the vertex_array (cut the length of the prelim_vertex_array
    int vertex_array[prelim_vertex_array_length];
    int inverted_vertex_array[prelim_vertex_array_length]; // NOTE: Array which saves the index of each vertex in the vertex_array

    int k = 0;
    while(k < prelim_vertex_array_length){
        vertex_array[k] = prelim_vertex_array[k];
        k++;
    }
 
    int fd_shm;
    circular_buffer_t *circ_buffer_used;
    initialize_shared_mem_as_client(&fd_shm, &circ_buffer_used); 

    sem_t *used_sem;
    sem_t *free_sem;
    sem_t *mutex_sem;

    initialize_sem_as_client(&used_sem, USED_SEM_NAME);
    initialize_sem_as_client(&free_sem, FREE_SEM_NAME);
    initialize_sem_as_client(&mutex_sem, MUTEX_SEM_NAME);

    /* NOTE: workaround for the problem with closing the semaphore and the shared memory (assumes there is just one generator) */
    sem_t *close_sem;
    initialize_sem_as_client(&close_sem, "/11904658_sem_close");

    int vertex_array_length = k; 

    int random_index;
    int temporary;

    struct edge_set fb_arc_set = {.length = 0};

    /* NOTE: loop to continously create solutions and write them to the circular-buffer */
    while(circ_buffer_used->quit_flag_generators == false){
        srand(time(NULL));

        /* NOTE: Fisher-Yates-Shuffle */
        for(k = vertex_array_length; k >= 1; k--){
            random_index = rand() % k;
            temporary = vertex_array[k-1];
            vertex_array[k-1] = vertex_array[random_index];
            vertex_array[random_index] = temporary;

            inverted_vertex_array[vertex_array[k-1]] = (k-1);
        }

        fb_arc_set.length = 0;
        create_fb_arc_set(&fb_arc_set, &graph, vertex_array, inverted_vertex_array);

        if(fb_arc_set.length >= MAX_NO_EDGES_ALLOWED_IN_SOLUTION){
            continue;
        } else {
            /* NOTE: write in the circular buffer */
            
            if( (sem_wait(free_sem) == -1) || (sem_wait(mutex_sem) == -1) ){
                if(errno == EINTR){
                    continue;
                }
                fprintf(stderr, "Prog. 'generator' - sem_wait failed: %s\n", strerror(errno));
            }

            memcpy((circ_buffer_used->buffer_array)[circ_buffer_used->write_end].edge_array, fb_arc_set.edge_array, sizeof(struct edge)*MAX_NO_EDGES_ALLOWED_IN_SOLUTION);
            
            (circ_buffer_used->buffer_array)[circ_buffer_used->write_end].length = fb_arc_set.length;

            //print_edge_set(&fb_arc_set, "Solution generated");

            circ_buffer_used->write_end = (circ_buffer_used->write_end+1) % BUFFER_SIZE;

            if( (sem_post(used_sem) == -1) || (sem_post(mutex_sem) == -1) ){
                fprintf(stderr, "Prog. 'generator' - sem_post failed: %s\n", strerror(errno));            
            }
        }
    }

    remove_sem_as_client(used_sem);
    remove_sem_as_client(free_sem);
    remove_sem_as_client(mutex_sem);

    close_shared_mem_as_client(fd_shm, circ_buffer_used);

    /* NOTE: workaround for the problem with closing the semaphore and the shared memory (assumes there is just one generator) */
    sem_post(close_sem);
   
    /* NOTE: workaround vor the problem with closing the semaphore and the shared memory (assumes there is just one generator) */
    remove_sem_as_client(close_sem);
    
    fprintf(stdout, "\nGenerator closed!\n");

    return EXIT_SUCCESS;
}





