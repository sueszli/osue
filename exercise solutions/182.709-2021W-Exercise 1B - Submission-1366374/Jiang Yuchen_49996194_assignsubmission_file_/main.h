/**
 * @file main.h
 * @author Jiang Yuchen 12019845
 * @date 31.10.2021
 * @brief Provides constants and data structures for the supervisor and generator
 */

#ifndef AUFGABE1B_FBARCSET_H
#define AUFGABE1B_FBARCSET_H

#define MAX_DATA (100) /* max shared array size */
#define SHM_NAME "/12019845fbarcset"
#define FBARC_MAX (8) /* max number of edges in solution (if more, the solution is discarded) */

/**
 * @brief Data structure for edges
 */
struct edges {
    long from[FBARC_MAX];
    long to[FBARC_MAX];
};

/**
 * @brief Data structure for the shared memory
 */
struct shm {
    unsigned int state;
    int stop;
    int wr_pos;
    int edges_size[MAX_DATA];
    struct edges edges[MAX_DATA];
};

#endif //AUFGABE1B_FBARCSET_H
