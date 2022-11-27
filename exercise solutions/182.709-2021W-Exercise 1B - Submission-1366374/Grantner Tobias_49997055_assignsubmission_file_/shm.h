/**
 * @file graph.h
 * @author Tobias Grantner (12024016)
 * @brief This class represents the data that should be shared between the supervisor and the generator and contains a circularbuffer and a flag to stop the programs.
 * @date 2021-11-11
 */

#ifndef SHMCONSTANTS_H /* include guard */
#define SHMCONSTANTS_H

#include "circularbuffer.h"

/**
 * @brief The name of teh shared memory
 * 
 */
#define SHM_NAME "/12024016_shm"

/**
 * @brief Represents the data to be shared between supervisor and generator.
 */
typedef struct {
    int quit;
    circularbuffer_t cb;
} shm_t;

#endif