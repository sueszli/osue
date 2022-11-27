/// @file coloring.h
/**
 * @author Tobias Jakob 01611148
 * @brief this module is designed to compute the minimal number of edges you have to remove from a graph to make it 3-colorable
 * @date 22.11.2020
*/
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <time.h> 
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h> 
#include <sys/types.h> 

#define SHM_NAME "01611148myshm" 
#define BUF_SIZE (20) /// the size of the circular buffer
#define SEM_MUX "01611148sem_mux"
#define FREE_SPACE "01611148free"
#define USED_SPACE "01611148used"
#define WRITE_POS "01611148wrpos"
#define RUNNING "01611148running"

/**
 * the circular buffer: in shmbuffer[position][0][0] the number of collisions is stored, the vertices of the colliding edges are stored in shmbuffer[position][i][0] and shmbuffer[position][i][1] respectively where i is bigger than 1
*/
struct myshm {
	int shmbuffer[BUF_SIZE][9][2];
};
