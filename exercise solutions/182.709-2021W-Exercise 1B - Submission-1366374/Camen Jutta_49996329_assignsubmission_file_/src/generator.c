/**
@file generator.c
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 12.11.2021

@brief program entry point
@details generates random feedback arc sets

synopsis: generator EDGE1...
    e.g.: generator 0-1 1-2 2-0
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> //getopt

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <semaphore.h>
#include <time.h>

#include "configuration.h"

/*
inputs:
for i in {1..10}; do (./generator 0-1 1-2 2-0 &); done
./generator 0-1 1-2 1-3 1-4 2-4 3-6 4-3 4-5 6-0
./generator 1-4 1-5 2-6 3-4 3-6 4-5 6-0 6-5

./generator 0-2 0-9 0-11 1-4 3-2 3-6 4-2 4-9 5-2 5-11 6-2 6-4 7-2 7-4 7-5 7-8 7-16 7-17 8-9 8-12 8-17 10-2 10-9 11-2 12-1 12-6 12-10 13-5 13-6 13-8 14-4 14-12 15-8 15-11 15-13 16-1 16-6 16-17 17-6 17-10 17-11 18-7 18-8 18-11
for i in {1..10}; do (./generator 0-2 0-9 0-11 1-4 3-2 3-6 4-2 4-9 5-2 5-11 6-2 6-4 7-2 7-4 7-5 7-8 7-16 7-17 8-9 8-12 8-17 10-2 10-9 11-2 12-1 12-6 12-10 13-5 13-6 13-8 14-4 14-12 15-8 15-11 15-13 16-1 16-6 16-17 17-6 17-10 17-11 18-7 18-8 18-11 &); done
*/

/**
@author Jutta Camen <e01452468@student.tuwien.ac.at>
@date 14.11.2021
@details program entry point, generates random feedback arc sets of given set of nodes,
writes removed set of edges to circular
buffer, global variables: quit

@brief program entry point

@param argc count of strings in argv
@param argv[] strings
@return returns EXIT_SUCCESS on success, else EXIT_FAILURE
*/
int main(int argc, char *argv[]) {
    char *prog_name=argv[0];

    //ÜBERPRÜFEN, DASS KEINE OPTIONEN ÜBERGEBEN*/
    int c;
    while ( (c = getopt(argc, argv, "")) != -1 ){ //argv ist aufruf als array
        switch ( c ) {
            default: /* invalid option */
                fprintf(stderr, "[%s:%d] ERROR: wrong input-unknown option, synopsis generator EDGE1... :\n", prog_name, __LINE__);
                exit(EXIT_FAILURE);
                break;
            }
    }
    
    //SPEICHER FÜR EDGES UND NODES ALLOCIEREN UND BELEGEN
    //no options - progam name in first argv
    if(optind==0){
        optind=1;
    }
    int numberEdges=argc-optind;
    //überprüfen, ob mindestens eine Kante übergeben
    if(numberEdges==0){
        fprintf(stderr, "[%s:%d] ERROR: wrong input-at least 1 edge needed, synopsis generator EDGE1... : \n", prog_name, __LINE__);
        exit(EXIT_FAILURE);
    }
    int *edges = malloc(2*numberEdges*sizeof(*edges));
    if (edges==NULL){
        fprintf(stderr, "[%s:%d] ERROR: malloc edges failed: %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    int numberNodes = 0;
    for (int i=0;i<numberEdges;++i){
        char *edge = argv[optind+i];
        int length = strlen(edge);
        char *minus = strstr(edge, "-");
        
        //wenn Anfang oder Ende edge keine zahl ist
        if (!isdigit(edge[0]) || !isdigit(edge[length-1])){
            fprintf(stderr, "[%s:%d] ERROR: wrong input-edge should be int-int, synopsis generator EDGE1... : %s\n", prog_name, __LINE__, edge);
            exit(EXIT_FAILURE);
        }
        if (minus==NULL){
            fprintf(stderr, "[%s:%d] ERROR: wrong input-edge should be int-int (did not find '-'), synopsis generator EDGE1... : %s\n", prog_name, __LINE__, edge);
            exit(EXIT_FAILURE);
        }
        
        *minus = '\0';
        char *end;
        int a = strtol(edge, &end, 10);
        int b = strtol(minus+1, &end, 10);
        
        edges[2*i]=a;
        edges[2*i+1]=b;
        if (a+1>numberNodes){
            numberNodes = a+1;
        }
        if (b+1>numberNodes){
            numberNodes = b+1;
        }
    }
    
    //OPEN SHARED MEMORY, SEMAPHORE
    int shmfd = shm_open(SHM_NAME, O_RDWR, 0600); //(0600) User kann lesen und schreiben
    if (shmfd == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not open shared memory : %s\n", prog_name, __LINE__, strerror(errno));
        free(edges);
        exit(EXIT_FAILURE);
    }
    
    // map shared memory object:
    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: mmap failed : %s\n", prog_name, __LINE__, strerror(errno));
        free(edges);
        exit(EXIT_FAILURE);
    }
    
    // semaphore
    sem_t *sem_write = sem_open(SEM_WRITE, 0);
    if (sem_write==SEM_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: could not create semaphore write : %s\n", prog_name, __LINE__, strerror(errno));
        free(edges);
        exit(EXIT_FAILURE);
    }
    sem_t *sem_read = sem_open(SEM_READ, 0);
    if (sem_read==SEM_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: could not create semaphore read : %s\n", prog_name, __LINE__, strerror(errno));
        free(edges);
        exit(EXIT_FAILURE);
    }
    sem_t *sem_excl = sem_open(SEM_EXCL, 0);
    if (sem_excl==SEM_FAILED){
        fprintf(stderr, "[%s:%d] ERROR: could not create semaphore exclusive : %s\n", prog_name, __LINE__, strerror(errno));
        free(edges);
        exit(EXIT_FAILURE);
    }
    
    //for creating random orderings
    int *ordering = malloc(numberNodes * sizeof(int));
    for(int i=0; i<numberNodes; ++i) {
        ordering[i] = i;
    }

    int edgesRemove[2*MAX_REMOVE+1];
    srand(time(0)); //damit unterschiedliche Generatoren unterschiedliche Ergebnisse haben, time(0)=jetzige Zeit in sekunden seit 1970
    while(myshm->quit == 0){
        //RANDOM ORDERING OF NODES
        for (int i=numberNodes-1;i>=0;--i){
            int j = rand()%numberNodes;

            int temp = ordering[i];
            ordering[i] = ordering[j];
            ordering[j] = temp;
        }
        
        //LIST OF EDGES TO BE REMOVED
        int numberEdgesRemoved=0; //kommt dann das doppelte rein
        for (int i=0; i<numberEdges;i++){
            //remove
            if (ordering[edges[2*i]]>ordering[edges[2*i+1]]){
                if (numberEdgesRemoved>=MAX_REMOVE){
                    break;
                }

                edgesRemove[2*numberEdgesRemoved+1]=edges[2*i];
                edgesRemove[2*numberEdgesRemoved+2]=edges[2*i+1];
                ++numberEdgesRemoved;
            }
        }
        //more edges than willing to remove - find another one
        if(numberEdgesRemoved>=MAX_REMOVE){
            continue;
        }
        
        edgesRemove[0] = numberEdgesRemoved;
        
        //WRITE SOLUTION TO CIRCULAR BUFFER
        if (sem_wait(sem_write) == -1){ //warte dass ein generator schreibt
            fprintf(stderr, "[%s:%d] ERROR: sem_wait(sem_write) failed : %s\n", prog_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (sem_wait(sem_excl) == -1){ //warte dass ein generator schreibt
            fprintf(stderr, "[%s:%d] ERROR: sem_wait(sem_excl) failed : %s\n", prog_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        int writePos = myshm->writePos;
        memcpy(myshm->data[writePos], edgesRemove, (2*numberEdgesRemoved+1)*sizeof(int));
        myshm->writePos = (writePos+1)%MAX_DATA;
        
        if (sem_post(sem_excl) == -1){ //warte dass ein generator schreibt
            fprintf(stderr, "[%s:%d] ERROR: sem_post(sem_excl) failed : %s\n", prog_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        } 
        if (sem_post(sem_read) == -1){ //warte dass ein generator schreibt
            fprintf(stderr, "[%s:%d] ERROR: sem_post(sem_read) failed : %s\n", prog_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    //act as though everything read
    if(myshm->quit == 1) {
        if (sem_post(sem_write) == -1){ // gib ein schreiben frei
            fprintf(stderr, "[%s:%d] ERROR: sem_post(sem_write) failed : %s\n", prog_name, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    free(edges);
    free(ordering);
    
    //CLOSE SHARED MEMORY, SEMAPHORE
    // unmap shared memory:
    if (munmap(myshm, sizeof(*myshm)) == -1){
        fprintf(stderr, "[%s:%d] ERROR: munnmap shared memory failed: %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // close shared memory
    if (close(shmfd) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close shared memory : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    // close semaphores
    if (sem_close(sem_write) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close semaphore_write : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_read) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close semaphore_read : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (sem_close(sem_excl) == -1){
        fprintf(stderr, "[%s:%d] ERROR: could not close semaphore_excl : %s\n", prog_name, __LINE__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    printf("generator finished\n");
    return EXIT_SUCCESS;
}
