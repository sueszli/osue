/**
 * @file main.c
 * @author Georges Kalmar 9803393
 * @date 11.11.2021
 *
 * @brief Main program module.
 * 
 * This program takes care of getting the arguments and coordinates the functions used
 * to calculate how many edges have to be removed. 
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h> 	
#include <fcntl.h>
#include <semaphore.h>  	
#include <errno.h>

#include "generator.h"


struct myshm{
	unsigned int state;
	unsigned int pos;
	twoNodes_arr_t data[ARRSIZE];
};

/** 
 * @brief Main gets the program arguments and calculates and report which edges of the graph have to be removed to get it three colorable
 * @details Main gets the input arguments from a client in this Synopsis generator [EDGE: a-b...] a,b are int and a<b, e.g. 0-1 0-3 2-3.
 * It runs several checks on the input to ensure that the data is sufficient enough to continue the program properly. It creates an twodimensional
 * array where all edges are stored. First dimension is the number of the edge, second dimension stores number of node1 and node2.
 * Shared Memory, mappings and semaphores are opened (and later closed accordingly) and in a while loop differently colored graphes are created and 
 * edges that have to be removed are reported to an array in the Shared Memory.
 * @param argc Stores the amount of arguments
 * @param argv Stores the text string of the arguments in an array
 * @return The program return EXIT_SUCCESS on success or returns EXIT_FAILURE in case of errors 
 **/
int main(int argc, char* argv[]){
	int n;
	int node1;
	int node2;
	char* endptr;
	char* endptr2;
	int amount_edges;
	int* nodeCol_arr;
	int size_nodeCol_arr = 0;
	twoNodes_t edge_rm_arr[8];
	int size_edge_rm_arr = 0;
	
	//get arguments and create edge arrays
	if(argc < 2){abortProg(argv[0]);}
	amount_edges=(argc-1);
	int edge_arr[amount_edges][2];
	for(int i = 1; i <argc; i ++){
		n = strtol(argv[i],&endptr,10);
		if((n >=0) && (*endptr == '-')&& (strlen(endptr)>1)){
			node1 = n;
			endptr = (endptr+1);
			}
			else {abortProg(argv[0]);}
			
		n = strtol(endptr,&endptr2,10);
		if((n>node1) && (*endptr2 =='\0') ){
			node2 = n;
			}
			else {abortProg(argv[0]);}
		edge_arr[i-1][0]=node1;
		edge_arr[i-1][1]=node2;
		if(node2>size_nodeCol_arr){size_nodeCol_arr = node2;}
	}
	/* needs to be incremented, for Nodes (0,1,2) an array size 3 is needed */
	size_nodeCol_arr +=1;
	nodeCol_arr = calloc(size_nodeCol_arr,sizeof(int));
	if(nodeCol_arr == NULL){
		fprintf(stderr,"%s Error: No Memory available to create nodeCol_arr: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	// sets the seed for different random numbers
	setRandom();
	
	int shmfd = shm_open(SHM_NAME, O_RDWR, 0600);
	if(shmfd == -1){
		fprintf(stderr,"%s Error on opening or creating shared memory: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	struct myshm *myshm;
	myshm = mmap(NULL,sizeof(*myshm),PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if(myshm==MAP_FAILED){
		fprintf(stderr,"%s Error on mmap: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//open semaphores
	sem_t *s1 = sem_open(SEM1, 0);
	sem_t *s2 = sem_open(SEM2, 0);
	sem_t *s3 = sem_open(SEM3, 0);
	if((s1 == SEM_FAILED) || (s2 == SEM_FAILED) || (s3 == SEM_FAILED)){
		fprintf(stderr,"%s Error on opening semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	//coordinates the semaphores and the manages colors and removed edges 
	int i = 0;
	while (1){	
		colorNodeArr(size_nodeCol_arr,nodeCol_arr);
		removeEdges(edge_arr,amount_edges,nodeCol_arr,edge_rm_arr,&size_edge_rm_arr);
		if(sem_wait(s3) == -1){
			fprintf(stderr,"%s Error on sem_wait: %s\n",argv[0],strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(sem_wait(s1) == -1){
			fprintf(stderr,"%s Error on sem_wait: %s\n",argv[0],strerror(errno));
			exit(EXIT_FAILURE);
		}
		i = myshm->pos;
		for(int j = 0; j<size_edge_rm_arr;j++){
			myshm->data[i].arr[j].n1 = edge_rm_arr[j].n1;
			myshm->data[i].arr[j].n2 = edge_rm_arr[j].n2;
		}
		myshm->data[i].size = size_edge_rm_arr;
		
		printf("%s: generator:",argv[0]);		
		for(int j = 0; j<size_edge_rm_arr;j++){
			printf(" %d-%d",edge_rm_arr[j].n1,edge_rm_arr[j].n2);
		}
		printf("\n");
		
		i++;
		myshm->pos = i%ARRSIZE;
		if(sem_post(s2) == -1){
			fprintf(stderr,"%s Error on sem_post: %s\n",argv[0],strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(sem_post(s3) == -1){
			fprintf(stderr,"%s Error on sem_post: %s\n",argv[0],strerror(errno));
			exit(EXIT_FAILURE);
		}
		if(myshm->state == 1){break;}
	}
	
	//sem close and unlink
	if(sem_close(s1) == -1){
		fprintf(stderr,"%s Error on closing semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	} 
	if(sem_close(s2) == -1){
		fprintf(stderr,"%s Error on closing semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(sem_close(s3) == -1){
		fprintf(stderr,"%s Error on closing semaphore: %s\n",argv[0],strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	//close shared memory and unmap
	if(close(shmfd) == -1){
		fprintf(stderr,"%s Error on closing Shared Memory: %s\n",argv[0],strerror(errno));
	}
	if(munmap(myshm,sizeof(*myshm)) == -1){
		fprintf(stderr,"%s Error on munmap: %s\n",argv[0],strerror(errno));
	}
	//free allocated memory
	free(nodeCol_arr);
	
	return EXIT_SUCCESS;
}