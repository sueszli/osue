/**
 * General shared memory module.
 * @file myshm.h
 * @author Marvin Flandorfer, 52004069
 * @date 04.11.2021
 * 
 * @brief This module defines the shared memory name and the max data as well as a struct for the shared memory.
 * @details This module only consists of Macro defines and a struct typedef.
 */

#ifndef MYSHM_H
#define MYSHM_H

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

#define SHM_NAME "/52004069myshm"               /**< Macro for the name of the shared memory*/
#define MAX_DATA (1019)                         /**< Macro for the max data that can be stored in the buffer*/

/**
 * Shared memory struct
 * @brief Struct for the shared memory object (circular buffer).
 */
typedef struct myshm{
    int state;                                  /**< State of the circular buffer (0 when set up, -1 when termination is coming up)*/
    int write_pos;                              /**< Current writing position in the circular buffer*/
    int read_pos;                               /**< Current reading position in the circular buffer*/
    int buffer[MAX_DATA];                       /**< Integer array that stores all data (supervisors read, generators write)*/
}shm_t;

#endif