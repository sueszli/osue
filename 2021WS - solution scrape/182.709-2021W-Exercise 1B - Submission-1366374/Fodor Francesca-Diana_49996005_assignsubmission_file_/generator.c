/**
 * @file generator.c
 * @author Fodor Francesca Diana, 11808223
 * @date 08.11.2021
 * @brief This program writes solution to the 3-coloring problem into a shared memory
 * file, until a certain value in memory is set and it exits.
 * @details This program opens a file in shared memory and opens semaphores to synchronize writing
 * to the shared memory. It takes a graph as an input from the command line and then generates random
 * solutions to the 3-coloring problem by randomly assigning one of the three colors [red, green, blue]
 * to all nodes and then removes the edges between nodes that share the same color. These edges form a
 * minimal 3-coloring set. All sets longer than eight edges are discarded and not written into the
 * shared memory object.
 */

#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include "sharedfunctions.h"

#define ERR_EXIT(msg) fprintf(stderr, "[%s] %s error: %s\n", argv[0], msg, strerror(errno)); exit(EXIT_FAILURE);
#define USER_ERROR(msg) fprintf(stderr, "[%s] %s\n", argv[0], msg); exit(EXIT_FAILURE);

/**
 * @brief The main method opening the shared memory object and all necessary semaphores, afterwards
 * random solutions are generated and written to the shared memory object, until a flag of the
 * object is set.
 * @param argc The argument counter, e.g. the number of edges passed
 * @param argv The argument vector, e.g. the edges of the graph in a format of '%d-%d', seperated
 * by spaces
 */
int main(int argc, char *argv[]) {

    if (argc == 1) {
        ERR_EXIT("No edges were given!")
    }

    //open shared memory object
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        ERR_EXIT("shm_open");
    }

    struct myshm *myshm;

    //map memory to virtual space
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED) {
        ERR_EXIT("mmap");
    }

    int max = 0;
    for (int i = 1; i < argc; i++) {
        int x = 0;
        int j;
        for (j = 0; argv[i][j] != '-'; j++) {
            //check if the node count is a valid number
            if (strchr("0123456789", argv[i][j]) == NULL) {
                ERR_EXIT("node count is invalid");
            } else {
                x = x * 10 + argv[i][j] - '0';
            }
        }
        if (max < x) {
            max = x;
        }
        x = 0;
        for (int k = j + 1; argv[i][k] != '\0'; k++) {
            //check if the node count is a valid number
            if (strchr("0123456789", argv[i][k]) == NULL) {
                ERR_EXIT("node count is invalid");
            } else {
                x = x * 10 + argv[i][k] - '0';
            }
        }
        if (max < x) {
            max = x;
        }
    }

    unsigned int graphMatrix[max + 1][max + 1];

    //initialize graph matrix with zeros
    for (int i = 0; i <= max; i++) {
        for (int j = 0; j <= max; j++) {
            graphMatrix[i][j] = 0;
        }
    }
    for (int i = 1; i < argc; i++) {
        int v = 0;
        int u = 0;
        int j;
        for (j = 0; argv[i][j] != '-'; j++) {
            v = v * 10 + argv[i][j] - '0';
        }
        for (int k = j + 1; argv[i][k] != '\0'; k++) {
            u = u * 10 + argv[i][k] - '0';
        }
        //check if the edges between nodes are not given correctly
        if (v == u) {
            ERR_EXIT("edge between the same node is not allowed");
        }
        //set the edge between node v and u
        graphMatrix[v][u] = 1;
        graphMatrix[u][v] = 1;
    }

    //open (already created) semaphores
    sem_t *sem_pst = sem_open(SEM_PST, 0);
    if (sem_pst == SEM_FAILED) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("sem_open SEM_PST");
    }
    sem_t *sem_rd = sem_open(SEM_RD, 0);
    if (sem_rd == SEM_FAILED) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("sem_open SEM_RD");
    }
    sem_t *sem_sync = sem_open(SEM_SYNC, 0);
    if (sem_sync == SEM_FAILED) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("sem_open SEM_SYNC");
    }

    colors graphColors[max + 1];

    while (myshm->state != 1) {
        //initiate a copy of the original graph
        unsigned int graphCopy[max + 1][max + 1];
        for (int i = 0; i <= max; i++) {
            for (int j = 0; j <= max; j++) {
                graphCopy[i][j] = graphMatrix[i][j];
            }
        }

        //assign to each node a random color of the enum colors
        //by using the random generator
        for (int i = 0; i <= max ; i++) {
            int random = rand() % 10;
            if (random >= 0) {
                if(random < 3) {
                    graphColors[i] = RED;
                }
            }
            if(random >= 3) {
                if(random < 6) {
                    graphColors[i] = BLUE;
                }
            }
            if(random >= 6) {
                if (random <= 9) {
                    graphColors[i] = GREEN;
                }
            }
        }

        int count = 0;
        int x = 0;
        unsigned int solution[MAX_LENGTH * 2];
        for (int i = 0; i < MAX_LENGTH * 2; i++) {
            solution[i] = 0;
        }
        //go over graph and randomly assigned colors and check whether
        //to adjacent nodes have the same colors. if so remove the edge between
        //these nodes (i-j)
        for (int i =0; i <= max ; i++) {
            for (int j = i; j <= max; j++) {
                if(graphCopy[i][j] == 1) {
                    if(graphColors[i] == graphColors[j]) {
                        solution[x++] = i;
                        solution[x++] = j;
                        graphCopy[i][j] = 0;
                        graphCopy[j][i] = 0;
                        count++;
                        //if a solution has more than 8 edges terminate
                        if(count > MAX_LENGTH) {
                            break;
                        }
                    }
                }
            }
            //if a solution has more than 8 edges terminate
            if(count > MAX_LENGTH) {
                break;
            }
        }
        //if a solution has more than 8 edges terminate
        if(count > MAX_LENGTH) {
            break;
        }

        if(sem_wait(sem_sync) == -1) {
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_wait SEM_SYNC");
        }

        if(sem_wait(sem_pst) == -1) {
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_wait SEM_PST");
        }

        for(int i = 0; i < x; i++) {
            myshm->solution[myshm->writeIndex].edges[i] = solution[i];
        }

        myshm->solution[myshm->writeIndex].numberOfEdges = count;
        myshm->writeIndex = (myshm->writeIndex + 1) % MAX_LENGTH;

        if(sem_post(sem_pst) == -1) {
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_post SEM_PST");
        }

        if(sem_post(sem_rd) == -1) {
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_post SEM_RD");
        }

        if(sem_post(sem_sync) == -1) {
            shm_unlink(SHM_NAME);
            sem_close_all(2, sem_rd, sem_pst);
            sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
            ERR_EXIT("sem_post SEM_SYNC");
        }
    }

    //cleanup semaphores
    if (sem_close(sem_pst) == -1) {
        shm_unlink(SHM_NAME);
        sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
        ERR_EXIT("sem_close SEM_PST");
    }
    if (sem_close(sem_rd) == -1) {
        shm_unlink(SHM_NAME);
        sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
        ERR_EXIT("sem_close SEM_RD");
    }
    if (sem_close(sem_sync) == -1) {
        shm_unlink(SHM_NAME);
        sem_unlink_all(3, SEM_RD, SEM_PST, SEM_SYNC);
        ERR_EXIT("sem_close SEM_SYNC");
    }

    //cleanup shared memory object
    if (munmap(myshm, sizeof(*myshm)) == -1) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("munmap");
    }

    //close SHM file descriptor
    if (close(shmfd) == -1) {
        shm_unlink(SHM_NAME);
        ERR_EXIT("close");
    }

    return 0;
}