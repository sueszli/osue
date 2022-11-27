/**
 * @author Artem Chornyi. 11922295
 * @brief Generator that finds edges in a graph (given via arguments), that make it cyclic
 *        and writes them to shared circular buffer, that then will be read by supervisor.
 * @details Uses Monte Carlo Algorithm to find solution edges.
 *          
 *          Takes a graph as an input (via arguments), which has a form of many 'n-m' edges,
 *          where n indicates a number of start vertex that point to vertex with number m;
 *          For example: '0-1 2-128 3-4 5-1 222-1 25-26'
 * 
 * @date 9-th November 2021 (09.11.2021)
 */


#ifndef EDGE
#include "./edge.c"
#endif

#ifndef SHARED_STRUCTURES
#include "./shared_structures.c"
#endif

#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h> 

#define RANDOM_SEED_LENGTH (16)


char* filename;

/**
 * @brief Prints right usage syntax for this program. 
 */
static void usage(void){
    printf("Usage: %s edge1 [edge2 [edge3 [...]]]\n", filename);
    exit(EXIT_FAILURE);
}

/**
 * @brief Generates pseuderandom seed for random operations.
 * 
 * @details Reads /dev/random module to get pseuderandom string of length RANDOM_SEED_LENGTH
 *          and set it in srand().
 */
static void set_random_seed(void){
    FILE *f;
    if((f = fopen("/dev/random", "r")) == NULL){
        fprintf(stderr, "[%s] fopen (could not open /dev/random) failed: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
    }
    char seed[RANDOM_SEED_LENGTH];
    if(fread(seed, 1, RANDOM_SEED_LENGTH, f) != RANDOM_SEED_LENGTH){
        fprintf(stderr, "[%s] failed to read %d /dev/random bytes: %s\n", filename, RANDOM_SEED_LENGTH, strerror(errno));
		exit(EXIT_FAILURE);
    }
    srand(strtol(seed, NULL, 0));
    fclose(f);
}

/**
 * @brief Writes new solution for acyclic graph in shared circular buffer.
 *
 * @details For syncronization purposes waits for both
 *          mutex (which lets only one generator write to circular buffer) and 
 *          write semaphore (to be sure there is free place to write).
 *          Also posts to 
 *          mutex (to let another process write its solutions)
 *          read semaphore (to let supervisor know, that a new entry was written to circular buffer)
 * 
 *          Function writes to circular buffer only at indexes from 0 to (CIR_BUFFER_LENGTH - 1)
 *          i.e. (pControl->writePos = (pControl->writePos + 1) % CIR_BUFFER_LENGTH;) 
 * 
 * @param semaphores custom sepahore structure which holds all needed semaphores (mutex,readSem,writeSem).
 * @param entry a circular buffer entry to be written.
 * @param pBuffer pointer to shared circular buffer, to which function will write.
 * @param pControl pointer to shared control structure,
 *                 which is used here to retrieve/save write position to circular buffer.
 */
static void write_to_cir_buffer(const semaphores semaphores, const cir_buffer_entry entry, cir_buffer *pBuffer, shared_control *pControl){
    sem_wait(semaphores.mutex);
    sem_wait(semaphores.writeSem);
    //printf("Writing at %d position\n", pControl->writePos);
    pBuffer->entries[pControl->writePos] = entry;
    pControl->writePos = (pControl->writePos + 1) % CIR_BUFFER_LENGTH;
    sem_post(semaphores.readSem);
    sem_post(semaphores.mutex);
}

/**
 * @brief Permutes given vertices in 'vertices' variable.
 *
 * @details Function uses malloc to allocate memory for permuted vertices
 *      
 *          Function copies all vertices from 'vertices' variable, to not accidentaly change
 *          initial state of 'vertices' variable
 * 
 * @param vertices pointer to vertices from argv.
 * @param verticesAmount amount of valid vertices in 'vertices' variable 
 * 
 * @return a pointer to permuted vertices
 */
static vertex *malloc_permuted_vertices(vertex * const vertices, const int verticesAmount){
    vertex *permutedVertices = malloc(verticesAmount * sizeof(vertex));
    vertex copiedVertices[verticesAmount];
    memcpy(copiedVertices, vertices, verticesAmount * sizeof(vertex));
    for (int i = 0, localverticesAmount = verticesAmount; i < verticesAmount; i++)
    {
        int permuteIndex = rand() % localverticesAmount;
        permutedVertices[i] = copiedVertices[permuteIndex];
        if(permuteIndex != localverticesAmount - 1){
            memcpy(copiedVertices+permuteIndex, copiedVertices+permuteIndex+1, localverticesAmount - permuteIndex - 1);
        }
        localverticesAmount -= 1;
    }

    return permutedVertices;
}

/**
 * @brief Finds index of 'vertex' varible in 'vertices' variable
 *
 * @param vertices pointer to vertices from argv.
 * @param vertex vertex structure to be found in 'vertices' variable
 * @param verticesAmount amount of valid vertices in 'vertices' variable 
 */
static int get_vertex_index(const vertex* const vertices, vertex vertex, int verticesAmount){
    for (int i = 0; i < verticesAmount; i++)
    {
        if(vertices[i] == vertex){
            return i;
        }
    }
    
    return -1;
}

/**
 * @brief Looks for unwanted (non positional) arguments in argv.
 * 
 * @details If any non positional is given, runs usage()
 *
 * @param argc argc from main
 * @param argv argv from main
 */
static void parse_argv_flags(const int argc, char * const* argv){
    int c;
    while((c = getopt(argc, argv, "")) != -1){
        switch(c){
            default:
                usage();
        }
    }
}

/**
 * @brief Monte Carlo Algorithm to find solutions to graph.
 * @details Writes edges to solution entry, namly 'pBuffEntry' variable,
 *          whose '.from' parameter index in 'vertices' variable
 *          is less than '.to' parameter indexes in 'vertices' variable.
 * 
 * @param pBuffEntry pointer to circular buffer entry, that will hold solution entry,
 *                   generated in this function.
 * @param vertices pointer to vertices (permuted ones).
 * @param edges pointer to all edges found in argv.
 * @param verticesAmount amount of valid vertices in 'vertices' variable
 * @param edgesAmount amount of edges in 'edges' variable
 * @return amount of edges in a resulting solution entry.
 */
static int monte_carlo(cir_buffer_entry *pBuffEntry, const vertex *vertices, const edge *const edges, const int verticesAmount, const int edgesAmount){
    cir_buffer_entry buffEntry = {.edgesAmount=0};
    int currentEdgesAmount = 0;
    for (int edgeIndex = 0; edgeIndex < edgesAmount; edgeIndex++)
    {
        for (int vertexIndex = 0; vertexIndex < verticesAmount; vertexIndex++)
        {
            int fromEdge_index, toEdge_index;
            if((fromEdge_index = get_vertex_index(vertices, edges[edgeIndex].from, verticesAmount)) == -1){
                fprintf(stderr, "[%s] [monte_carlo] get_vertex_index returned -1, BUT SHOULDN HAVE: %s\n", filename, strerror(errno));
		        exit(EXIT_FAILURE);
            }
            if((toEdge_index = get_vertex_index(vertices, edges[edgeIndex].to, verticesAmount)) == -1){
                fprintf(stderr, "[%s] [monte_carlo] get_vertex_index returned -1, BUT SHOULDN HAVE: %s\n", filename, strerror(errno));
		        exit(EXIT_FAILURE);
            }
            if(fromEdge_index > toEdge_index){
                if(currentEdgesAmount == MAX_EDGES_PER_ENTRY){
                    return -1;
                }
                buffEntry.edges[currentEdgesAmount++] = edges[edgeIndex];
                break;
            }
        }
    }
    *pBuffEntry = buffEntry;

    return currentEdgesAmount;
}

/**
 * @brief Parses string representations of edges from argv to edge structures.
 * @details For every edge in argv:
 *              split it on '-' and:
 *                  set LHS and RHD values to edge structure.
 *                  put LHS and RHD into 'vertices' variable IF they are not already there.
 * 
 * @param argc argc from main.
 * @param argv argv from main.
 * @param vertices pointer, that will hold all resulting vertices generated in this function.
 * @param verticesAmount pointer, that will hold amount of resulting vertices generated in this function.
 * @param inputEdges pointer, that will hold all edges from argv.
 */
static void parse_input_edges(const int argc, char * const* argv, vertex *const vertices, int *verticesAmount, edge *inputEdges){
    for (int i = optind, j = 0; i < argc; i++, j++)
    {  
        int cpyLength = strlen(argv[i]);
        // So that we can place nullbyte after whole string.
        char currentEdgeStr[cpyLength+1];
        
        strncpy(currentEdgeStr, argv[i], cpyLength);

        // Check if unmatched (from left side) edge was found with form e.g. '-2' or '-34' ...
        if(currentEdgeStr[0] == '-'){
            fprintf(stderr, "[%s] unmatched edge (left vertex) \n", filename);
		    exit(EXIT_FAILURE);
        }
        currentEdgeStr[cpyLength] = '\0';

        // check if '-' char is in edge string.        
        char* startMinusStr = strstr(currentEdgeStr, "-");
        if(startMinusStr == NULL){
            fprintf(stderr, "[%s] munmap inputedges failed: \n", filename);
		    exit(EXIT_FAILURE);
        }

        // check if some value was provided after '-' char in edge.
        if(strlen(startMinusStr+1) == 0){
            fprintf(stderr, "[%s] unmatched edge (right vertex) \n", filename);
		    exit(EXIT_FAILURE);
        }
        *startMinusStr = '\0';

        inputEdges[j] = (edge) {.from=strtol(currentEdgeStr, NULL, 0), .to=strtol(startMinusStr + 1, NULL, 0)}; //argv[optind+i]
        // Check if not duplicate
        if(get_vertex_index(vertices, inputEdges[j].from, *verticesAmount) == -1){
            vertices[(*verticesAmount)++] = inputEdges[j].from;
        }
        // Check if not duplicate
        if(get_vertex_index(vertices, inputEdges[j].to, *verticesAmount) == -1){
            vertices[(*verticesAmount)++] = inputEdges[j].to;
        }
        memset(currentEdgeStr, 0, cpyLength+1);
    }
}


int main(int argc, char * const* argv){
    // Exit if no arguments were specified.
    filename = argv[0];
    if(argc == 1){
        usage();
    }
    //Check if user specifies some args, apart from edges.
    parse_argv_flags(argc, argv);


    // Initialize buffers to store edges, vertices and so on.
    edge inputEdges[argc - optind];
    int verticesAmount = 0;
    int maxVerticesAmount = (argc - optind) * 2;
    vertex vertices[maxVerticesAmount];
    memset(vertices, 0, sizeof(vertices));

    parse_input_edges(argc, argv, vertices, &verticesAmount, inputEdges);


    int shmfd;
    cir_buffer *pBuffer;
    if((shmfd = shm_open(SHARED_BUFFER_NAME, O_RDWR, 0644)) == -1){
        fprintf(stderr, "[%s] shm_open with name %s failed: %s\n", filename, SHARED_BUFFER_NAME, strerror(errno));
		exit(EXIT_FAILURE);
    }
    if(ftruncate(shmfd, sizeof(*pBuffer)) < 0){
        close(shmfd);
        
        fprintf(stderr, "[%s] ftruncate for cir_buffer fd failed: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
    }
    if((pBuffer = mmap(NULL, sizeof(*pBuffer), PROT_WRITE, MAP_SHARED, shmfd, 0)) == MAP_FAILED){
        close(shmfd);
        
        fprintf(stderr, "[%s] mmap for cir_buffer failed: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
    }
    sem_t *writeSem;
    if((writeSem = sem_open(WRITE_SEMAPHORE_NAME, O_RDWR)) == SEM_FAILED){
        close(shmfd);
        munmap(pBuffer, sizeof(*pBuffer));
        
        fprintf(stderr, "[%s] sem_open with name %s failed: %s\n", filename, WRITE_SEMAPHORE_NAME, strerror(errno));
		exit(EXIT_FAILURE);
    }
    sem_t *readSem;
    if((readSem = sem_open(READ_SEMAPHORE_NAME, O_RDWR)) == SEM_FAILED){
        close(shmfd);
        munmap(pBuffer, sizeof(*pBuffer));
        
        sem_close(writeSem);

        fprintf(stderr, "[%s] sem_open with name %s failed: %s\n", filename, READ_SEMAPHORE_NAME, strerror(errno));
		exit(EXIT_FAILURE);
    }
    sem_t *mutex;
    if((mutex = sem_open(MUTEX_NAME, O_CREAT, 0644, 1)) == SEM_FAILED){
        close(shmfd);
        munmap(pBuffer, sizeof(*pBuffer));
        
        sem_close(writeSem);
        sem_close(readSem);

        fprintf(stderr, "[%s] sem_open with name %s failed: %s\n", filename, MUTEX_NAME, strerror(errno));
		exit(EXIT_FAILURE);
    }
    semaphores semaphores = {
        .mutex = mutex,
        .writeSem = writeSem,
        .readSem = readSem,
    };
    int shcfd;
    if((shcfd = shm_open(SHARED_CONTROL_NAME, O_RDWR, 0644)) == -1){
        close(shmfd);
        munmap(pBuffer, sizeof(*pBuffer));
        
        sem_close(mutex);
        sem_close(writeSem);
        sem_close(readSem);

        fprintf(stderr, "[%s] shm_open for shared_control failed: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
    }
    if(ftruncate(shcfd, sizeof(shared_control)) < 0){
        munmap(pBuffer, sizeof(*pBuffer));
        close(shmfd);
                
        sem_close(mutex);
        sem_unlink(MUTEX_NAME);
                
        sem_close(writeSem);
        sem_unlink(WRITE_SEMAPHORE_NAME);
                
        sem_close(readSem);
        sem_unlink(WRITE_SEMAPHORE_NAME);

        close(shcfd);
        

        fprintf(stderr, "[%s] ftruncate for shared_control failed: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
    }
    shared_control *pControl;
    if((pControl = mmap(NULL, sizeof(shared_control), PROT_WRITE | PROT_READ, MAP_SHARED, shcfd, 0)) == MAP_FAILED){
        munmap(pBuffer, sizeof(*pBuffer));
        close(shmfd);
        
                
        sem_close(mutex);
        sem_unlink(MUTEX_NAME);
                
        sem_close(writeSem);
        sem_unlink(WRITE_SEMAPHORE_NAME);
                
        sem_close(readSem);
        sem_unlink(WRITE_SEMAPHORE_NAME);

        close(shcfd);
        
        
        fprintf(stderr, "[%s] mmap for shared_control failed: %s\n", filename, strerror(errno));
		exit(EXIT_FAILURE);
    }

    // Generate a new pseudorandom seed.
    set_random_seed();

    // Iterate until a signal SIGINT, SIGTERM comes to supervisor.
    while(pControl->recievedInterrupt == 0)
    {
        cir_buffer_entry buffEntry;
        vertex *permutedVertices = malloc_permuted_vertices(vertices, verticesAmount);
        int mcEdgesAmount = monte_carlo(&buffEntry, permutedVertices, inputEdges, verticesAmount, (argc - optind));
        free(permutedVertices);
        // If we have calculated more edges than the boundary value.
        if(mcEdgesAmount == -1){
            continue;
        }
        printf("Sending: ");
        for (int i = 0; i < mcEdgesAmount; i++)
        {
            printf("%d-%d ", buffEntry.edges[i].from, buffEntry.edges[i].to);
        }
        printf("\n");
        write_to_cir_buffer(semaphores, buffEntry, pBuffer, pControl);
    }

    if(close(shmfd) == -1){
        fprintf(stderr, "[%s] close for cir_buffer fd failed: %s\n", filename, strerror(errno));
    }
    if(close(shcfd) == -1){
        fprintf(stderr, "[%s] close for shared_control failed: %s\n", filename, strerror(errno));
    }


    if(munmap(pBuffer, sizeof(cir_buffer)) == -1){
        fprintf(stderr, "[%s] munmap for cir_buffer failed: %s\n", filename, strerror(errno));
        // exit_with_error("munmap pBuffer %s\n");
    }
    if(munmap(pControl, sizeof(shared_control)) == -1){
        fprintf(stderr, "[%s] munmap for shared_control failed: %s\n", filename, strerror(errno));
        // exit_with_error("munmap pWritePos failed: %s\n");
    }


    if(sem_close(mutex) == -1){
        fprintf(stderr, "[%s] sem_close with name %s failed: %s\n", filename, MUTEX_NAME, strerror(errno));
        // exit_with_error("sem_close(mutex) failed: %s\n");
    }
    if(sem_close(writeSem) == -1){
        fprintf(stderr, "[%s] sem_close with name %s failed: %s\n", filename, WRITE_SEMAPHORE_NAME, strerror(errno));
        // exit_with_error("sem_close(writeSem) failed: %s\n");
    }
    if(sem_close(readSem) == -1){
        fprintf(stderr, "[%s] sem_close with name %s failed: %s\n", filename, READ_SEMAPHORE_NAME, strerror(errno));
        //exit_with_error("sem_close(readSem) failed: %s\n");
    }

    printf("Exited successfully\n");
    exit(EXIT_SUCCESS);//return 0;
}