/**
 * @name: supervisor.h
 * @author: Moritz Hammerer, 11902102
 * @date: 08.11.2021
 */

#define SHM_NAME "/shm11902102"
#define MAX_EDGES 8
#define BUFFER_SIZE 20
#define WRITESEM "/sem1_11902102"
#define READSEM "/sem2_11902102"
#define PERMSEM "/sem3_11902102"

struct edge {
    int u;
    int v;
};

struct dataEntry {
    int count;
    struct edge edges[MAX_EDGES];
};

struct myshm {
    unsigned int state;
    unsigned int wr_pos;
    unsigned int currentMin;
    struct dataEntry data[BUFFER_SIZE];
};