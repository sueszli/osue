/** Feedback Arc Set  
 *	@file generator.c
 *  @author Matthias Grausgruber <e00525708@student.tuwien.ac.at>
 *  @date 13.11.2021
 *  
 *  @brief Generator module.
 *
 *	This program reads directed graphs from the shell and searches
 *  for minimal Feedback Arc Sets. It doesn't compute an exact solution.
 *  The algorithm is very simple and chooses the vertices randomly.
 *  Every possible solution will be transfered to the supervisor application
 *  via a shared memory. Access is handled by semaphores.
 *  
 **/

#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#define SEM_FS "/00525708_free"
#define SEM_US "/00525708_used"
#define SEM_WR "/00525708_write"
#define SHM_NAME "/00525708_myshm"
#define CB_SIZE (50)
#define E_SIZE (8)

static char *pgm_name; /**< The program name. */
volatile __sig_atomic_t quit = 0; /**< Quit-flag for SIGINT shutdown. */

struct edges { int v1, v2; };
struct circularBuffer {
    int size;
    struct edges e[E_SIZE+1]; 
};
struct myshm
{   int wr_pos, rd_pos, active, shutd;
    struct circularBuffer cb[CB_SIZE];
};

/**
 * Print error and exit.
 * @brief This function prints the error-message to the shell in case of an error and ends the program.
 * @details global variables: pgm_name
 */
void errorExit (int line) {
	fprintf (stderr, "%s: error at line #%d: %s\n", pgm_name, line, strerror(errno));
	exit (EXIT_FAILURE);
}

/**
 * Print error (and no exit).
 * @brief This function writes prints the error-message to the shell in case of an error.
 * 		  In difference to errorExit() it doesn't end the program.
 * @details global variables: pgm_name
 */
void errorPrint (int line) {
	fprintf (stderr, "%s: error at line #%d: %s\n", pgm_name, line, strerror(errno));
}

/**
 * What to do in case of SIG-Interrupt.
 * @brief This function decodes the SIGINT number and starts the application shutdown.
 * @details global variables: quit
 */
void handle_signal (int signal) {
    if (signal == SIGINT) {
        printf ("\n\nSIGINT (%i)! Shutdown Feedback Arc Set.\n\n", signal);
        quit = 1;
    } else if (signal == SIGTERM) {
        printf ("\n\nSIGTERM (%i)! Shutdown Feedback Arc Set.\n\n", signal);
        quit = 1;
    }
    else printf ("\n\nSIGINT code: %i; Signal ignored.\n\n", signal);
}

/**
 * Checks the substring: digit or not
 * @brief This function checkes the entered edges.
 * @details return: succesfull
 */
int check_digit (char *vertice) {
    for (int j=0; j < strlen(vertice); j++) {
        if( isdigit (vertice[j]) == 0) {
            fprintf (stderr, "Usage: %s [0-9]+[-][0-9]+ [...]\n", pgm_name);
            quit = 1;
            return -1;
        }
    }
    return 0;
}

/**
 * Program entry point.
 * @brief The main program starts here. This function reads all vectors from *argv.
 * Shared memory and semaphores are opened (managed by supervisor). The search for
 * possible solutions beginns. Every Feedback Arc Set is written to the shm (syncronised
 * by semaphores). In case of a shutdown (communicated by the supervisor) the
 * algorithm will be terminated, reserved memory will be freed and the semaphores and
 * shared memory will be disconnected.
 * 
 * @details The Semaphore API helps by synchronizing the shared memory access.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {

    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction (SIGINT, &sa, NULL);

    int count_e = 0, size_e = E_SIZE, max = 0, count_s = 0;
    struct edges *e = malloc (sizeof (e) * size_e);
    struct circularBuffer sol, best_sol;
    best_sol.size = __INT_MAX__;

    pgm_name = argv [0];

    /** Join semaphores created by supervisor. */
    sem_t *s_free = sem_open (SEM_FS, 0);
    if (s_free == SEM_FAILED) errorExit(__LINE__);
    sem_t *s_used = sem_open (SEM_US, 0);
    if (s_used == SEM_FAILED) errorExit(__LINE__);
    sem_t *s_write = sem_open (SEM_WR, 0);
    if (s_write == SEM_FAILED) errorExit(__LINE__);

    /** Join shared memory created by supervisor. */
    int shmfd = shm_open (SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1) errorExit(__LINE__);
    struct myshm *m = mmap (NULL, sizeof(*m), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (m == MAP_FAILED) errorExit(__LINE__);

    /** Read edges from argv input. Create edge-array. */
    for (int i = 1; i < argc; i++) {
        if (strstr(argv [i], "-") != NULL) {
            char *s_v1 = strtok (argv[i], "-");
            if (check_digit (s_v1) == -1) break;
            e[i-1].v1 = strtol (s_v1, NULL, 10);
            if (e[i-1].v1 > max) max = e[i-1].v1;

            char *s_v2 = strtok (NULL, "-");
            if (check_digit (s_v2) == -1) break;
            e[i-1].v2 = strtol (s_v2, NULL, 10);;
            if (e[i-1].v2 > max) max = e[i-1].v2;

            //printf("%i: %i->%i\n", (i-1), e[i-1].v1, e[i-1].v2);
            count_e++;
        }
        if (count_e >= size_e) {
            size_e *= 1.5;
            e = realloc (e, sizeof (e) * size_e);
        }
    }

    /* create vertices-array
    int vert[max+1][max+1], count_v[max+1];
    for (int i=0; i<=max; i++) {
        count_v[i] = 0;
        for (int j=0; j<= max; j++) vert[i][j] = -1;
    }
    for (int i=0; i<count_e; i++) vert[e[i].v1][count_v[e[i].v1]++] = e[i].v2;

    for (int i=0; i<=max; i++) {
        printf ("\n Vert %i:", i);
        for (int j=0; (j<=max) && (vert[i][j] >= 0); j++) {
            printf (" ->%i", vert[i][j]);
        }
    } */

    m->active++;
    int v_random [max+1];

    while ((m->shutd == 0) && (quit == 0)) {
        
        /** Generate random order of vertices. */
        for (int i=0; i<=max; i++) v_random[i] = i;
        for (int i=max; i>0; i--) {
            int r = (rand()*e[i-1].v1+e[i-1].v2) % (i+1);
            int temp = v_random[i];
            v_random[i] = v_random[r];
            v_random[r] = temp; 
        }

        //for (int j=0; j<=max; j++) printf ("%i ", v_random[j]);
        //printf ("\n");
        
        /** Generate possible solution. */
        count_s = 0;
        // Lösung mit edge-array
        for (int i=0; i<count_e; i++) {
            int v1_r = 0, v2_r = 0;
            for (int j=0; j<=max; j++) {
                if (v_random[j] == e[i].v1) v1_r = j;
                if (v_random[j] == e[i].v2) v2_r = j;
                if ((v1_r != 0) && (v2_r != 0)) break;
            }
            if (v1_r > v2_r) {
                //printf ("(%i->%i) ", e[i].v1, e[i].v2);
                if (count_s < E_SIZE ) sol.e[count_s++] = e[i];
                else count_s++;
            }
        }
        //printf ("\n");

        /** For easier handling the size of the solution is stored seperatly.
         *  The end is marked by '-1' in 'v1'
         */
        sol.size = count_s;
        if (count_s < E_SIZE) sol.e[count_s].v1 = -1;
        else sol.e[E_SIZE].v1 = -1;

        /** Just for information: local best solution. */
        if (sol.size < best_sol.size) {
            best_sol = sol;
            printf ("Solution Size: %i {", best_sol.size);
            for (int i=0; (best_sol.e[i].v1 >= 0) && (i < E_SIZE); i++) printf (" (%i->%i)", best_sol.e[i].v1, best_sol.e[i].v2);
            printf (" }\n");
        }

        //printf ("\n\n");

        /** Write solution to circular buffer. 
         *  s_free -> free space in circular buffer
         *  s_write -> only one generator should write at a time
        */
        if (sem_wait (s_free) == -1) errorPrint(__LINE__);
        if (sem_wait (s_write) == -1) errorPrint(__LINE__);
        m->cb[m->wr_pos] = sol;
        m->wr_pos++;
        m->wr_pos %= CB_SIZE;
        if (sem_post (s_used) == -1) errorPrint(__LINE__);
        if (sem_post (s_write) == -1) errorPrint(__LINE__);

        if (sol.size == 0) quit = 1;
    }

    /** Shutdown measurements. */
    m->active--;
    if (sem_post (s_free) == -1) errorPrint(__LINE__);

    free (e);
    if (close(shmfd) == -1) errorPrint (__LINE__);
    if (sem_close (s_free) == -1) errorPrint (__LINE__);
    if (sem_close (s_used) == -1) errorPrint (__LINE__);
    if (sem_close (s_write) == -1) errorPrint (__LINE__);
    
    return EXIT_SUCCESS;
}
