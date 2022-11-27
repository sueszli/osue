/**
 * @file generator.c
 * @author Marcel Boros e11937377@student.tuwien.ac.at
 * @date 12.11.2021
 *
 * @brief Main generator module.
 * 
 * This program is generating feedback-arc-sets for a graph specified as input using a shared memory.
 **/
 
#include "generator.h"

/**
 * @brief This function has an infinite loop and tries to find a solution for a graph until an interrupt occurs. After initializing
 * in every loop, the solution is written into a shared memory. The solution respectively the input is of the form
 * "a-b", where a and b are the nodes of an edge in the graph. Generally, this program works in cooperation with a supervisor
 * program, which handles all the solutions. The IO-operations on the shared memory are working with shared semaphores
 * together with the supervisor. There are 3 semaphore, one for used-space, one for free-space and one handles the write-end
 * (mutex). Furthermore, solutions are written into a circular buffer inside the shared memory with a  fixed size.
 * Consequently, only solutions smaller than 8 edges are accepted.
 * @param argc The argument counter
 * @param argv The argument vector
 * @return On success this function regularly ends the process
 */


int main(int argc, char**argv) {
    
    if(argc == 1) {
        //No program arguments specified
        fprintf(stderr, "Error in %s: No edges specified\n", argv[0]);
    }
    
    
    int fd = openSharedMemory();
    //supervisor already has to have created the shared memory, otherwise
    //shm_open would have thrown an error already
    sem_t* free_sem;
    sem_t* used_sem;
    sem_t* mutex_sem;
    struct myshm* myshm = mapAddress(fd);
    
    
    
    //infinit loop for finding solutions
    while(1==1) {
        
        //Begin: read-in solution
        char** edges = malloc((argc-1)*sizeof(char*));
        readEdges(edges, argc, argv);
        char* nodes = malloc (sizeof(char) * argc*2);
        memset(nodes, 0, argc);
        
        //create more edges-arrays
        char** cpy_edges = malloc((argc-1)*sizeof(char*));
        char** cpy_edges2 = malloc((argc-1)*sizeof(char*));
        cpyStrArray(cpy_edges,edges,argc-1);
        cpyStrArray(cpy_edges2,edges,argc-1);
        createNodes(cpy_edges,nodes, argc);   
        
        //permutate
        FisherYates(nodes);
        
        //select edges
        char** chosenEdges = malloc((argc-1)*sizeof(char*));
        memset(chosenEdges, 0, argc);
        int size = solution(argc, nodes, chosenEdges, cpy_edges2, edges);
        
        
        //free all allocated memory
        free(cpy_edges2);
        free(cpy_edges);
        free(nodes);
        free(edges);
        
        
        //End: read-in solution
    
        
    
        //terminate if variable is set from supervisor
        if((myshm->terminate == 1)) {
            closeSHM_gen(myshm, fd, free_sem, mutex_sem, used_sem);
            return EXIT_SUCCESS;
        }
    
        //open semaphores for generator
        if((free_sem = sem_open(S1, O_RDWR)) == SEM_FAILED) {
            fprintf(stderr, "Error at generator.c : sem_open : %s\n", strerror(errno));
        }
    
        if((used_sem = sem_open(S2, O_RDWR)) == SEM_FAILED) {
            fprintf(stderr, "Error at generator.c : sem_open : %s\n", strerror(errno));
        }
    
        if((mutex_sem = sem_open(S3, O_RDWR)) == SEM_FAILED) {
            fprintf(stderr, "Error at generator.c : sem_open : %s\n", strerror(errno));
        }
    

        //write to shared memory
        writeSHM(myshm, chosenEdges, size, free_sem, mutex_sem, used_sem);
    }
}



//closing shared memory process for the generator
void closeSHM_gen(struct myshm* myshm, int fd, sem_t* free_sem, sem_t* mutex_sem, sem_t* used_sem) {
    
    //close semaphores
    if((sem_close(free_sem) == -1) || (sem_close(used_sem) == -1) 
    || (sem_close(mutex_sem) == -1)) {
            
        fprintf(stderr, "Error at generator.c : sem_close : %s\n", strerror(errno));
    }
    
    
    //release mapping for generator
    if(munmap(myshm, CIRCULAR_BUFFER_SIZE) == -1) {
        fprintf(stderr, "Error at generator.c : munmap : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    //close shared memory object for generator
    if(close(fd) == -1) {
        fprintf(stderr, "Error at generator.c : close : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}



struct myshm* mapAddress(int fd) {
    
    struct myshm* myshm;
    
    //map to virtual memory of the process
    if((myshm = mmap(NULL, CIRCULAR_BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        fprintf(stderr, "Error at supervisor.c : mmap : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    return myshm;
}


int openSharedMemory() {
    int fd;
    if((fd = shm_open(SHM_NAME, O_RDWR, 0600)) == -1) {
        fprintf(stderr, "Error at generator.c : shm_open : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

//FisherYatesalgorithm for creating a permutation of the nodes
void FisherYates(char* nodes) {
    int n = strlen(nodes), j, tmp;
    for(int i=n-1; i>0; --i) {
        j = rand() % (i+1);
        tmp = nodes[j];
        nodes[j] = nodes[i];
        nodes[i] = tmp;
    }
}


//write solution into the shared memory
void writeSHM(struct myshm* myshm, char** chosenEdges, int size, sem_t* free_sem, sem_t* mutex, sem_t* used) {
    
    if(size <= 8) {
        //solution has less than 8 edges -> write into circular buffer
        
        //decrement free space semaphore
        if(sem_wait(free_sem) == -1) {
            fprintf(stderr, "Error at generator.c : sem_wait : %s\n", strerror(errno));
        }
        
        
        //exclusive access to write end of the circular buffer with mutex
        if(sem_wait(mutex) == -1) {
            fprintf(stderr, "Error at generator.c : sem_wait : %s\n", strerror(errno));
        }
        
        int idx = 0;
        for(int i=0; i<size; ++i) {
            
            for(int k=0; k<strlen(chosenEdges[i]); ++k) {
                //write edges into circular buffer
                myshm->data[myshm->writeEnd][idx++] = chosenEdges[i][k];
            }
            
            
            myshm->data[myshm->writeEnd][idx++] = ' ';
            ++idx;
        }
        
        myshm->writeEnd = ((myshm->writeEnd) + 1);
        myshm->writeEnd = ((myshm->writeEnd) % ((int)ENTRIES));
        
        
        if(sem_post(mutex) == -1) {
            fprintf(stderr, "Error at generator.c : sem_post : %s\n", strerror(errno));
        }
        
        //write is complete...
        
        //increment used space semaphore
        if(sem_post(used) == -1) {
            fprintf(stderr, "Error at generator.c : sem_post : %s\n", strerror(errno));
        }
    }
    
    
}


//create a copy of a string array
void cpyStrArray(char** a, char** b, int size) {
    for(int i=0; i<size; ++i) {
        a[i] = malloc(sizeof(char)*4);
        memcpy(a[i], b[i], sizeof(4));
    }
}


//read-in all edges of the argument vector
void readEdges(char** edges, int argc, char** argv) {
    int idx = 1;
    //read in edges
    while(idx < argc) {
        edges[idx-1] = argv[idx];
        ++idx;
    }
}


//selecting nodes out of the given edges
void createNodes(char** edges, char* nodes, int argc) {
    char* node1, *node2, *n1_occ, *n2_occ;
    char* substr1 = malloc (sizeof(char) * argc*2), *substr2 = malloc (sizeof(char) * argc*2);
    
    for(int i=0; i<argc-1; ++i) {
        node1 = strtok(edges[i],"-");     //edges are of the form "x-y", where x and y are the two nodes
        node2 = strtok(NULL, "-");
        n1_occ = strstr(nodes, node1);    //first occurrence of node1
        n2_occ = strstr(nodes, node2);    //first occurrence of node2
        if(n1_occ == NULL) {
            //if node hasn't been read yet
            strcat(nodes, node1);
        } else {
            substr1 = strcpy(substr1, n1_occ+sizeof(char)*2);
            if((strlen(node1)==1) && (*(n1_occ+sizeof(char)) == *node1) && (strstr(substr1,node1) == NULL))  {
                //if node with twice the digit has been read, but not the node with one digit itself
                strcat(nodes, node1);
            }
        }
        
        if(n2_occ == NULL) {
            strcat(nodes ,node2);
        } else {
            substr2 = strcpy(substr2, n2_occ+sizeof(char)*2);
            if((strlen(node2)==1) && (*(n2_occ+sizeof(char)) == *node2) && (strstr(substr2,node2) == NULL)) {
                strcat(nodes,node2);
            }
        }
    }
    
    free(substr1);
    free(substr2);
}


//creating a feedback-arc-set
int solution(int argc, char* nodes, char** chosenEdges, char** cpy_edges2, char** edges) {
    int pos1, pos2, idx = 0;
    char* node1, *node2, *n1_occ, *n2_occ;
    
    for(int i=0; i<argc-1; ++i) {
        
        node1 = strtok(cpy_edges2[i],"-");
        node2 = strtok(NULL, "-");
        
        n1_occ = strstr(nodes, node1);
        n2_occ = strstr(nodes, node2);
        
        pos1 = n1_occ - nodes;
        pos2 = n2_occ - nodes;
        if(pos2 < pos1) {
            //put edge into feedback arc set
            chosenEdges[idx++] = edges[i];
        }
    }
    
    //return size of solution (edge-count)
    return idx-1;
    
}

