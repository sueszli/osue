#ifndef CIRCBUFF_H   /* Include guard */
#define CIRCBUFF_H
#include <stdbool.h>
#include <semaphore.h>

#define MAX_DATA (1024)

sem_t *sem_free;
sem_t *sem_used;
sem_t *sem_mutex; 

struct circ_buff
{
	bool sv_stopped;
	int rd_index;
	int wr_index;
	char solutions[MAX_DATA];
};

struct node {
    int color;
    int node_ind;
};

struct edge {
    int node_ind_1;
    int node_ind_2;
};

struct graph
{
	int nodes_size;
	struct node *nodes; 
    struct edge *edges;
};



struct circ_buff *create_buff(bool is_sv);

int close_buff(bool is_sv, struct circ_buff *circ_buff);

int write_circ_buff(struct circ_buff *circ_buff, char *content);

char *read_circ_buff(struct circ_buff *circ_buff);

#endif
