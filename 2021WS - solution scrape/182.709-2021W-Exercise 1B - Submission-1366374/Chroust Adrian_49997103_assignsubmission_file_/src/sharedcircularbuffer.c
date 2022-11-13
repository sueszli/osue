/**
 * @file sharedcircularbuffer.c
 * @author Adrian Chroust <e12023146@student.tuwien.ac.at>
 * @date 12.11.2021
 * @brief Implementation of the shared circular buffer module.
 */

#include <stdbool.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "sharedcircularbuffer.h"

/**
 * @brief Holds the id of the current process.
 * @details It is set after the function initialize_shared_circular_buffer and
 *          unset after the function terminate_shared_circular_buffer has been successfully called.
 *          If the shared circular buffer is not set the process_id is 0.
 */
unsigned int process_id = 0;

/**
 * @brief The shared circular buffer which points to shared memory.
 * @details It is set after the function open_shared_circular_buffer and
 *          unset after the function close_shared_circular_buffer has been successfully called.
 */
buffer_t *shared_buffer;

/**
 * @brief The semaphore indicating the free space,
 *        meaning the amount of unsigned integers that can be written to the buffer data.
 * @details It is set after the function open_semaphores and
 *          unset after the function close_semaphores has been successfully called.
 *          Upon creation it is initialized with the BUFFER_DATA_LENGTH.
 */
sem_t *free_space_sem;

/**
 * @brief The semaphore indicating the used space,
 *        meaning the amount of unsigned integers that have been written and not yet read to and from the buffer data.
 * @details It is set after the function open_semaphores and
 *          unset after the function close_semaphores has been successfully called.
 *          Upon creation it is initialized with the 0.
 */
sem_t *used_space_sem;

/**
 * @brief The semaphore indicating the mutually exclusive write access,
 *        meaning that only one writing process can write to the shared buffer at a time.
 * @details It is set after the function open_semaphores and
 *          unset after the function close_semaphores has been successfully called.
 *          Upon creation it is initialized with the 1.
 */
sem_t *write_access_sem;

/**
 * @brief Closes the shared memory and unmaps shared_buffer.
 * @details If remove is set to true, the processes are notified to quit and the last process to close removes the shared memory.
 * @param remove A boolean value indicating if the shared memory should be removed or just closed.
 * @return -1 if an error occurred while unmapping, closing or removing the memory, the number of remaining processes otherwise.
 */
static unsigned int close_shared_circular_buffer(bool remove) {

    if (remove) {
        // Notify writing processes to quit.
        (*shared_buffer).quit = 1;
    }

    if (sem_wait(write_access_sem) != 0) return -1;

    // Remove the process from the process count.
    (*shared_buffer).process_count--;
    unsigned int process_count = (*shared_buffer).process_count;

    if (sem_post(write_access_sem) != 0) return -1;

    // Unmap the existing shared_buffer.
    if (munmap(shared_buffer, sizeof(*shared_buffer)) != 0) return -1;

    // Set the process id to 0 and its pointer to indicate that it has been closed.
    process_id = 0;
    shared_buffer = NULL;

    if (process_count > 0) return process_count;

    // If the current process is the last one, remove the shared memory completely.
    if (shm_unlink(BUFFER_SHM_NAME) != 0) return -1;
    return 0;
}

/**
 * @brief Opens the shared memory and assigns it to shared_buffer.
 * @param create A boolean value indicating if the shared memory has yet to be created or just opened.
 * @return -1 if an error occurred while creating, opening or mapping the memory, 0 otherwise.
 */
static int open_shared_circular_buffer(bool create) {
    // Open the file descriptor of the shared memory.
    int shm_fd = shm_open(BUFFER_SHM_NAME, O_RDWR | (create ? O_CREAT | O_EXCL : 0), 0600);
    if (shm_fd == -1) return -1;

    if (create) {
        // Set the size to the size of buffer_t upon creation.
        if (ftruncate(shm_fd, sizeof(buffer_t)) < 0) {
            close(shm_fd);
            return -1;
        }
    }

    // Map the buffer into the program memory.
    buffer_t *b = mmap(NULL, sizeof(*shared_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (b == MAP_FAILED) {
        close(shm_fd);
        return -1;
    }

    // If the mapping was successful set shared_buffer.
    shared_buffer = b;

    if (close(shm_fd) != 0) {
        // Removes or closes the shared buffer again on failure.
        close_shared_circular_buffer(create);
        return -1;
    }

    // If the shared memory was just created, quit has to be false,
    // and the process count has to be 1 and the write position has to be 0.
    // The values of the buffer data can be left as they are.
    if (create) {
        (*shared_buffer).quit = 0;
        process_id = 1;
        (*shared_buffer).process_count = process_id;
        (*shared_buffer).write_pos = 0;
    } else {
        if (sem_wait(write_access_sem) != 0) {
            close_shared_circular_buffer(false);
            return -1;
        }

        // Add the current process to the process count.
        (*shared_buffer).process_count++;
        process_id = (*shared_buffer).process_count;

        if (sem_post(write_access_sem) != 0) {
            close_shared_circular_buffer(false);
            return -1;
        }
    }
    return 0;
}

/**
 * @brief Closes the semaphores free_space_sem, used_space_sem and write_access_sem.
 * @param remove A boolean value indicating if the semaphores should be removed or just closed.
 * @return -1 if an error occurred while closing or removing one of the semaphores, 0 otherwise.
 */
static int close_semaphores(bool remove) {
    bool error = false;

    if (free_space_sem != SEM_FAILED) {
        // Remove free_space_sem if it has been successfully set previously.
        if (sem_close(free_space_sem) == 0) free_space_sem = NULL;
        else error = true;
    }
    if (used_space_sem != SEM_FAILED) {
        // Remove used_space_sem if it has been successfully set previously.
        if (sem_close(used_space_sem) == 0) used_space_sem = NULL;
        else error = true;
    }
    if (write_access_sem != SEM_FAILED) {
        // Remove write_access_sem if it has been successfully set previously.
        if (sem_close(write_access_sem) == 0) write_access_sem = NULL;
        else error = true;
    }

    if (error) return -1;
    if (!remove) return 0;

    // Unlink all semaphores and return the error value.
    return sem_unlink(FREE_SPACE_SEM_NAME)
         | sem_unlink(USED_SPACE_SEM_NAME)
         | sem_unlink(WRITE_ACCESS_SEM_NAME);
}

/**
 * @brief Opens the semaphores free_space_sem, used_space_sem and write_access_sem.
 * @param create A boolean value indicating if the semaphores should be created or just opened.
 * @return -1 if an error occurred while creating or opening one of the semaphores, 0 otherwise.
 */
static int open_semaphores(bool create) {
    if (create) {
        int oflag = O_CREAT | O_EXCL;
        int mode = 0600;

        // Create the semaphores with the specified values.
        free_space_sem = sem_open(FREE_SPACE_SEM_NAME, oflag, mode, BUFFER_DATA_LENGTH);
        used_space_sem = sem_open(USED_SPACE_SEM_NAME, oflag, mode, 0);
        write_access_sem = sem_open(WRITE_ACCESS_SEM_NAME, oflag, mode, 1);
    } else {
        // Open the existing semaphores.
        free_space_sem = sem_open(FREE_SPACE_SEM_NAME, 0);
        used_space_sem = sem_open(USED_SPACE_SEM_NAME, 0);
        write_access_sem = sem_open(WRITE_ACCESS_SEM_NAME, 0);
    }

    if (free_space_sem == SEM_FAILED || used_space_sem == SEM_FAILED || write_access_sem == SEM_FAILED) {
        // If one of the semaphores could not be created all of them are closed again.
        close_semaphores(create);
        return 1;
    }

    return 0;
}

/**
 * @brief Initializes the shared circular buffer and all semaphores.
 * @details Uses the functions open_semaphores and open_shared_circular_buffer to initialize the shared circular buffer.
 */
int initialize_shared_circular_buffer(bool create) {
    if (open_semaphores(create) != 0) return -1;
    if (open_shared_circular_buffer(create) != 0) {
        close_semaphores(create);
        return -1;
    }
    return 0;
}

/**
 * @brief Terminates the shared circular buffer and all semaphores.
 * @details If remove is set to true, the processes are notified to quit and
 *          the last process to close removes the shared memory and the semaphores.
 * @details Uses the functions close_semaphores and close_shared_circular_buffer to terminate the shared circular buffer.
 */
int terminate_shared_circular_buffer(bool remove) {
    unsigned int status = close_shared_circular_buffer(remove);
    if (status < 0) return -1;
    if (close_semaphores(status == 0) != 0) return -1;
    return 0;
}
