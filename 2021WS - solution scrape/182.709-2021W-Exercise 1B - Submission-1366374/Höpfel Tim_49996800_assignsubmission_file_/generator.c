/**
 * @file generator.c
 * @author Tim Höpfel Matr.Nr.: 01207099 <e01207099@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Generator of solutions for an feedback-arc-set for a graph in the input and delivers it to a running supervisor-process.
 *
 * The generator program takes a graph as input. The program repeatedly generates a random solution
 * to the problem as described on the first page and writes its result to the circular buffer. It repeats this
 * procedure until it is notified by the supervisor to terminate.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <sys/types.h>

#define SHM_NAME "/01207099_arcset"
#define MAX_SOLUTION (8)
#define CIRC_BUFFER_SIZE (1024)
#define SEM_FREE "/01207099_sem_free"
#define SEM_USED "/01207099_sem_used"
#define SEM_WRITE "/01207099_sem_write"

static char *myprog; /** The Program name. */

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: myprog
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s EDGE1...", myprog);
	exit(EXIT_FAILURE);
}

/**
 * Static function to determin the decimal digits of a positive long.
 * @brief counts the digits of a positive long.
 * @details The program recursivly calls itself. Every time, the argument gets divided by 10, to make it one digit smaller.
 * In the end of the programm, the number of charakters read and written and the compression ration are printed to sterr.
 * @param number Number, that digits should be counted.
 * @return int, representing the numbers of digits of a long. Returns 1 for a long with the value of 0.
 */
static int numberofdezdigits(long number) {
    if(number > 9) {
        return 1 + numberofdezdigits((number / 10));
    } else {
        return 1;
    }
}

struct edge
{
    long start;
    long end;
};
struct arcset
{
    int size;
    struct edge solution[MAX_SOLUTION];
};
struct arcshm {
    int state;
    unsigned int wr_pos;
    struct arcset arcsets[CIRC_BUFFER_SIZE];
};

/**
 * Program entry point.
 * @brief The program starts here. This function generates  solutions for an feedback-arc-set for a graph in the input
 * @details The program opens a existing shm and maps it. It opens semaphores to communicate with the supervisor. The input-graph is beeing stored in an arcset-struct. Then, the generator
 * creates a random array of numbers, from 0 to the highest value of vercite-numbers. The base for randomness is the getpid(3)-function.
 * All edges (s, e) for which s > e in the ordering is false are ersed from the input-graph. The remaining edges form a feedback arc set, that is beeing stored in the shared memory, if it is
 * small enough, to be a good solution.
 * @param argc The argument counter.
 * @param argv The argument vector. Has to consist of a graph, consisting of edges. e.g.: 0-1 1-2 1-3 1-4
 * @return Returns EXIT_SUCCESS.
 */
int main (int argc, char *argv[]) {
//set up shared memory
    //create shared memory object
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
    if (shmfd == -1) {
        fprintf(stderr, "shm_open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // set the size of the shared memory:
    if (ftruncate(shmfd, sizeof(struct arcshm)) < 0) {
        fprintf(stderr, "ftruncate failed: %s\n", strerror(errno));
        close(shmfd);
        exit(EXIT_FAILURE);
    }
    // map shared memory object:
    struct arcshm *arcshm;
    arcshm = mmap(NULL, sizeof(*arcshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0); //0 offset
    if (arcshm == MAP_FAILED) {
        fprintf(stderr, "mmap failed: %s\n", strerror(errno));
        close(shmfd);
        exit(EXIT_FAILURE);
    }
    if (close(shmfd) == -1) {
        arcshm->state = (-1);
        munmap(arcshm, sizeof(*arcshm));
        fprintf(stderr, "close failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //open semaphores to communicate with supervisor
    sem_t *s_free = sem_open(SEM_FREE, 0); //equals CIRCULAR_BUFFER_SIZE in the beginning
    sem_t *s_used = sem_open(SEM_USED, 0); //eqals 0 in the beginning
    sem_t *s_write = sem_open(SEM_WRITE, 0); //equals 1 in the beginning

    if(s_free == SEM_FAILED || s_used == SEM_FAILED || s_used == SEM_FAILED) {
        sem_close(s_free); sem_close(s_used); sem_close(s_write);
        arcshm->state = (-1);
        munmap(arcshm, sizeof(*arcshm));
        fprintf(stderr, "sem_open failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //store name of program for usage-message
    myprog = argv[0];
    // search for a ? to trigger usage-message
    int c; //saves return of getopt
    while((c = getopt(argc, argv, "")) != -1 ){
        switch(c) {
            case '?':
                usage();
        }
    }
    //read input edges to array
    int number_of_edges = (argc - optind);
    assert(number_of_edges > 0);


   //stores the edges in an array
    struct edge edges[number_of_edges];

    //for strtol
    char *notnumbers;
    //store every edge in edges by looping through argv
    //store highest value;
    long max_vertice = 0;

    for(int i = 1; i <= number_of_edges; i++){
        long start_of_edge = strtol(argv[i], &notnumbers, 10);

        long end_of_edge;
        int digits_of_start_of_edge = numberofdezdigits(start_of_edge);
        //get string after the first number and the minus character
        end_of_edge = strtol(argv[i]+digits_of_start_of_edge+1, &notnumbers, 10);

        //only positive values for vertices are allowed
        assert(start_of_edge >= 0);
        assert(end_of_edge >= 0);
        assert(start_of_edge != end_of_edge);
//store in edges startin with edges[0]
        edges[i-1].start = start_of_edge;
        edges[i-1].end = end_of_edge;

        if(max_vertice < start_of_edge) {
            max_vertice = start_of_edge;
        }
        if(max_vertice < end_of_edge) {
            max_vertice = end_of_edge;
        }
    }

    // at least one other vertice has to exist to make up a edge
    assert(max_vertice >= 1);

//find solution
    //set random seed with current processId.
    srand(getpid());
    //generate random array filled with zeros, size is max value + a place for the 0 value
    long random_longs[max_vertice + 1];
    //long random_longs_size = max_vertice;
    for(int i = 0; i<=max_vertice; i++) {
        random_longs[i] = -1;
    }
    //search a pseudo random place for every value but the highest (will be added to the last empty place)
    for(int i = 0; i<max_vertice; i++) {
        int newpos;
        bool found = false;
        while(!found) {
        //if value at that place is neg, search new place
            int random_number = rand();
            newpos = random_number % (max_vertice+1);
            //if the position is empty place the value there, if not, try next position (if not allready at the last position)
            if(random_longs[newpos] < 0) {
                random_longs[newpos] = i;
                found = true;
            } else if (newpos < max_vertice) {
                if(random_longs[newpos+1] < 0) {
                    random_longs[newpos+1] = i;
                    found = true;
                }
            }
        }
    }
    //place last value in the last empty slot of the array
    for(int j = 0; j<=max_vertice; j++) {
        if(random_longs[j] < 0) {
            random_longs[j] = max_vertice;
            break;
        }
    }
//delete all edges ( u, v ) for which u > v in the ordering is false, by setting their values to -1; count the number of edges remaining in feedback_arc_set_size
    int feedback_arc_set_size = 0;
    for(int i = 0; i < number_of_edges; i++) {
        //locate edge[i].start
        int pos_edge_start;
        for(int j = 0; j <= max_vertice; j++) {
            if(edges[i].start == random_longs[j]) {
                pos_edge_start = j;
                break;
            }
        }
        int pos_edge_end;
        for(int j = 0; j <= max_vertice; j++) {
            if(edges[i].end == random_longs[j]) {
                pos_edge_end = j;
                break;
            }
        }
        if(pos_edge_start > pos_edge_end) {
            feedback_arc_set_size++;
        } else {
            edges[i].start = (-1);
            edges[i].end = (-1);
        }
    }
    //if the solution is to bad, meaning the feedback_arc is bigger than MAX_SOLUTION, end with success
    if(feedback_arc_set_size > MAX_SOLUTION) {
        fprintf(stderr, "solution is too bad.\n");
        return EXIT_SUCCESS;
    }

    //collect remaining edges in feedback_arc_set
    struct arcset feedback_arc_set;
    int temp_int_arc_set = 0;
    feedback_arc_set.size = feedback_arc_set_size;
    for(int i = 0; i < number_of_edges; i++) {
        if(edges[i].start != (-1) && edges[i].end != (-1)) {
            feedback_arc_set.solution[temp_int_arc_set].start = edges[i].start;
            feedback_arc_set.solution[temp_int_arc_set].end = edges[i].end;
            temp_int_arc_set++;
            assert(temp_int_arc_set <= feedback_arc_set_size);
        }
    }

    //check status, -1 means stop
    if(arcshm->state > 0) {
        //wait for opputunity to write
        sem_wait(s_write);
        //wait for empty position in circular buffer
        sem_wait(s_free);
        //write solution
        arcshm->arcsets[arcshm->wr_pos] = feedback_arc_set;
        arcshm->arcsets[arcshm->wr_pos].size = feedback_arc_set.size;
        memcpy(arcshm->arcsets[arcshm->wr_pos].solution, feedback_arc_set.solution, sizeof(feedback_arc_set.solution));
        //the solution can now be read
        sem_post(s_used);
        //increment position, circulate to start if position exceeds CIRC_BUFFER_SIZE
        arcshm->wr_pos += 1;
        arcshm->wr_pos %= CIRC_BUFFER_SIZE;
        //other generators can now write their solution
        sem_post(s_write);
    } else {
        fprintf(stderr, "arcshm->state is smaller than 0. Nothing written to supervisor!");
    }


//unlink all shared resources
    // unmap shared memory:
    if (munmap(arcshm, sizeof(*arcshm)) == -1) {
        fprintf(stderr, "munmap failed: %s\n", strerror(errno));
    }

    // remove shared memory object:
     if(sem_close(s_free) == -1) {
        fprintf(stderr, "sem_close of s_free failed: %s\n", strerror(errno));
    }
    if(sem_close(s_used) == -1) {
        fprintf(stderr, "sem_close of s_used failed: %s\n", strerror(errno));
    }
    if(sem_close(s_write) == -1) {
        fprintf(stderr, "sem_close of s_write failed: %s\n", strerror(errno));
    }

    //exit
    return EXIT_SUCCESS;
}





