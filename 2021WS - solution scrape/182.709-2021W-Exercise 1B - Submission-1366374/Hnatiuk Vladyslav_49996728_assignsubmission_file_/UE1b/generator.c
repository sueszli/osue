/*
 * @author Vladyslav Hnatiuk(01613669)
 * @brief  Feedback set graph generator
 * @details Based on edges from arguments generate graph without cycles.
 * @date  13-11-2021
 * */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>

#include "datastructures.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("No edges given\n");
        exit(EXIT_FAILURE);
    }
    int return_value = EXIT_SUCCESS;
    struct edge edges[100];
    int edges_count = 0;
    srand(time(NULL));
    for (int i=1; i<argc; i++) {
        struct edge new_edge;
        char *node_str = argv[i];
        char *result;
        result = strtok(node_str, "-");
        if (node_str == NULL) {
            printf("Failed to parse input\n");
            exit(EXIT_FAILURE);
        }
        new_edge.node1 = (int)strtol(result, NULL, 10);
        node_str = strtok(NULL, "-");
        if (node_str == NULL) {
            printf("Failed to parse input\n");
            exit(EXIT_FAILURE);
        }
        new_edge.node2 = (int)strtol(node_str, NULL, 10);
        edges[edges_count] = new_edge;
        edges_count++;
    }

    // find vertex count by finding the largest number of node
    int vertex_count = 0;
    for (int i=0; i<edges_count; i++) {
        if (edges[i].node1 > vertex_count) {
            vertex_count = edges[i].node1;
        }
        if (edges[i].node2 > vertex_count) {
            vertex_count = edges[i].node2;
        }
    }
    // first vertex is 0
    vertex_count++;

    int permutation[vertex_count];
    for (int i=0; i<vertex_count; i++) {
        permutation[i] = i;
    }

    for (int i=vertex_count-1; i>=1; i--) {
        int j = rand() / (RAND_MAX / i);
        int a_j = permutation[j];
        permutation[j] = permutation[i];
        permutation[i] = a_j;
    }

    struct edge_list feedback_set;
    feedback_set.edge_count = 0;
    for (int i=0; i<edges_count; i++) {
        if (permutation[edges[i].node1] <= permutation[edges[i].node2]) {
            continue;
        }
        feedback_set.edges[feedback_set.edge_count] = edges[i];
        feedback_set.edge_count++;
    }

    sem_t *semptr_write = NULL;
    sem_t* semptr_used = NULL;
    sem_t* semptr_free = NULL;
    int fd = shm_open("01613669_edges", O_RDWR, 0664);
    if (fd < 0) {
        printf("shm_open failed");
        return_value = EXIT_FAILURE;
        goto end;
    }

    void *memptr = mmap(NULL,
                        sizeof(struct list_of_edge_lists),
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED,
                          fd,
                          0);
    if (MAP_FAILED == memptr) {
        printf("mmap failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    semptr_free = sem_open("01613669_semaphore_free_space",
                             O_RDWR);
    if (semptr_free == SEM_FAILED) {
        printf("sem failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    semptr_used = sem_open("01613669_semaphore_used_space",
                                  O_RDWR);
    if (semptr_used == SEM_FAILED) {
        printf("sem failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    semptr_write = sem_open("01613669_semaphore_write",
                            O_RDWR);
    if (semptr_write == SEM_FAILED) {
        printf("sem_open failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    if (sem_wait(semptr_free) != 0) {
        printf("sem_wait failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }
    if (sem_wait(semptr_write) != 0) {
        printf("sem_wait failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    int new_current_index = 0;
    if (sem_getvalue(semptr_used, &new_current_index) != 0) {
        printf("sem_getvalue failed\n");
        return_value = EXIT_FAILURE;
        goto end;
    }

    struct list_of_edge_lists *list = (struct list_of_edge_lists *) memptr;

    if (list->end == 1) {
        goto end;
    }

    memcpy(&(list->lists[new_current_index]), &feedback_set, sizeof(struct edge_list));
    sem_post(semptr_used);
    sem_post(semptr_write);

end:
    if (munmap(memptr, sizeof(struct list_of_edge_lists)) == -1) {
        printf("munmap error");
        return_value = EXIT_FAILURE;
    }
    if (close(fd) != 0) {
        printf("close error");
        return_value = EXIT_FAILURE;
    }
    if (semptr_used != NULL && semptr_used != SEM_FAILED) {
        if (sem_close(semptr_used) == -1) {
            printf("sem_close error");
            return_value = EXIT_FAILURE;
        }
    }
    if (semptr_free != NULL && semptr_free != SEM_FAILED) {
        if (sem_close(semptr_free) == -1) {
            printf("sem_close error");
            return_value = EXIT_FAILURE;
        }
    }
    if (semptr_write != NULL && semptr_write != SEM_FAILED) {
        if (sem_close(semptr_write) == -1) {
            printf("sem_close error");
            return_value = EXIT_FAILURE;
        }
    }

    return return_value;
}
