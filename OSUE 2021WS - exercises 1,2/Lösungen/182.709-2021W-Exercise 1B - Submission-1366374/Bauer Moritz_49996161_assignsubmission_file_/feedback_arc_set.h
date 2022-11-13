/**
 * @brief constants and types for EXERCISE 1B
 * @author Moritz Bauer, 0649647
 * @date 21.11.11
 */

#ifndef B1_FEEDBACK_ARC_SET_H
#define B1_TUTORIALS_FEEDBACK_ARC_SET_H

#define MAX_ARC_SET_SIZE 8

#define SEM_1 "/sem_0649647_1"
#define SEM_2 "/sem_0649647_2"

#define SEM_FREE "/sem_0649647_free"
#define SEM_USED "/sem_0649647_used"
#define SEM_MUTEX "/sem_0649647_gen_mutex"

#define SHM_NAME "/shm_0649647"
#define BUFFER_SIZE (8)


struct Edge {
    long from;
    long to;
};

struct Feedback_Arc_Set {
    int size;
    struct Edge edges[MAX_ARC_SET_SIZE];
};

struct ring_buffer {
    unsigned int front;
    unsigned int end;
    struct Feedback_Arc_Set buffer[BUFFER_SIZE];
};

struct shared_memory {
    unsigned int quit;
    struct ring_buffer buffer;
};

#endif //B1_FEEDBACK_ARC_SET_H
