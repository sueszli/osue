/**
 * @file supervisor.c
 * @author Gernot Hahn 01304618
 * @date 14.11.2021
 *
 * @brief This program starts the supervisor, which is part of a feedback arc set
 * algorithm.
 *
 * @details This program starts the supervisor. It initializes shared memory
 * (circular buffer) and semaphores to be used by the supervisor and generators.
 * After the intial setup, the supervisor waits for generators to write feedback
 * arc sets into the shared memory and checks them for the best solution.
 */

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "util.h"
#include "graph.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <stdbool.h>
#include <semaphore.h>

char *prog_name;
volatile sig_atomic_t *quit;

/**
 * @brief This function is a signal handler, which set the value quit
 * is pointing to, to 1.
 * @details global variables: quit
 * @param signal code
 */
static void handle_signal (int signal) {
	*quit = 1;
}

/**
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: prog_name
 */
static void usage(void) {
	fprintf(stderr, "usage: %s \n", prog_name);
	exit(EXIT_FAILURE);
}

/**
 * @brief This function starts the supervisor.
 * @details The function parses arguments and defines singal handling for SIGINT
 * and SIGTERM. Shared memory and semaphores are initialized. It then waits for 
 * new solutions written to the circular buffer and checks them. This process is
 * continued until either a solution with zero edges is found (acyclic graph)
 * or a SIGINT or SIGTERM signal is received. Before terminating, ressources the
 * function cleans up all ressources.
 * global variables: prog_name, quit
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

	if ((argc - optind) != 0 ) {
		usage();
	}

	// signal handling
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);

	// shared memory initialization
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (shmfd == -1) {
		err_handler(prog_name, "failed to open shared memory");
		exit(EXIT_FAILURE);
	}

	if (ftruncate(shmfd, sizeof(myshm_t)) < 0) {
		err_handler(prog_name, "failed to truncate shared memory");
		if (close(shmfd) == -1) {
			err_handler(prog_name, "failed to close shared memory file descriptor");
		}
		if (shm_unlink(SHM_NAME) == -1) {
			err_handler(prog_name, "failed to unlink shared memory");
		}
		exit(EXIT_FAILURE);
	}

	// memory mapping
	myshm_t *myshm;
	myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

	if (myshm == MAP_FAILED) {
		err_handler(prog_name, "failed to map memory");
		if (close(shmfd) == -1) {
			err_handler(prog_name, "failed to close shared memory file descriptor");
		}
		if (shm_unlink(SHM_NAME) == -1) {
			err_handler(prog_name, "failed to unlink shared memory");
		}
		exit(EXIT_FAILURE);
	}

	quit = &myshm->quit;
	myshm->quit = 0;
	myshm->wr_pos = 0;

	if (close(shmfd) == -1) {
		err_handler(prog_name, "failed to close shared memory file descriptor");
		if (munmap(myshm, sizeof(*myshm)) == -1) {
			err_handler(prog_name, "failed to unmap memory");
		}
		if (shm_unlink(SHM_NAME) == -1) {
			err_handler(prog_name, "failed to unlink shared memory");
		}
		exit(EXIT_FAILURE);
	}

	// semaphore initialization
	sem_t *sem_used = sem_open(SEM_1, O_CREAT | O_EXCL, 0600, 0);
	sem_t *sem_free = sem_open(SEM_2, O_CREAT | O_EXCL, 0600, CBUF_SIZE);
	sem_t *sem_mutex = sem_open(SEM_3, O_CREAT | O_EXCL, 0600, 1);
	if (sem_used == SEM_FAILED || sem_free == SEM_FAILED || sem_mutex == SEM_FAILED) {
		err_handler(prog_name, "failed to open semaphore");
		if (munmap(myshm, sizeof(*myshm)) == -1) {
			err_handler(prog_name, "failed to unmap memory");
		}
		if (sem_close(sem_used) == -1 || sem_close(sem_free) == -1 || sem_close(sem_mutex) == -1) {
			err_handler(prog_name, "failed to close semaphore");
		}
		if (shm_unlink(SHM_NAME) == -1) {
			err_handler(prog_name, "failed to unlink shared memory");
		}
		if (sem_unlink(SEM_1) == -1 || sem_unlink(SEM_2) == -1 || sem_unlink(SEM_3) == -1) {
			err_handler(prog_name, "failed to unlink semaphore");
		}
		exit(EXIT_FAILURE);
	}

	// program logic
	int rd_pos = 0;
	int lowest = MAX_SET;
	while (!myshm->quit) {
		if (sem_wait(sem_used) == -1) {
			err_handler(prog_name, "sem_wait(sem_used) failed");
			break;
		}
		fb_arc_set set = myshm->cbuf[rd_pos];
		if (sem_post(sem_free) == -1) {
			err_handler(prog_name, "sem_post(sem_free) failed");
			break;
		}
		rd_pos += 1;
		rd_pos %= CBUF_SIZE;
		
		int edges = calculate_edges(&set);
		if (edges == 0) {
			fprintf(stdout, "[%s] The graph is acyclic!\n", prog_name);
			myshm->quit = 1;
			break;
		}

		if (edges < lowest) {
			fprintf(stdout, "[%s] Solution with %d edges:", prog_name, edges);
			for (int i = 0; i < edges; i++) {
				fprintf(stdout, " %d-%d", set.edges[i].v1, set.edges[i].v2);
			}
			fprintf(stdout, "\n");
			lowest = edges;
		}
	}

	// memory unmapping
	if (munmap(myshm, sizeof(*myshm)) == -1) {
		err_handler(prog_name, "failed to unmap memory");
	}

	// shared memory unlinking
	if (shm_unlink(SHM_NAME) == -1) {
		err_handler(prog_name, "failed to unlink shared memory");
	}

	// cleanup semaphores
	if (sem_close(sem_used) == -1 || sem_close(sem_free) == -1 || sem_close(sem_mutex) == -1) {
		err_handler(prog_name, "failed to close semaphore");
	}

	if (sem_unlink(SEM_1) == -1 || sem_unlink(SEM_2) == -1 || sem_unlink(SEM_3) == -1) {
		err_handler(prog_name, "failed to unlink semaphore");
	}

	return EXIT_SUCCESS;
}

