/*
 * @author Vladyslav Hnatiuk(01613669)
 * @brief  Supervisor that collects data from arc set generators
 * @details Collects edge collections from arc set generators and outputs them
 * @date  13-11-2021
 * */
#include <stdio.h>
#include <semaphore.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>
#include <limits.h>

#include "datastructures.h"

static volatile int end = 0;

/*
 * @brief callback to SIGTERM signal
 * @param signum signal number
 * @param info signal info
 * @param ptr pointer
 * */
static void sig_term_handler(int signum, siginfo_t *info, void *ptr)
{
    end = 1;
}

int main() {
    int return_value = EXIT_SUCCESS;
    static struct sigaction _sigact;
    _sigact.sa_sigaction = sig_term_handler;
    _sigact.sa_flags = SA_SIGINFO;

    sigaction(SIGTERM, &_sigact, NULL);
    sigaction(SIGINT, &_sigact, NULL);

    int fd;
    void *memptr = NULL;
    sem_t *semptr_free = NULL;
    sem_t *semptr_used = NULL;
    sem_t *semptr_write = NULL;

    fd = shm_open("01613669_edges",
                      O_RDWR | O_CREAT,
                      0644);
    if (fd < 0) {
        printf("shm_open failed\n");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(struct list_of_edge_lists)) != 0) {
        printf("ftruncate failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    memptr = mmap(NULL,
                  sizeof(struct list_of_edge_lists),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          fd,
                          0);
    if (MAP_FAILED  == memptr) {
        printf("mmap failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }
    memset(memptr, 0, sizeof(struct list_of_edge_lists));

    semptr_free = sem_open("01613669_semaphore_free_space",
                             O_CREAT,
                             0644,
                             BUFFER_SIZE);
    if (semptr_free == SEM_FAILED) {
        printf("sem_open failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    semptr_used = sem_open("01613669_semaphore_used_space",
                      O_CREAT,
                      0644,
                      0);
    if (semptr_used == SEM_FAILED) {
        printf("sem_open failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    semptr_write = sem_open("01613669_semaphore_write",
                           O_CREAT,
                           0644,
                           1);
    if (semptr_write == SEM_FAILED) {
        printf("sem_open failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    struct list_of_edge_lists *list = (struct list_of_edge_lists *) memptr;
    struct edge_list current_solution;
    memset(&current_solution, 0, sizeof(struct edge_list));
    current_solution.edge_count = INT_MAX;

    int current_index = 0;
    while (end == 0) {
        if (sem_wait(semptr_used) == 0) {
            if (sem_getvalue(semptr_used, &current_index) != 0) {
                printf("sem_getvalue failed");
                return_value = EXIT_FAILURE;
                goto end;
            }
            if (list->lists[current_index].edge_count == 0) {
                printf("The graph is acyclic!\n");
                break;
            }

            if (list->lists[current_index].edge_count < current_solution.edge_count) {
                // save the best
                memset(&current_solution, 0, sizeof(struct edge_list));
                memcpy(&current_solution, &(list->lists[current_index]), sizeof(struct edge_list));

                // and print it
                printf("Solution with %d edges:", current_solution.edge_count);
                for (int j = 0; j < current_solution.edge_count; j++) {
                    printf(" %d-%d", current_solution.edges[j].node1, current_solution.edges[j].node2);
                }
                printf("\n");
            }
        }
        if (sem_post(semptr_free) != 0) {
            printf("sem_post error");
            return_value = EXIT_FAILURE;
            break;
        }
    }

end:
    if (memptr != NULL && munmap(memptr, sizeof(struct list_of_edge_lists)) == -1) {
        printf("munmap error");
        return_value = EXIT_FAILURE;
    }
    if (close(fd) != 0) {
        printf("close error");
        return_value = EXIT_FAILURE;
    }

    if (shm_unlink("01613669_edges") != 0) {
        printf("shm_unlink error");
        return_value = EXIT_FAILURE;
    }

    if (semptr_used != NULL && semptr_used != SEM_FAILED) {
        if (sem_close(semptr_used) == -1) {
            printf("sem_close error");
            return_value = EXIT_FAILURE;
        } else {
            if (sem_unlink("01613669_semaphore_used_space") != 0) {
                printf("sem_unlink error");
                return_value = EXIT_FAILURE;
            }
        }
    }
    if (semptr_free != NULL && semptr_free != SEM_FAILED) {
        if (sem_close(semptr_free) == -1) {
            printf("sem_close error");
            return_value = EXIT_FAILURE;
        } else {
            if (sem_unlink("01613669_semaphore_free_space") != 0) {
                printf("sem_unlink error");
                return_value = EXIT_FAILURE;
            }
        }
    }
    if (semptr_write != NULL && semptr_write != SEM_FAILED) {
        if (sem_close(semptr_write) == -1) {
            printf("sem_close error");
            return_value = EXIT_FAILURE;
        } else {
            if (sem_unlink("01613669_semaphore_write") != 0) {
                printf("sem_unlink error");
                return_value = EXIT_FAILURE;
            }
        }
    }

    return return_value;
}
