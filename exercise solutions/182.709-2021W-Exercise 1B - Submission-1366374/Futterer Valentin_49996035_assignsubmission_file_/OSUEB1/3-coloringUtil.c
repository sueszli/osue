/**
 * @file 3-coloringUtil.c
 * @author Valentin Futterer 11904654
 * @date 01.11.2021
 * @brief Provides methods for mygrep.c to use.
 * @details The error and usage function are provided by this file. It also provides the core functionality for mygrep.c.
 * The mygrep method reads lines from the input and uses search_str to search the line for a keyword. If one is found,
 * it is printed to the output file.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include "3-coloringUtil.h"

/**
 * @brief Exits with error.
 * @details Prints a message to stderr and prints the error code from errno, exits with error.
 * @param prog_name Programme name to print in error message.
 * @param msg The error message to print.
 * @return Returns nothing.
*/
void handle_error(const char *prog_name, char *msg) {
    fprintf(stderr, "[%s] ERROR: %s: %s\n",prog_name, msg, strerror(errno));
    exit(EXIT_FAILURE);
}


/**
 * @brief Opens the circular buffer used by supervisor and generator.
 * @details Calls shm_open and truncates the memory area. Then it maps the memory area, closes the file
 * descriptor and returns the circular buffer.
 * @param prog_name Programme name to print in possible error messages.
 * @return Returns the circular buffer as shared memory.
*/
Circ_buf* open_circular_buffer(const char *prog_name) {
    Circ_buf *circ_buf;
    // ready the circular buffer for use
    int circ_buf_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (circ_buf_fd == -1) {
        handle_error(prog_name, "Opening of shared memory region for circular buffer failed");
    }

    if (ftruncate(circ_buf_fd, sizeof(Circ_buf)) < 0) {
        shm_unlink(SHM_NAME);
        handle_error(prog_name, "Truncating of shared memory region for circular buffer failed");
    }

    circ_buf = mmap(NULL, sizeof(*circ_buf), PROT_READ | PROT_WRITE, MAP_SHARED, circ_buf_fd, 0);
    if (circ_buf == MAP_FAILED) {
        shm_unlink(SHM_NAME);
        handle_error(prog_name, "Something went wrong mapping the circular buffer");
    }

    if(close(circ_buf_fd) == -1) {
        munmap(circ_buf, sizeof(*circ_buf));
        shm_unlink(SHM_NAME);
        handle_error(prog_name, "Closing of file descriptor for circular buffer failed");
    }

    return circ_buf;
}
