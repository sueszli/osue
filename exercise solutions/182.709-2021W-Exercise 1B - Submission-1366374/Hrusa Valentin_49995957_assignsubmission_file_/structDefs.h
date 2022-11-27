/**
 * @file structDefs.h
 * @author Valentin Hrusa 11808205
 * @brief contains all structs and definitions used in supervisor and generator  
 * 
 * @date 12.11.2021
 * 
 */
#define MAX_ALLOWED_EDGES (8)
#define SHM_MEM_INDEX (32)
#define SHM_NAME "/11808205-shared-memory"
#define SEM_WRITE_NAME "/11808205-write-semaphore"
#define SEM_READ_NAME "/11808205-read-semaphore"
#define SEM_MUTEX_NAME "/11808205-mutex-semaphore"

/**
 * @brief an Edge is represented by its start node and the node the edge points to.
 * 
 * @param endFlag used for Edge-arrays as a NULL-substitute
 * 
 */
typedef struct Single_Edge_Representation{
    int start_node;
    int end_node;
    int endFlag;
} Edge;

/**
 * @brief a Fb-Arc-Set is represented by its set of Edges which have a defined upper bound of
 *        MAX_ALLOWED_EDGES
 * 
 * @param count actual numbers of Edges in the Fb_Arc_Set
 * 
 */
typedef struct Feedback_Arc_Set_Representation{
    Edge edges[MAX_ALLOWED_EDGES+1];
    int count;
} Fb_Arc_Set;

/**
 * @brief the Circular Buffer is represented by an array of Fb_Arc_Sets 
 * 
 * @param index mutual-exclusive index to write a new solution to
 * @param max_edges number of Edges of the best solutions so far
 * @param exit signals to generators to keep going or to terminate
 * 
 */
typedef struct Circular_Buffer_Representation{
    Fb_Arc_Set buffer[SHM_MEM_INDEX];
    int index;
    int max_edges;
    volatile sig_atomic_t exit;
} Circular_Buffer;

