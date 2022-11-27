/**
 * @file generator.c
 * @author Andreas Hoessl <e11910612@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Generator module.
 *
 * This is the generator module for the implementation of a command line tool for finding out if a given graph is 3-colorable. It communicates with the supervisor via semaphores and a shared memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "util.h"

struct edge { /** < Structure used for representing edges of a graph. */
    int u;
    int v;
};

/**
 * Usage function
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 */
void usage(void) {
    fprintf(stderr, "usage: %s EDGE1... \n \t EDGE1...:"
            "The edges of the input graph written as 'u-v' where u and v are positive integers."
            "A vertex must not connect to itself! \n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Function
 * @brief This function opens and maps the shared memory.
 * @details The function opens the shared memory and maps it to the virtual memory of the process.
 * global vairables: shmfd, myshm,
 * @return Returns 0 on succes and -1 on failure
 */
static int open_map_shm() {
    
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0);
    if (shmfd == -1) {
        return -1;
    }
    
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    
    if (myshm == MAP_FAILED) {
        return -1;
    }
    
    if (close(shmfd) == -1) {
        return -1;
    }
    
    return 0;
}

/**
 * Function
 * @brief This function opens the semaphores.
 * @details The function opens the semaphores for synchronisation purposes.
 * global vairables: sem_free, sem_used, sem_mutex
 * @return Returns 0 on succes / -1 on failure
 */
static int open_sem() {
    
    sem_free = sem_open(SEM_FREE, 0);
    if (sem_free == SEM_FAILED) {
        return -1;
    }
    
    sem_used = sem_open(SEM_USED, 0);
    if (sem_used == SEM_FAILED) {
        return -1;
    }
    
    sem_mutex = sem_open(SEM_MUTEX, 0);
    if (sem_mutex == SEM_FAILED) {
        return -1;
    }
    
    return 0;
}

/**
 * Function
 * @brief This function parses the edges.
 * @details The function detects the edges in the input string and stores them accordingly in a edge structure.
 * If the input is not how it is supposed to be, the vertex u of the edge is set to -1 in order to signal a fault.
 * @param in_str_edges the input string
 * @return Returns edge_ret
 */
static struct edge edge_detection(char *in_str_edge) {
    
    struct edge edge_ret;
    char *u_ptr = NULL;
    
    edge_ret.u = (int) strtol(in_str_edge, &u_ptr, 10);
    edge_ret.v = (int) strtol(u_ptr + 1, NULL, 10);
    
    if (edge_ret.u == edge_ret.v) {
        edge_ret.u = -1;
    }
    
    return edge_ret;
}

/**
 * Function
 * @brief This function generates a buffer entry.
 * @details The function generates a buffer entry. A random number 0, 1 or 2 is assigned to each vertex of the graph.
 * If both vertices of an edge are assigned to the same number, the edge is written to the output string.
 * Only solutions within ENTRY_SIZE are allowed. If a solution is bigger, the function returns -1.
 * @param in_edges the input edges
 * @param in_str_edge the input edges as a string
 * @param n number of edges
 * @param out_edges string where the solution is stored
 * @return Returns 0 on succes / -1 on failure
 */
static int generate_buffer_entry(struct edge *in_edges,char **in_str_edges, int n, char *out_edges) {
    
    char vertices[n];
    memset(vertices, -1, n);
    
    for (int i = 0; i < n; i++) {
        
        if (vertices[in_edges[i].u] == -1) {
            vertices[in_edges[i].u] = (random() % 3);
        }
        
        if (vertices[in_edges[i].v] == -1) {
            vertices[in_edges[i].v] = (random() % 3);
        }
        
        if (vertices[in_edges[i].u] == vertices[in_edges[i].v]) {
            
            
            if (strlen(out_edges) + strlen(in_str_edges[i]) + 1 < ENTRY_SIZE) {
                strcat(out_edges, in_str_edges[i]);
                strcat(out_edges, " ");
                continue;
            }
            
            return -1;
        }
    }
    
    return 0;
}

/**
 * Function
 * @brief This function writes to the shared memory buffer.
 * @details Solutions are written to the circular buffer with regards to mutal exclusion.
 * @param buffer_entry string to be written in the buffer
 * @param stop flag to signal termination
 */
static void buffer_write(char *buffer_entry, int *stop) {
    
    s_wait(sem_mutex);
    
    if (myshm->state == 0) {
        
        s_wait(sem_free);
        
        strncpy(myshm->data[myshm->wr_pos], buffer_entry, ENTRY_SIZE);
        myshm->wr_pos = (myshm->wr_pos + 1) % MAX_DATA;
        
        s_post(sem_used);
        
    } else {
        *stop = 1;
    }
    
    s_post(sem_mutex);
}

/**
 * Function
 * @brief This function closes the semaphores.
 * @details Before terminating all semaphores are closed.
 * global vairables: sem_free, sem_used, sem_mutex
 * @return Returns 0 on succes / -1 on failure
 */
static int close_sem() {
    
    if (sem_close(sem_free) == -1) {
        return -1;
    }
    
    if (sem_close(sem_used) == -1) {
        return -1;
    }
    
    if (sem_close(sem_mutex) == -1) {
        return -1;
    }
    
    return 0;
}

/**
 * Program entry point.
 * @brief The main function that handles all parts of the program.
 * @details The main function starts with checking for the right command syntax. It proceeds to read in the edges and determining the number of vertices. In the while loop solutions are checked and eventually written to the buffer. Finally all resources are cleaned up.
 * global vairables: myprog, sem_free, sem_used, sem_mutex, myshm
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS
 */
int main(int argc, char **argv) {
    
    myprog = argv[0];
    int c;
    
    while ((c = getopt(argc, argv, "")) != -1) {
        usage();
    }
    if (argc == 1) {
        usage();
    }
    
    if (open_map_shm() == -1) { /** < Cheking for the succesful opening of the shared memory. */
        exit_error("failed opening/mapping shared memory");
    }
    
    if (open_sem() == -1) { /** < Cheking for the succesful opening of the semaphores. */
        exit_error("failed opening semaphore");
    }
    
    int nr_edges = argc - 1;
    char **str_edges = argv + 1;
    struct edge edges[nr_edges];
    int nr_vertices = 0;
    
    
    for (int i = 0; i < nr_edges; i++) { /** < Reading in all edges. */
        
        edges[i] = edge_detection(str_edges[i]);
        
        if (edges[i].u == -1) {
            fprintf(stderr, "The provided graph is not valid!");
            usage();
        }
        
        if (edges[i].u + 1 > nr_vertices) {
            nr_vertices = edges[i].u + 1;
        }
        
        if (edges[i].v + 1 > nr_vertices) {
            nr_vertices = edges[i].v + 1;
        }
        
    }
    
    char buffer_entry[ENTRY_SIZE];
    int stop = 0;
    
    while (!stop) {
        
        memset(buffer_entry, '\0', ENTRY_SIZE);
        
        if (generate_buffer_entry(edges, str_edges, nr_edges, buffer_entry) == -1) {
            continue; /** < Not valid solution will not be written to the buffer. */
        }
        
        buffer_write(buffer_entry, &stop);
    }
    
    /**
     * Cleanup of the resources.
     */
    if (munmap(myshm, sizeof(*myshm)) == -1) {
        exit_error("munmap failed");
    }
    
    if (close_sem() == -1) {
        exit_error("failed closing semaphore");
    }
    
    exit(EXIT_SUCCESS);    
}


