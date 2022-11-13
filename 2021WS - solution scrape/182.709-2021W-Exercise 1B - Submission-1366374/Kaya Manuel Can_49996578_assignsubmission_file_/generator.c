/**
 * @file generator.c
 * @author Manuel Can Kaya 12020629
 * @brief Generator program for assignment 1B Feedback arc set.
 * @details Generator program that reads a graph's edges as its input and produces random feedback arc set
 * solutions, which it writes to circular buffer in shared memory with semaphores for synchronization
 * with a supervisor and other generator processes.
 * @details
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "feedback_arc_set.h"

/**
 * @brief Initializes the shared memory, circular buffer and semaphores.
 * @details Opens up the shared memory, circular buffer and semaphores used for communication
 * and synchronization with the supervisor. Uses the global variable 'program_name' to do this.
 * 
 * @param sem_used Double pointer to semaphore for number of used positions in circular buffer.
 * @param sem_free Double pointer to semaphore for number of free positions in circular buffer.
 * @param sem_mutex Double pointer to semaphore for mutually exclusive write access to circular buffer.
 * @param c_buf Double pointer to circular buffer.
 * @return int Returns -1, if an error occurs during initialization and 1 otherwise.
 */
static int initialize(sem_t **sem_used, sem_t **sem_free, sem_t **sem_mutex, circular_buffer **c_buf);

/**
 * @brief Parses command line arguments.
 * @details The program takes no command line options and at least one positional argument.
 * 
 * @param argc Number of command line arguments.
 * @param argv Array holding the command line arguments.
 * @return int Returns -1, if an invalid option or no positional argument is read and 1 otherwise.
 */
static int parse_arguments(int argc, char *argv[]);

/**
 * @brief Reads edges provided to program.
 * @details Reads the edges provided in 'argv' into 'edges' and checks, whether they are valid.
 * 
 * @param argc Number of command line arguments.
 * @param argv Array holding the command line arguments.
 * @param edges Pointer to memory, where the edges should be read into.
 * @return int Returns -1, if at least one of the edges was not valid and 1 otherwise.
 */
static int read_edges(int argc, char *argv[], edge *edges);

/**
 * @brief Checks whether an edge is valid.
 * @details Check whether an edge 'edge' is valid, by making sure it is of the form
 * 'd+-d+', meaning exactly one hypen is present and everything before and after it is an integer.
 * 
 * @param edge Edge of the graph.
 * @return int Returns -1, if the edge does not conform to specification and 1 otherwise.
 */
static int check_edge(const char *edge);

/**
 * @brief Finds the number of vertices in the graph.
 * @details Finds the number of vertices in the graph, provided as a list of edges 'edges'.
 * 
 * @param edges Edges of the graph.
 * @param num_of_edges Number of edges in the graph.
 * @return int Returns number of vertices in the graph.
 */
static int number_of_vertices(edge *edges, int num_of_edges);

/**
 * @brief Generates a random vertex permutation.
 * @details Uses the Durstenfeld variant of the Fisher-Yates-Shuffle to permute a list of vertices 'vertices'.
 * 
 * @param vertices Vertices of the graph.
 * @param num_of_vertices Number of vertices in the graph.
 */
static void durstenfeld_fisher_yates_shuffle(vertex *vertices, int num_of_vertices);

/**
 * @brief Finds a feedback arc set solution.
 * @details All edges u-v in 'edges', for which u > v in the ordering of the vertex permutation 'vertices'
 * are saved into 'fb_sol', therefore generating a solution for a feedback arc set. If the found feedback
 * arc set is larger than MAX_EDGES than the solution is thrown out.
 * 
 * @param fb_sol Feedback arc set solution to be written into.
 * @param vertices Vertices of the graph.
 * @param num_of_vertices Number of vertices in the graph.
 * @param edges Edges of the graph.
 * @param num_of_edges Number of edges in the graph.
 * @return int Returns -1, if the found solution is larger than MAX_EDGES and 1 otherwise.
 */
static int find_fb_arc_set(fb_arc_set_sol *fb_sol, vertex *vertices, int num_of_vertices, edge *edges, int num_of_edges);

/**
 * @brief Finds the position of a vertex in a vertex permutation.
 * @details Finds the position of a vertex 'v' in a vertex permutation 'vertices' of size
 * 'num_of_vertices'.
 * 
 * @param v Vertex to look for.
 * @param vertices Vertices of the graph.
 * @param num_of_vertices Number of vertices in the graph.
 * @return int Returns -1, if 'v' could not be found and its position otherwise.
 */
static int find_pos_of_vertex(vertex v, vertex *vertices, int num_of_vertices);

/**
 * @brief Seeds the random number generator.
 * @details Seeds the random number generator 'next_rand()', so that each generator process
 * produces different random numbers (seed = time(NULL) * getpid()).
 * 
 */
static void seed_rand_generator(void);

/**
 * @brief Return a random number.
 * @details Returns a random number between 0 and 'upper_limit'.
 * 
 * @param upper_limit Upper limit of random number.
 * @return int Returns the random number generated.
 */
static int next_rand(int upper_limit);

/**
 * @brief Writes feedback arc set solutions to the circular buffer.
 * @details Uses 'sem_mutex' and 'c_buf' to write a feedback arc
 * solution 'fb_sol' to 'c_buf'. Global variable 'program_name' is used as well.
 * 
 * @param fb_sol Pointer to a fb_arc_set_sol struct to be written into to 'c_buf'.
 * @param sem_mutex Pointer to semaphore for mutually exclusive write access to circular buffer.
 * @param c_buf Pointer to circular buffer.
 * @return int Returns -1, if 'sem_wait()' was interrupted by a signal, -2, if an error occurred
 * and 1 otherwise.
 */
static int circular_buffer_write(fb_arc_set_sol *fb_sol, sem_t *sem_mutex, circular_buffer *c_buf);

/**
 * @brief Cleans up all open semaphores and shared memory.
 * @details Uses 'sem_used', 'sem_free', 'sem_mutex' and 'c_buf' to close said
 * resources cleanly. Global variable 'program_name' is also used.
 * 
 * @param sem_used Pointer to semaphore for number of used positions in circular buffer.
 * @param sem_free Pointer to semaphore for number of free positions in circular buffer.
 * @param sem_mutex Pointer to semaphore for mutually exclusive write access to circular buffer.
 * @param c_buf Pointer to circular buffer.
 * @return int Returns -1, if an error occurred during cleanup and 1 otherwise.
 */
static int cleanup(sem_t *sem_used, sem_t *sem_free, sem_t *sem_mutex, circular_buffer *c_buf);

/**
 * @brief Prints the usage message to stderr.
 * @details Prints the usage message 'Usage: generator EDGE...' and an example call to stderr.
 * 
 */
static void usage(void);


/** Global variable for the program name. */
static char *program_name = NULL;


int main(int argc, char *argv[])
{
    program_name = argv[0];

    if (parse_arguments(argc, argv) < 0)
    {
        // Command line arguments were not provided according to specification.
        usage();
        exit(EXIT_FAILURE);
    }

    int num_of_edges = argc - 1;
    edge *edges = (edge*)malloc(sizeof(edge) * num_of_edges);   // DON'T FORGET TO FREE.
    if (edges == NULL)
    {
        // malloc() failed.
        error_msg(program_name, "malloc() failed", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (read_edges(argc, argv, edges) < 0)
    {
        // Edges didn't adhere to specification.
        usage();
        free(edges);
        exit(EXIT_FAILURE);
    }

    int num_of_vertices = number_of_vertices(edges, num_of_edges);
    vertex *vertex_permutation = (vertex*)malloc(sizeof(vertex) * num_of_vertices); // DON'T FORGET TO FREE.
    if (vertex_permutation == NULL)
    {
        // malloc() failed.
        error_msg(program_name, "malloc() failed", strerror(errno));
        free(edges);
        exit(EXIT_FAILURE);
    }

    sem_t *sem_used = NULL;
    sem_t *sem_free = NULL;
    sem_t *sem_mutex = NULL;
    circular_buffer *c_buf = NULL;

    // Initialize shared memory and semaphores.
    if (initialize(&sem_used, &sem_free, &sem_mutex, &c_buf) < 0)
    {
        free(vertex_permutation);
        free(edges);
        exit(EXIT_FAILURE);
    }

    seed_rand_generator();

    // Look for solutions.
    fb_arc_set_sol fb_sol;
    bool success = true;
    while (c_buf->terminate == false)
    {
        // Generates a random permutation of the vertices.
        durstenfeld_fisher_yates_shuffle(vertex_permutation, num_of_vertices);
    
        int fb_sol_validity = find_fb_arc_set(&fb_sol, vertex_permutation, num_of_vertices, edges, num_of_edges);
        if (fb_sol_validity < 0)
        {
            // The solution was bigger than MAX_EDGES.
            continue;
        }

        if (sem_wait(sem_free) == -1)
        {
            if (errno == EINTR)
            {
                // Signal interrupt.
                error_msg(program_name, "sem_wait() was interrupted by signal", strerror(errno));
                continue;
            }
            else
            {
                // Other error.
                error_msg(program_name, "sem_wait() failed", strerror(errno));
                success = false;
                break;
            }
        }

        if (c_buf->terminate == true)
        {
            break;
        }
        else
        {
            if (circular_buffer_write(&fb_sol, sem_mutex, c_buf) < 0)
            {
                success = false;
                break;
            }

            if (sem_post(sem_used) == -1)
            {
                error_msg(program_name, "sem_post() failed", strerror(errno));
                success = false;
                break;
            }
        }
    }

    // Free resources.
    free(vertex_permutation);
    free(edges);
    
    if (cleanup(sem_used, sem_free, sem_mutex, c_buf) < 0 || success == false)
    {
        exit(EXIT_FAILURE);
    }
    else
    {
        exit(EXIT_SUCCESS);
    }
}

static int initialize(sem_t **sem_used, sem_t **sem_free, sem_t **sem_mutex, circular_buffer **c_buf)
{
    // Open a file.
    int shm_fd;
    if ((shm_fd = shm_open(SHM_NAME, O_RDWR, 0600)) == -1)
    {
        error_msg(program_name, "shm_open() failed", strerror(errno));
        return -1;
    }

    // Map memory.
    void *mmap_ptr = mmap(NULL, sizeof(circular_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mmap_ptr == MAP_FAILED)
    {
        error_msg(program_name, "mmap() failed", strerror(errno));
        
        if (close(shm_fd) == -1)
        {
            error_msg(program_name, "close() failed", strerror(errno));
        }
        return -1;
    }
    (*c_buf) = (circular_buffer*)mmap_ptr;

    // Close the file.
    if (close(shm_fd) == -1)
    {
        error_msg(program_name, "close() failed", strerror(errno));

        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }
        return -1;
    }

    // Open semaphores.
    if (((*sem_used) = sem_open(SEM_USED, 0)) == SEM_FAILED)
    {
        error_msg(program_name, "sem_open() failed", strerror(errno));        
        
        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }
        return -1;
    }

    if (((*sem_free) = sem_open(SEM_FREE, 0)) == SEM_FAILED)
    {
        error_msg(program_name, "sem_open() failed", strerror(errno));        
        
        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }

        if (sem_close(*sem_used) == -1)
        {
            error_msg(program_name, "sem_close() failed", strerror(errno));
        }
        return -1;
    }

    if (((*sem_mutex) = sem_open(SEM_MUTEX, 0)) == SEM_FAILED)
    {
        error_msg(program_name, "sem_open() failed", strerror(errno));        
        
        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }

        if (sem_close(*sem_used) == -1)
        {
            error_msg(program_name, "sem_close() failed", strerror(errno));
        }
        
        if (sem_close(*sem_free) == -1)
        {
            error_msg(program_name, "sem_close() failed", strerror(errno));
        }
        return -1;
    }

    return 1;
}

static int parse_arguments(int argc, char *argv[])
{
    if (argc < 2)
    {
        // At least one edge must be provided.
        return -1;
    }

    // The program takes no command line arguments.
    char c;
    while ((c = getopt(argc, argv, "")) != -1)
    {
        switch (c)
        {
            case '?':
                // Invalid option read.
                // Indicate error.
                return -1;
            default:
                // Simply continue looking for other invalid options.
                break;
        }
    }

    return 1;
}

static int read_edges(int argc, char *argv[], edge *edges)
{
    int hyphen_index = -1;
    int edge_index = 0;

    char *from_vertex = NULL;
    char *to_vertex = NULL;

    for (int i = 1; i < argc; i++, edge_index++)
    {
        if ((hyphen_index = check_edge(argv[i])) < 0)
        {
            // The edge is not valid.
            return -1;
        }

        from_vertex = strtok(argv[i], "-");
        to_vertex = strtok(NULL, "-");
        
        long from_l = strtol(from_vertex, NULL, 10);
        if (from_l == LONG_MIN || from_l == LONG_MAX)
        {
            // Overflow or underflow.
            return -1;
        }
        long to_l = strtol(to_vertex, NULL, 10);
        if (to_l == LONG_MIN || to_l == LONG_MAX)
        {
            return -1;
        }

        edges[edge_index].from_vertex = from_l;
        edges[edge_index].to_vertex = to_l;
    }

    return 1;
}

static int check_edge(const char *edge)
{
    int edge_length = strlen(edge); // Excluding '\0'.

    if (edge_length < 3)
    {
        // An edge must be at least of length 3, since "d+-d+".
        return -1;
    }

    int i = 0;
    int hyphen_counter = 0;
    int hyphen_index = 0;
    while (edge[i] != '\0')
    {
        if (edge[i] == '-')
        {
            hyphen_index = i;
            hyphen_counter++;
        }
        i++;
    }

    if (hyphen_counter != 1)
    {
        // If the hyphen does not appear exactly once, return with error.
        return -1;
    }

    if (hyphen_index == 0 || hyphen_index == (edge_length - 1))
    {
        // If the hyphen is the first or last character, return with error.
        return -1;
    }

    if (edge[0] == '0' && hyphen_index != 1)
    {
        // If the first vertex starts with a zero and is not equal to 0, return with error.
        return -1;
    }

    if (edge[hyphen_index + 1] == '0' && (hyphen_index + 2) != edge_length)
    {
        // If the second vertex starts with a zero and is not equal to 0, return with error.
        return -1;
    }

    // First vertex.
    for (i = 0; i < hyphen_index; i++)
    {
        if (isdigit(edge[i]) == 0)
        {
            // The character is not a digit.
            return -1;
        }
    }

    // Second vertex.
    for (i = hyphen_index + 1; i < edge_length; i++)
    {
        if (isdigit(edge[i]) == 0)
        {
            // The character is not a digit.
            return -1;
        }
    }

    // Caller can use hyphen_index to turn edge into vertices.
    return hyphen_index;
}

static int number_of_vertices(edge *edges, int num_of_edges)
{
    vertex v = -1;
    vertex u = -1;
    vertex from_vertex = -1;
    vertex to_vertex = -1;

    // Find maximum vertex.
    for (int i = 0; i < num_of_edges; i++)
    {
        from_vertex = edges[i].from_vertex;
        to_vertex = edges[i].to_vertex;

        u = (from_vertex > to_vertex ? from_vertex : to_vertex);
        v = (u > v ? u : v);
    }

    return v + 1;  // +1 since the vertices begin at 0
}

static void durstenfeld_fisher_yates_shuffle(vertex *vertices, int num_of_vertices)
{
    for (int i = 0; i < num_of_vertices; i++)
    {
        vertices[i] = i;
    }

    int j;
    for (int i = num_of_vertices - 1; i > 0; i--)
    {
        j = next_rand(i);
        vertex temp = vertices[i];
        vertices[i] = vertices[j];
        vertices[j] = temp;
    }
}

static int find_fb_arc_set(fb_arc_set_sol *fb_sol, vertex *vertices, int num_of_vertices, edge *edges, int num_of_edges)
{
    int sol_size = 0;
    int pos_from_vertex = -1;
    int pos_to_vertex = -1;

    for (int i = 0; i < num_of_edges; i++)
    {
        pos_from_vertex = find_pos_of_vertex(edges[i].from_vertex, vertices, num_of_vertices);
        pos_to_vertex = find_pos_of_vertex(edges[i].to_vertex, vertices, num_of_vertices);

        if (pos_from_vertex > pos_to_vertex)
        {
            if (sol_size == MAX_EDGES)
            {
                // The feedback arc set is too large and can be dismissed.
                return -1;
            }

            fb_sol->edges[sol_size].from_vertex = edges[i].from_vertex;
            fb_sol->edges[sol_size].to_vertex = edges[i].to_vertex;
            sol_size++;
        }
    }
    fb_sol->size = sol_size;
    
    return 1;
}

static int find_pos_of_vertex(vertex v, vertex *vertices, int num_of_vertices)
{
    for (int i = 0; i < num_of_vertices; i++)
    {
        if (v == vertices[i])
        {
            return i;
        }
    }
    return -1;  // Should never reach this line.
}

static void seed_rand_generator(void)
{
    unsigned int seed = time(NULL) * getpid();
    srand(seed);
}

static int next_rand(int upper_limit)
{
    return (int)(rand() / (float)RAND_MAX * upper_limit);
}

static int circular_buffer_write(fb_arc_set_sol *fb_sol, sem_t *sem_mutex, circular_buffer *c_buf)
{
    if (sem_wait(sem_mutex) == -1)
    {
        if (errno == EINTR)
        {
            // Singal interrupt.
            error_msg(program_name, "sem_wait() was interrupted by signal", strerror(errno));
            return -1;
        }
        else
        {
            // Other error.
            error_msg(program_name, "sem_wait() failed", strerror(errno));
            return -2;
        }
    }

    c_buf->solutions[c_buf->write_pos].size = fb_sol->size;
    for (int i = 0; i < fb_sol->size; i++)
    {
        c_buf->solutions[c_buf->write_pos].edges[i].from_vertex = fb_sol->edges[i].from_vertex;
        c_buf->solutions[c_buf->write_pos].edges[i].to_vertex = fb_sol->edges[i].to_vertex;
    }
    c_buf->write_pos = (c_buf->write_pos + 1) % BUFFER_SIZE;

    if (sem_post(sem_mutex) == -1)
    {
        error_msg(program_name, "sem_post() failed", strerror(errno));
        return -2;
    }

    return 1;
}

static int cleanup(sem_t *sem_used, sem_t *sem_free, sem_t *sem_mutex, circular_buffer *c_buf)
{
    int result = 1;

    // Delete memory mapping.
    if (munmap(c_buf, sizeof(circular_buffer)) == -1)
    {
        error_msg(program_name, "munmap() failed", strerror(errno));
        result = -1;
    }

    // Close semaphores.
    if (sem_close(sem_used) == -1)
    {
        error_msg(program_name, "sem_close() failed", strerror(errno));
        result = -1;
    }

    if (sem_close(sem_free) == -1)
    {
        error_msg(program_name, "sem_close() failed", strerror(errno));
        result = -1;
    }

    if (sem_close(sem_mutex) == -1)
    {
        error_msg(program_name, "sem_close() failed", strerror(errno));
        result = -1;
    }

    return result;
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s EDGE...\n", program_name);
    fprintf(stderr, "Example: %s 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0\n", program_name);
}
