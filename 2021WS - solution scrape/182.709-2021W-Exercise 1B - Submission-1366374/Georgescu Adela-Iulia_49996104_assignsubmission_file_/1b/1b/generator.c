/**
* @author Adela-Iulia Georgescu, 11810894
* @date 02-11-2021
* @brief generates random 3 colorings of a given graph and sends them into shared memory
* @details the supervisor that provides shared memory and semaphores must be started
*
**/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>

#define CIRCULARBUFFFERSIZE 516


//an edge is between 2 nodes: "to" and "from"
typedef struct edge{
    int to;
    int from;
}edge;

char* myProgram;
volatile sig_atomic_t quit = 0;
int shmfd;
void* memory;
sem_t *spaceSem;
sem_t *usedSem;
sem_t *mutex;

//@brief processes every edge to make sure it is written correctly
static void processArguments(char* arg, edge* edge);

//@brief generates a random color for every node
static void randomColor(int* coloredNodes, int len);

//@brief chooses the edges to be removed
static int chooseEdges(edge* edges, edge* chosenEdges, int edgeCount, int* coloredNodes);

//@brief prints an error message
static void error(char* message);

//@brief closes and unmaps the resources used in the program and then exits with exitvalue
static void exit_save(int exitvalue);

//@brief sets the global value to 1 when the right signal is received
static void handle_signal();

int main(int argc, char *argv[]){
    myProgram = argv[0];
    int edgeNr = argc - 1;

    //signal handling setup
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = &handle_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    edge edges[edgeNr];
    int curr = 1;
    while(curr < argc){
        processArguments(argv[curr], &edges[curr - 1]);
        curr++;
    }

    //find how many nodes there are
    int maxNode = -1;	
	for(int i = 0; i < edgeNr; i++){
		if(edges[i].from > maxNode) maxNode = edges[i].from;
		if(edges[i].to > maxNode) maxNode = edges[i].to;
	}
	int nodeNr = maxNode + 1;
    int graphComplete = (nodeNr * (nodeNr-1)) /2;
    if(edgeNr > graphComplete){
        error("there are too many edges");
    }

    //set up shm
	shmfd = shm_open("/11810894_shm3col", O_RDWR, 511);
	if(shmfd<0){
		error("shm_open failed");
	}
	errno = 0;
	memory = mmap(NULL, 512, PROT_WRITE | PROT_READ | PROT_EXEC,MAP_SHARED,shmfd,0);
	if(errno!=0){
		error("mmap failed");
	}

    //set up semaphores
	spaceSem = sem_open("/11810894_space", 0, 511,CIRCULARBUFFFERSIZE/8);
	if(spaceSem == SEM_FAILED){
		error("sem_open failed");
	}
	usedSem = sem_open("/11810894_used", 0, 511,0);
	if(spaceSem == SEM_FAILED){
		error("sem_open failed");
	}
	mutex = sem_open("/11810894_mutex", 0, 511,1);
	if(spaceSem == SEM_FAILED){
		error("sem_open failed");
	}
    //If random numbers are generated with rand() without first calling srand(), 
    //your program will create the same sequence of numbers each time it runs
    srand(time(0) * getpid());

    //the generation of solutions is started
    while(!quit){

        
        int coloredNodes[nodeNr];
        randomColor(&coloredNodes[0], nodeNr);

        //chose all edges (u-v) that for which the color of u is identical to the color of v
        edge chosenEdges[8];
        int nrchosenEdges = chooseEdges(edges, chosenEdges, edgeNr, coloredNodes);
        char solution[100];
        if(nrchosenEdges < 0){
            continue;
        }
        else{
            if(nrchosenEdges == 0){
                sprintf(solution, "0");
            }
            else{
                sprintf(solution, "%d: ", nrchosenEdges);
                for(int i = 0; i < nrchosenEdges; i++){
                    char edgeChosen[10]; //just one edge
                    sprintf(edgeChosen, "%d-%d ", chosenEdges[i].from, chosenEdges[i].to);
                    strcat(solution, edgeChosen);
                }
                strcat(solution, "\n");
            }
        }

        //send solution to supervisor
        int sth = sem_wait(mutex);
        if(sth < 0){
            error("sem_wait(mutex) failed");
        }

        //write the solution to the circular buffer
        char* circularMem = (char*) memory;
        int* current = (int*) memory + CIRCULARBUFFFERSIZE;
        int* flag = (int*) memory + CIRCULARBUFFFERSIZE + 4;

        if(*flag == 1){
            sem_post(mutex);
            exit_save(EXIT_SUCCESS);
        }

        //write the current solution to the buffer if there is spaceSem, else wait
        for(int i = 0; i <= strlen(solution); i++){
            sth = sem_wait(spaceSem);
            if(sth == -1){
                if(errno == EINTR){
                    exit_save(EXIT_SUCCESS);
                }
                error("sem_wait(spaceSem) failed");
            }
            circularMem[*current] = solution[i];
            *current = (*current + 1) % CIRCULARBUFFFERSIZE;
            sth = sem_post(usedSem);
            if(sth == -1){
                error("sem_post(usedSem) failed");
            }
        }

        //signal that mutex is finished, so another generator can enter the critical phase
        sth = sem_post(mutex);
        if(sth == -1){
            error("sem_post(mutex) failed");
        }
    }

    exit_save(EXIT_SUCCESS);
}

static void error(char* message){
	fprintf(stderr, "%s: %s\n",myProgram,message);
	exit_save(EXIT_FAILURE);
}

static void handle_signal(){
	quit = 1;
}

void exit_save(int exitvalue){
  //cleaning and removing

  int var = 0;

  //unmap shared memory
  var = munmap(memory, CIRCULARBUFFFERSIZE + 8); //8 because we chose a limit of 8 edges
  if(var == -1){
    exitvalue = EXIT_FAILURE;
  }
  var = close(shmfd);
  if(var == -1){
    exitvalue = EXIT_FAILURE;
  }

  //closing semaphores
  var = sem_close(spaceSem);
  if(var == -1){
    exitvalue = EXIT_FAILURE;
  }
  var = sem_close(usedSem);
  if(var == -1){
    exitvalue = EXIT_FAILURE;
  }
  var = sem_close(mutex);
  if(var == -1){
    exitvalue = EXIT_FAILURE;
  }

  exit(exitvalue);

}

static void randomColor(int* coloredNodes, int len){

    for(int i = 0; i < len; i++){
        int col = rand() % 3;
        coloredNodes[i] = col;
    }
}

static void processArguments(char* arg, edge* edge){

    if(strlen(arg) < 3){
        error("edges must have the form u-v, where u and v are positive integers");

    }

    char* firstRest;
    errno = 0;
    //strtol converts the char into longint
    edge->from = strtol(arg, &firstRest, 10);
    if(errno != 0){
        error("edges must have the form u-v, where u and v are positive integers");
    }

    if(firstRest[0] != '-'){
        error("edges must have the form u-v, where u and v are positive integers");
    }
    firstRest = &firstRest[1];
    char* secondRest;
    errno = 0;
    edge->to = strtol(firstRest, &secondRest, 10);
    if(errno!=0){
        error("edges must have the form u-v, where u and v are positive integers");
    }

    if(strcmp(secondRest, "")!=0 && strcmp(secondRest, "\n")!=0){
        error("edges must have the form u-v, where u and v are positive integers");
    }

    if(edge->from < 0 || edge-> to < 0){
        error("nodes must be positive integers");
    }

}

static int chooseEdges(edge* edges, edge* chosenEdges, int edgeCount, int* coloredNodes){
    int current = 0;
    for(int i = 0; i < edgeCount; i++){
        int a = edges[i].from;
        int b = edges[i]. to;
        if(coloredNodes[a] == coloredNodes[b]){
            if(current == 8){
                return - 1;
            }
            else{
                chosenEdges[current++] = edges[i];
            }
        }
    }
    return current;
}
