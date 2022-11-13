/**
 *
 * @author:     Atheer ELobadi
 * @date:       08.11.2021
 * @brief:      Generates random feedback set arc solutions.
 * @details:    Using a randomasiation algorithm, this program generates feedback set arc solutions and stores
 *              them in the shared memory which will be mapped by the supervisor.
 *
 */

#include "generatorfunctions.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <regex.h>
#include <stdlib.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"

volatile sig_atomic_t quit = 0;

/**
 * This function terminates the program on signal recieving by setting the quit flag to 1;
 *
 * @param signal The catches signal which should be handled.
 */
void handle_signal(int signal)
{
    printf("Exiting safely..\n");
    quit = 1;
}

int main(int argc, char **argv)
{
    if (argc <= 1)
    {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    regex_t regex;

    graph_t graph;
    graph.size = argc - 1;

    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);

    if (shmfd == -1)
    {
        fprintf(stderr, "Error: can´t open shared memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct shm *shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shm == MAP_FAILED)
    {
        fprintf(stderr, "Error: mapping file to memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    init_graph(&graph);

    int reg = regcomp(&regex, "[0-9][0-9]*-[0-9][0-9]*", 0);

    for (int i = 1; i < argc; i++)
    {

        reg = regexec(&regex, argv[i], (size_t)0, NULL, 0);
        if (reg == REG_NOMATCH)
        {
            errno = EINVAL;
            fprintf(stderr, "Error reading edge %d:.\n%s\n", i, strerror(errno)); // TODO: replace with usage
            printf("Usage: %s {[d]-[d]}*\n", argv[0]);
            exit(EXIT_FAILURE);
        }
        add_string_edge_to_graph(argv[i], &graph);
    }

    sem_t *sem_used = sem_open(SEM_CIR_BUF_R, 0);
    sem_t *sem_free = sem_open(SEM_CIR_BUF_W, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX, 0);

    set_t *feedback_arc_set;

    struct sigaction sa = {.sa_handler = handle_signal};
    sigaction(SIGINT, &sa, NULL);
    fprintf(stdout, "\033[0;34m%d: Generating Feedback Set Arc...\033[0;0m\n", getpid());
    while (!quit && !shm->terminated)
    {
        feedback_arc_set = get_feedback_arc_set(graph);
        if (feedback_arc_set->size < MAX_SET_SIZE)
        {
            if (sem_wait(sem_free) == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                exit(EXIT_FAILURE);
            }
            shm->data[shm->writeIndex] = *feedback_arc_set;
            if (sem_wait(sem_mutex) == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                exit(EXIT_FAILURE);
            }
            shm->writeIndex = (shm->writeIndex + 1) % CIR_BUF_SIZE;
            if (sem_post(sem_mutex) == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                exit(EXIT_FAILURE);
            }

            if (sem_post(sem_used) == -1)
            {
                if (errno == EINTR)
                {
                    continue;
                }
                exit(EXIT_FAILURE);
            }
        }
    }
    //free(feedback_arc_set);
    if (munmap(shm, sizeof(*shm)) == -1)
    {
        fprintf(stderr, "Error: Can´t unmap shared memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (close(shmfd) == -1)
    {
        fprintf(stderr, "Error: Can´t close shared memory.\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sem_close(sem_used) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_free) == -1)
    {
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_mutex) == -1)
    {
        exit(EXIT_FAILURE);
    }

    regfree(&regex);
    fprintf(stderr, "\033[0;32mDone!\033[0;0m\n");
    return EXIT_SUCCESS;
}
