/**
 * @file utils.h
 * @author Atheer Elobadi <e01049225@student.tuwien.ac.at>
 * @date 08.11.2021
 *  
 * @brief contains the shared constants as well as the shm and set constructs for both generator and supervisor
 */

#ifndef UTILS_H
#define UTILS_H

#define CIR_BUF_SIZE 50
#define MAX_SET_SIZE 8

#define SHM_NAME "/01049225_shm"

#define SEM_CIR_BUF_R "/01049225_sem_cir_buf_read"
#define SEM_CIR_BUF_W "/01049225_sem_cir_buf_write"
#define SEM_MUTEX "/01049225_sem_mutex"

typedef struct
{
    uint32_t u, v;
} edge_t;

typedef struct
{
    edge_t edge[MAX_SET_SIZE];
    uint8_t size;
} set_t;

struct shm
{
    set_t data[CIR_BUF_SIZE];
    unsigned int terminated;
    unsigned int writeIndex;
};


#endif