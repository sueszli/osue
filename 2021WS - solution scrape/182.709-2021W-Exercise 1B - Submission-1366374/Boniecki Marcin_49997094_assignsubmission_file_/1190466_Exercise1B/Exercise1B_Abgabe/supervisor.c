#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#define SHM_NAME "fb_arc_set"
#define MAX_EDGE_NUMBER (20)

#define SEM_SUPERVISOR_STARTS "/sem_supervisor_starts"
#define SEM_ALT_G_C_1 "/sem_alt_g_c_1"
#define SEM_ALT_G_C_2 "/sem_alt_g_c_2"

struct edge {
    unsigned int start_node;
    unsigned int end_node;
};

typedef struct edge edge_t;

/**
 * @brief struct for the shared memory
*/
struct fb_shm {
    unsigned int state;
    edge_t feedback_arcs[MAX_EDGE_NUMBER];
};

typedef struct fb_shm fb_shm_t;

/**
 * @brief Program name as a static string variable
 */
static char *program_name;

/**
 * @brief Variable that saves the best feedback arc
 */
edge_t best_feedback_arc[MAX_EDGE_NUMBER];

void reading_loop(fb_shm_t *shm, sem_t *s_alt_1, sem_t *s_alt_2);
static void exit_and_print_error(char error_message[]);

int main(int argc, char *argv[]) {
    printf("supervisor\n");

    program_name = argv[0];

    //semaphore that tells the generator if the supervisor has been executed and initiated the shared memory
    sem_t *s_sup = sem_open(SEM_SUPERVISOR_STARTS, O_CREAT | O_EXCL, 0600, 0);
    sem_t *s_alt_1 = sem_open(SEM_ALT_G_C_1, O_CREAT | O_EXCL, 0600, 1);
    sem_t *s_alt_2 = sem_open(SEM_ALT_G_C_2, O_CREAT | O_EXCL, 0600, 0);

    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1) {
        exit_and_print_error("Failed to open shared memory");
    }

    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct fb_shm)) < 0) {
        exit_and_print_error("Failed to ftruncate memory");
    }

    fb_shm_t *shm;
    shm = mmap(NULL, sizeof(*shm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    if (shm == MAP_FAILED) {
        exit_and_print_error("Failed to map the memory");
    }

    sem_post(s_sup);

    reading_loop(shm, s_alt_1, s_alt_2);

    // unmap shared memory:
    if (munmap(shm, sizeof(*shm)) == -1) {
        exit_and_print_error("Failed to unmap shared memory");
    }

    // remove shared memory object:
    if (shm_unlink(SHM_NAME) == -1) {
        exit_and_print_error("Failed to remove shared memory object");
    }

    //close the file descriptor
    if (close(shmfd) == -1) {
        exit_and_print_error("Failed to close the file descriptor");
    }

    sem_close(s_sup);
    sem_close(s_alt_1);
    sem_close(s_alt_2);

    sem_unlink(SEM_SUPERVISOR_STARTS);
    sem_unlink(SEM_ALT_G_C_1);
    sem_unlink(SEM_ALT_G_C_2);

    exit (EXIT_SUCCESS);
}

void reading_loop(fb_shm_t *shm, sem_t *s_alt_1, sem_t *s_alt_2) {
    while(1) {
        sem_wait(s_alt_2);


        sem_wait(s_alt_1);
    }
}

static void exit_and_print_error(char error_message[]) {
    fprintf (stderr, "%s: %s: %s\n", program_name, error_message, strerror (errno));
	exit (EXIT_FAILURE);
}