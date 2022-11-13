/**
 * @file shm.h
 * @author Filip Coja <e12023141@student.tuwien.ac.at>
 * @date 11.11.2021
 *
 * @brief Provides functions to open and close a custom shared memory struct and share it with multiple processes.
 * @details MAX_DATA shouldn't exceed 4Kb as specified in the exercise. To increase performance this implementation
 * should be switched to int16_t since the graphs in this exercise do not contain lots of nodes.
 *
 */

#ifndef SHM_H
#define SHM_H

/** Circular Buffer size => sizeof(long) is 8 Bytes on 64 Bit Systems => 400 * 8 Bytes = 3200 Bytes */
#define MAX_DATA (400)

/** Struct which is shared between multiple processes */
typedef struct {
    bool halt;
    long data[MAX_DATA];
    long write_idx;
    long read_idx;
} shared_memory_t;

/**
 * @brief Opens and returns a shared memory object defined above this comment. returns NULL on errors.
 *
 * @details This function uses shm_open, ftruncate and mmap, so there are lots of things that can go wrong.
 * The function returns NULL if one of these functions fail.
 *
 * Default values are set at the end and are the following:
 *      shm->read_idx = 0;
 *      shm->write_idx = 0;
 *      shm->halt = false;
 *
 * When finished must be freed with close_shm()
 *
 * @param fd Integer pointer for the file descriptor. Important since you need this if you want to close the buffer again.
 * @param server Boolean which clarifies if the shared memory is opened by a server or a client.
 * @return A shared memory object o defined above this comment block OR NULL.
 */
shared_memory_t *open_shm(int *fd, bool server);

/**
 * @brief Closes the shared memory created by open_shm().
 *
 * @details As well as in the open_shm documentation, close_shm can encounter a lot of problems when executing.
 * The return value shows if everything was closed accordingly. False implies that there was a problem with a resource to be closed.
 *
 * @param shm Shared memory object to be closed.
 * @param fd Integer pointer for the file descriptor. Should be the value which was set by open_shm().
 * @param server Boolean which clarifies if the shared memory is closed by a server or a client.
 * @return Boolean if everything was successfully.
 */
bool close_shm(shared_memory_t *shm, int fd, bool server);


#endif
