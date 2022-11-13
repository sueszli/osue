/**
 * @file generator.c
 * @author Gernot Hahn 01304618
 * @date 14.11.2021
 *
 * @brief This program starts a generator, which is part of a feedback arc set
 * algorithm.
 *
 * @details This program starts generator. It opens shared memory
 * (circular buffer) and semaphores to be used by the supervisor and generators.
 * After shared ressources are setup, the generator generates random, not necessarily
 * minimal, feedback arc sets and writes them to the buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "graph.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>
#include <semaphore.h>
#include <time.h>

char *prog_name;

/**
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: prog_name
 */
static void usage(void) {
	fprintf(stderr, "usage: %s EDGE1...\n", prog_name);
	exit(EXIT_FAILURE);
}

/**
 * @brief This function starts the generator.
 * @details The function parses arguments. Shared memory and semaphores are initialized.
 * It then generates random, not necessarily minimal, feedback arc sets and writes
 * them to the circular buffer. This process is continued until the supervisor sets
 * the shared quit flag. Before terminating, the function cleans up all ressources.
 * global variables: prog_name
 * @param argc argument counter.
 * @param argv argument vector.
 * @return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]) {

	prog_name = argv[0];
	int c;

	// argument parsing
	while ((c = getopt(argc, argv, "")) != -1) {
		switch (c) {
			case '?':
			default:
				usage();
		}
	}

	int edge_count = argc - optind;

	if (edge_count < 1) {
		usage();
	}

	edge_t graph[edge_count];
	int vertex_count = 0;

	for (int i = 0, j = edge_count; j > 0; i++) {
		int vertices[2];
		if ((parse_edge(argv[optind++], vertices)) == -1) {
			err_handler(prog_name, "edge parsing failed");
			exit(EXIT_FAILURE);
		}
		if (vertices[0] > vertex_count) {
			vertex_count = vertices[0];
		}
		if (vertices[1] > vertex_count) {
			vertex_count = vertices[1];
		}
		graph[i].v1 = vertices[0];
		graph[i].v2 = vertices[1];
		j--;
	}
	vertex_count++;

	// open shared memory
	int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
	if (shmfd == -1) {
		err_handler(prog_name, "failed to open shared memory");
		exit(EXIT_FAILURE);
	}
	
	// memory mapping
	myshm_t *myshm;
	myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	if (myshm == MAP_FAILED) {
		if (close(shmfd) == -1) {
			err_handler(prog_name, "failed to close shared memory file descriptor");
		}
		err_handler(prog_name, "failed to map memory");
		exit(EXIT_FAILURE);
	}

	if (close(shmfd) == -1) {
		err_handler(prog_name, "failed to close shared memory file descriptor");
		if (munmap(myshm, sizeof(*myshm)) == -1) {
			err_handler(prog_name, "failed to unmap memory");
		}
		exit(EXIT_FAILURE);
	}

	// open semaphores
	sem_t *sem_used = sem_open(SEM_1, 0);
	sem_t *sem_free = sem_open(SEM_2, 0);
	sem_t *sem_mutex = sem_open(SEM_3, 0);
	if (sem_used == SEM_FAILED || sem_free == SEM_FAILED || sem_mutex == SEM_FAILED) {
		err_handler(prog_name, "failed to open semaphore");
		if (munmap(myshm, sizeof(*myshm)) == -1) {
			err_handler(prog_name, "failed to unmap memory");
		}
		if (sem_close(sem_used) == -1 || sem_close(sem_free) == -1 || sem_close(sem_mutex) == -1) {
			err_handler(prog_name, "failed to close semaphore");
		}
		exit(EXIT_FAILURE);
	}

	// program logic
	srand(time(NULL));
	while (!myshm->quit) {
		bool valid = true;
		fb_arc_set set;
		int set_counter = 0;
		int permutation[vertex_count];
		permutate(permutation, vertex_count);

		for (int i = 0; i < edge_count; i++) {
			int u = graph[i].v1;
			int v = graph[i].v2;

			int index_u;
			int index_v;

			for (int j = 0; j < vertex_count; j++) {
				if (permutation[j] == u) {
					index_u = j;
				}
				if (permutation[j] == v) {
					index_v = j;
				}
			}

			if (index_u > index_v) {
				if (set_counter == MAX_SET) {
					valid = false;
					break;
				}
				set.edges[set_counter++] = graph[i];
			}
		}

		if (valid) {
			if (set_counter != MAX_SET) {
				set.edges[set_counter].v1 = -1;
				set.edges[set_counter].v2 = -1;
			}
			if (sem_wait(sem_free) == -1) {
				err_handler(prog_name, "sem_wait(sem_free) failed");
				break;
			}
			if (sem_wait(sem_mutex) == -1) {
				err_handler(prog_name, "sem_wait(sem_mutex) failed");
				break;
			}
			myshm->cbuf[myshm->wr_pos] = set;
			if (sem_post(sem_used) == -1) {
				err_handler(prog_name, "sem_post(sem_used) failed");
				break;
			}
			myshm->wr_pos += 1;
			myshm->wr_pos %= CBUF_SIZE;
			if (sem_post(sem_mutex) == -1) {
				err_handler(prog_name, "sem_post(sem_mutex) failed");
				break;
			}
		}
	}
	
	// memory unmapping
	if (munmap(myshm, sizeof(*myshm)) == -1) {
		err_handler(prog_name, "failed to unmap memory");
	}

	// cleanup semaphores
	if (sem_close(sem_used) == -1 || sem_close(sem_free) == -1 || sem_close(sem_mutex) == -1) {
		err_handler(prog_name, "failed to close semaphore");
	}

	return EXIT_SUCCESS;
}

