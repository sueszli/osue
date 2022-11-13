#ifndef API_HEADER
#define API_HEADER

/**
 * @brief key used in generator and supervisor to connect with
 */
#define API_KEY_STRING "12020632"

/**
 * @brief Size of the shared memory in bytes 
 */
enum
{
    SharedMemorySize = 256,
};

/**
 * A single edge in the graph colouring problem
 */
typedef struct Edge
{
    unsigned long first;
    unsigned long second;
} Edge;

/**
 * @brief A solution found by the generator
 * 
 * There are count amount of edges. Memory allocation for the
 * edges has to be done via reallocation of the solution with the given
 * count 
 */
typedef struct Solution
{
    size_t count;
    Edge edges[];
} Solution;

#endif 
