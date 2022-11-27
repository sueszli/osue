/**
 * @brief circular buffer implementation
 * @details circular buffer used to communicate between supervisor and generators
 *
 * @author Ugljesa Jovanovic 11807861
 * @date 2021-11-14
 */
#ifndef _H_CIRC_BUF
#define _H_CIRC_BUF

#include <semaphore.h>

#define SHM_NAME_CIRC_BUF "/11807861_circ_buf"
#define SEM_NAME_USED "/11807861_used"
#define SEM_NAME_FREE "/11807861_free"
#define SEM_NAME_WRITEABLE "/11807861_writeable"
#define SEM_NAME_CLOSING "/11807861_closing"

#define EDGE_SIZE 2
#define EDGE_SET_SIZE 16
#define MAX_EDGE_SET 16
#define DATA_SIZE (EDGE_SIZE * EDGE_SET_SIZE * MAX_EDGE_SET)

/**
 * @brief struct that gets mapped to shared memory
 */
struct circ_buf_shm {
    int rd_pos, wr_pos;
    int data[DATA_SIZE];
};

/**
 * @brief the circular buffer struct
 */
struct circ_buf {
    char* program;
    int shmfd;
    struct circ_buf_shm *shm;
    sem_t *used, *free, *writeable, *closing;
};

/**
 * @brief creates a new circular buffer
 *
 * @param the circular buffer to create
 */
void circbuf_create(struct circ_buf*);

/**
 * @brief initializes an existing circular buffer
 *
 * @param the circular buffer to initialize
 */
void circbuf_init(struct circ_buf*);

/**
 * @brief reads a solution from the circular buffer
 * @details may block if no solution is available
 *
 * @param the circular buffer to read solution from
 * @param the array to store the solution to
 * @return -1 on error (see errno) and 0 on success
 */
int circbuf_read(struct circ_buf*, int[EDGE_SIZE * EDGE_SET_SIZE]);

/**
 * @brief writes a solution to the circular buffer
 * @details may block if the circular buffer is already full
 *
 * @param the circular buffer to write solutions to
 * @param the solution to write
 * @return -1 on error (see errno), 0 on success
 */
int circbuf_write(struct circ_buf*, int[EDGE_SIZE * EDGE_SET_SIZE]);

/**
 * @brief checks whether the circular buffer is active
 *
 * @param the circular buffer to check activity from
 * @param returns 1 on active and 0 on inactive
 */
int circbuf_isactive(struct circ_buf*);

/**
 * @brief deactivates the circular buffer
 * @details subsequent calls to circbuf_isactive will return 0
 *
 * @param the circular buffer to deactivate
 */
void circbuf_deactivate(struct circ_buf*);

/**
 * @brief closes the circular buffer and its resources, but does not delete it persistently
 *
 * @param the circular buffer to close
 */
void circbuf_close(struct circ_buf*);

/**
 * @brief destroys the circular buffer so that it may not be initialized by other programs
 *
 * @param the circular buffer to destroy
 */
void circbuf_destroy(struct circ_buf*);

#endif
