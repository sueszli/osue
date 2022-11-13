/**
 * @file supervisor.c
 * @author Manuel Can Kaya 12020629
 * @brief Supervisor program for assignment 1B Feedback arc set.
 * @details Supervisor program that sets up the shared memory and semaphores
 * necessary for communcation and synchronization with the circular buffer.
 * The supervisor continuously reads solutions from the circular buffer and
 * remembers the best solution so far. Whenever it finds a better one, it gets
 * printed to stdout. If the graph is acyclic the supervisor recognizes this,
 * closes all open resources and tells the generators to terminate as well.
 * The SIGINT and SIGTERM signals can be handled by the supervisor and also lead
 * termination.
 * @version 0.1
 * @date 2021-11-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "feedback_arc_set.h"

/**
 * @brief Signal handler function.
 * @details The function that gets called, when the SIGINT or SIGTERM signal is
 * received. It sets the global variable 'quit' to 1 and terminates.
 * 
 * @param signal The number of the signal received.
 */
static void signal_handler(int signal);

/**
 * @brief Handles the configuration of the signal handlers.
 * @details Receives a struct sigaction 'sa' and a signal number 'signal' and configures the
 * program to handle the provided signal with the given signal handler to call the function
 * 'signal_handler' if such a signal is received. Uses the global variable 'program_name'.
 * 
 * @param sa The sigaction struct used to handle the given signal.
 * @param signal The signal to be handled.
 * @return int Returns -1, if the underlying call to 'sigaction()' failed and 1 otherwise.
 */
static int set_signal_handler(struct sigaction *sa, int signal);

/**
 * @brief Initializes the shared memory, circular buffer and semaphores.
 * @details Sets up the shared memory, circular buffer and semaphores used for communication
 * and synchronization with the generators. Uses the global variable 'program_name'.
 * 
 * @param sem_used Double pointer to semaphore for number of used positions in circular buffer.
 * @param sem_free Double pointer to semaphore for number of free positions in circular buffer.
 * @param sem_mutex Double pointer to semaphore for mutually exclusive write access to circular buffer.
 * @param c_buf Double pointer to circular buffer.
 * @return int Returns -1, if an error occurs during initialization and 1 otherwise.
 */
static int initialize(sem_t **sem_used, sem_t **sem_free, sem_t **sem_mutex, circular_buffer **c_buf);

/**
 * @brief Reads feedback arc set solutions from the circular buffer.
 * @details Uses the pointers 'sem_used', 'sem_free' and 'c_buf' to read a feedback arc
 * solution from 'c_buf' and saves it in 'fb_sol'. Global variable 'program_name' is used as well.
 * 
 * @param fb_sol Pointer to a fb_arc_set_sol struct, where the read solution is saved.
 * @param sem_used Pointer to semaphore for number of used positions in circular buffer.
 * @param sem_free Pointer to semaphore for number of free positions in circular buffer.
 * @param c_buf Pointer to circular buffer.
 * @return int Returns -1, if 'sem_wait()' was interrupted by a signal, -2, if an error occurred
 * and 1 otherwise.
 */
static int circular_buffer_read(fb_arc_set_sol *fb_sol, sem_t *sem_used, sem_t *sem_free, circular_buffer *c_buf);

/**
 * @brief Prints a feedback arc set solution to stdout.
 * @details Given a feedback arc set solution 'sol' it writes the solution to stdout. Uses global
 * variable 'program_name'.
 * 
 * @param sol Feedback arc set solution to be printed.
 * @return int Returns -1, if an error occurred and 1 otherwise.
 */
static int print_curr_best_fb_arc_set(fb_arc_set_sol *sol);

/**
 * @brief Cleans up all open semaphores and shared memory.
 * @details Uses 'sem_used', 'sem_free', 'sem_mutex' and 'c_buf' to close said
 * resources cleanly and removes them. Global variable 'program_name' is also used.
 * 
 * @param sem_used Pointer to semaphore for number of used positions in circular buffer.
 * @param sem_free Pointer to semaphore for number of free positions in circular buffer.
 * @param sem_mutex Pointer to semaphore for mutually exclusive write access to circular buffer.
 * @param c_buf Pointer to circular buffer.
 * @return int Returns -1, if an error occurred during cleanup and 1 otherwise.
 */
static int cleanup(sem_t *sem_used, sem_t *sem_free, sem_t *sem_mutex, circular_buffer *c_buf);

/**
 * @brief Prints the usage message to stderr.
 * @details Prints the usage message 'Usage: supervisor' to stderr.
 * 
 */
static void usage(void);


/** Global variable for the program name. */
static char *program_name = NULL;

/** Global variable to indicate signal induced termination. */
static volatile sig_atomic_t quit = 0;


int main(int argc, char *argv[])
{
    program_name = argv[0];

    if (argc > 1)
    {
        // The program takes no command line arguments.
        usage();
        exit(EXIT_FAILURE);
    }

    // Signal handling for SIGINT and SIGTERM.
    struct sigaction sa_SIGINT;
    if (set_signal_handler(&sa_SIGINT, SIGINT) < 0)
    {
        exit(EXIT_FAILURE);
    }
    struct sigaction sa_SIGTERM;
    if (set_signal_handler(&sa_SIGTERM, SIGTERM) < 0)
    {
        exit(EXIT_FAILURE);
    }

    sem_t *sem_used = NULL;
    sem_t *sem_free = NULL;
    sem_t *sem_mutex = NULL;
    circular_buffer *c_buf = NULL;

    // Initialize shared memory and semaphores.
    if (initialize(&sem_used, &sem_free, &sem_mutex, &c_buf) < 0)
    {
        // Error occurred while initializing.
        exit(EXIT_FAILURE);
    }

    // Initialize circular buffer.
    c_buf->terminate = false;
    c_buf->read_pos = 0;
    c_buf->write_pos = 0;

    int best_fb_arc_set_size = MAX_EDGES + 1;
    fb_arc_set_sol curr_fb_arc_set_sol;
    while (quit == 0)
    {
        int read_result;
        if ((read_result = circular_buffer_read(&curr_fb_arc_set_sol, sem_used, sem_free, c_buf)) < 0)
        {
            if (read_result == -1)
            {
                // Singal interruption.
                continue;
            }
            else
            {
                // Other error occurred.
                break;
            }
        }

        if (curr_fb_arc_set_sol.size == 0)
        {
            // Graph is acyclic.
            printf("[%s] The graph is acyclic!\n", program_name);
            break;
        }
        else if (curr_fb_arc_set_sol.size < best_fb_arc_set_size)
        {
            best_fb_arc_set_size = curr_fb_arc_set_sol.size;
            print_curr_best_fb_arc_set(&curr_fb_arc_set_sol);
        }
    }

    // Signalize generators to terminate.
    c_buf->terminate = true;
    int sem_free_value = 0;
    if (sem_getvalue(sem_free, &sem_free_value) == -1)
    {
        error_msg(program_name, "sem_getvalue() failed", strerror(errno));
    }

    if (sem_free_value == 0)
    {
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            if (sem_post(sem_free) == -1)
            {
                error_msg(program_name, "sem_post() failed", strerror(errno));
            }
        }
    }

    if (cleanup(sem_used, sem_free, sem_mutex, c_buf) < 0)
    {
        // Error while terminating.
        exit(EXIT_FAILURE);
    }
    else
    {
        exit(EXIT_SUCCESS);
    }
}

static void signal_handler(int signal)
{
    quit = 1;
}

static int set_signal_handler(struct sigaction *sa, int signal)
{
    memset(sa, 0, sizeof(struct sigaction));
    sa->sa_handler = signal_handler;

    if (sigaction(signal, sa, NULL) == -1)
    {
        error_msg(program_name, "sigaction() failed", strerror(errno));
        return -1;
    }

    return 1;
}

static int initialize(sem_t **sem_used, sem_t **sem_free, sem_t **sem_mutex, circular_buffer **c_buf)
{
    // Open a file.
    int shm_fd;
    if ((shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0600)) == -1)
    {
        error_msg(program_name, "shm_open() failed", strerror(errno));
        return -1;
    }

    // Truncate file to appropriate size.
    if (ftruncate(shm_fd, sizeof(circular_buffer)) == -1)
    {
        error_msg(program_name, "ftruncate() failed", strerror(errno));
        
        if (close(shm_fd) == -1)
        {
            error_msg(program_name, "close() failed", strerror(errno));
        }
        return -1;
    }

    // Map memory.
    void *mmap_ptr = mmap(NULL, sizeof(circular_buffer), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (mmap_ptr == MAP_FAILED)
    {
        error_msg(program_name, "mmap() failed", strerror(errno));
        
        if (close(shm_fd) == -1)
        {
            error_msg(program_name, "close() failed", strerror(errno));
        }
        return -1;
    }
    (*c_buf) = (circular_buffer*)mmap_ptr;

    // Close the file.
    if (close(shm_fd) == -1)
    {
        error_msg(program_name, "close() failed", strerror(errno));

        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }
        return -1;
    }

    // Create semaphores.
    if (((*sem_used) = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0)) == SEM_FAILED)
    {
        error_msg(program_name, "sem_open() failed", strerror(errno));        
        
        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }

        if (shm_unlink(SHM_NAME) == -1)
        {
            error_msg(program_name, "shm_unlink() failed", strerror(errno));
        }
        return -1;
    }

    if (((*sem_free) = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, BUFFER_SIZE)) == SEM_FAILED)
    {
        error_msg(program_name, "sem_open() failed", strerror(errno));        
        
        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }

        if (shm_unlink(SHM_NAME) == -1)
        {
            error_msg(program_name, "shm_unlink() failed", strerror(errno));
        }

        if (sem_close(*sem_used) == -1)
        {
            error_msg(program_name, "sem_close() failed", strerror(errno));
        }

        if (sem_unlink(SEM_USED) == -1)
        {
            error_msg(program_name, "sem_unlink() failed", strerror(errno));
        }
        return -1;
    }

    if (((*sem_mutex) = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0600, 1)) == SEM_FAILED)
    {
        error_msg(program_name, "sem_open() failed", strerror(errno));        
        
        if (munmap(*c_buf, sizeof(circular_buffer)) == -1)
        {
            error_msg(program_name, "munmap() failed", strerror(errno));
        }

        if (shm_unlink(SHM_NAME) == -1)
        {
            error_msg(program_name, "shm_unlink() failed", strerror(errno));
        }

        if (sem_close(*sem_used) == -1)
        {
            error_msg(program_name, "sem_close() failed", strerror(errno));
        }

        if (sem_unlink(SEM_USED) == -1)
        {
            error_msg(program_name, "sem_unlink() failed", strerror(errno));
        }
        
        if (sem_close(*sem_free) == -1)
        {
            error_msg(program_name, "sem_close() failed", strerror(errno));
        }

        if (sem_unlink(SEM_FREE) == -1)
        {
            error_msg(program_name, "sem_unlink() failed", strerror(errno));
        }
        return -1;
    }

    return 1;
}

static int circular_buffer_read(fb_arc_set_sol *fb_sol, sem_t *sem_used, sem_t *sem_free, circular_buffer *c_buf)
{
    if (sem_wait(sem_used) == -1)
    {
        if (errno == EINTR)
        {
            // Signal interrupt.
            error_msg(program_name, "sem_wait() was interrupted by signal", strerror(errno));
            return -1;
        }
        else
        {
            // Other error.
            error_msg(program_name, "sem_wait() failed", strerror(errno));
            return -2;
        }
    }

    fb_sol->size = c_buf->solutions[c_buf->read_pos].size;
    for (int i = 0; i < fb_sol->size; i++)
    {
        fb_sol->edges[i].from_vertex = c_buf->solutions[c_buf->read_pos].edges[i].from_vertex;
        fb_sol->edges[i].to_vertex = c_buf->solutions[c_buf->read_pos].edges[i].to_vertex;
    }
    c_buf->read_pos = (c_buf->read_pos + 1) % BUFFER_SIZE;

    if (sem_post(sem_free) == -1)
    {
        error_msg(program_name, "sem_post() failed", strerror(errno));
        return -2;
    }

    return 1;
}

static int print_curr_best_fb_arc_set(fb_arc_set_sol *sol)
{
    if (printf("[%s] Solution with %d edges:", program_name, sol->size) < 0)
    {
        error_msg(program_name, "printf() failed", strerror(errno));
        return -1;
    }

    for (int i = 0; i < sol->size; i++)
    {
        if (printf(" %ld-%ld", sol->edges[i].from_vertex, sol->edges[i].to_vertex) < 0)
        {
            error_msg(program_name, "printf() failed", strerror(errno));
            return -1;
        }
    }

    if (printf("\n") < 0)
    {
        error_msg(program_name, "printf() failed", strerror(errno));
        return -1;
    }

    return 1;
}

static int cleanup(sem_t *sem_used, sem_t *sem_free, sem_t *sem_mutex, circular_buffer *c_buf)
{
    int result = 1;

    // Delete memory mapping.
    if (munmap(c_buf, sizeof(circular_buffer)) == -1)
    {
        error_msg(program_name, "munmap() failed", strerror(errno));
        result = -1;
    }

    // Unlink shared memory.
    if (shm_unlink(SHM_NAME) == -1)
    {
        error_msg(program_name, "shm_unlink() failed", strerror(errno));
        result = -1;
    }

    // Close semaphores and unlink them.
    if (sem_close(sem_used) == -1)
    {
        error_msg(program_name, "sem_close() failed", strerror(errno));
        result = -1;
    }

    if (sem_unlink(SEM_USED) == -1)
    {
        error_msg(program_name, "sem_unlink() failed", strerror(errno));
        result = -1;
    }

    if (sem_close(sem_free) == -1)
    {
        error_msg(program_name, "sem_close() failed", strerror(errno));
        result = -1;
    }

    if (sem_unlink(SEM_FREE) == -1)
    {
        error_msg(program_name, "sem_unlink() failed", strerror(errno));
        result = -1;
    }

    if (sem_close(sem_mutex) == -1)
    {
        error_msg(program_name, "sem_close() failed", strerror(errno));
        result = -1;
    }

    if (sem_unlink(SEM_MUTEX) == -1)
    {
        error_msg(program_name, "sem_unlink() failed", strerror(errno));
        result = -1;
    }

    return result;
}

static void usage(void)
{
    fprintf(stderr, "Usage: %s\n", program_name);
}
