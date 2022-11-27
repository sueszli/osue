#include "supervisor.h"
struct semaphores{
	sem_t *used;
	sem_t *free;
	sem_t *mutex;
};
struct line{
	int i;
	int j;
};
struct solution{
	int numberEdges;
	line_t line[MAXLINE];
};
struct updateCircular{
	bool quit;
	int writepos;
	int readpos;
	solution_t solution[MAXLINE];
};
volatile sig_atomic_t quit = 0;
static updateC_t *circBuf;
static sema_t *semaphores;
void handle_signal(int signal) { quit = 1; }
sema_t *createSema(void){
	sema_t *temp = malloc(sizeof(sema_t));
	if(temp == NULL){
		fprintf(stderr,"error in creating semaphores");
		exit(EXIT_FAILURE);
	}
	if((temp -> free = sem_open(SEM_FREE,O_CREAT,0600,CIRCULARBUFFER)) == SEM_FAILED){
		fprintf(stderr,"error in creating free semaphores");
		exit(EXIT_FAILURE);
	}
	if((temp -> used = sem_open(SEM_USED,O_CREAT,0600,0)) == SEM_FAILED){
		fprintf(stderr,"error in creating used semaphores");
		exit(EXIT_FAILURE);
	}
	if((temp -> mutex = sem_open(SEM_MUTEX,O_CREAT,0600,1)) == SEM_FAILED){
		fprintf(stderr,"error in creating used semaphores");
		exit(EXIT_FAILURE);
	}
	return temp;
}
updateC_t *createCircular(void){
	updateC_t *circular;
	int sharedmem = shm_open(SHM_OP,O_RDWR | O_CREAT,0600);
	if(sharedmem == -1){
		fprintf(stderr,"error in opening shared memory");
		exit(EXIT_FAILURE);
	}
	if(ftruncate(sharedmem,sizeof(updateC_t))<0){
		fprintf(stderr,"error in setting the size of the shared memory");
		exit(EXIT_FAILURE);
	}
	circular = mmap(NULL,sizeof *circular,PROT_READ | PROT_WRITE,MAP_SHARED,sharedmem,0);
	if(circular == MAP_FAILED){
		fprintf(stderr,"error in mapping shared memory");
		exit(EXIT_FAILURE);
	}
	if(close(sharedmem) == -1){
		fprintf(stderr,"error in closing shared memory");
		exit(EXIT_FAILURE);
	}
	return circular;
}
void usage(char *c){
    fprintf(stderr, "Usage: %s\n", c);
    exit(EXIT_FAILURE);
}
void free_resources(updateC_t *circular,sema_t *semaphores){
    if (munmap(circular, sizeof *circular) == -1) {
        fprintf(stderr, "Unmap shared memory");
    }
    if (shm_unlink(SHM_OP) == -1) {
        fprintf(stderr, "Unlink shared memory");
    }
	if(sem_close(semaphores -> free) == -1){
		fprintf(stderr,"error in closing semaphores");
	}
	if(sem_close(semaphores -> used) == -1){
		fprintf(stderr,"error in closing semaphores");
	}
	if(sem_close(semaphores -> mutex) == -1){
		fprintf(stderr,"error in closing semaphores");
	}
	if(sem_unlink(SEM_FREE) == -1){
		fprintf(stderr,"error in unlink semaphores");
	}
	if(sem_unlink(SEM_USED) == -1){
		fprintf(stderr,"error in unlink semaphores");
	}
	if(sem_unlink(SEM_MUTEX) == -1){
		fprintf(stderr,"error in unlink semaphores");
	}
	free(semaphores);
}
void free_resources2(void){
	free_resources(circBuf,semaphores);
}
	
int main(int argc,char** argv){
	char* filename = argv[0];
	if(argc > 1){
		usage(filename);
	}
	if(atexit(free_resources2)!= 0){
		fprintf(stderr,"error in cleaning resources");
		exit(EXIT_FAILURE);
	}
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    circBuf = createCircular();
    semaphores = createSema();
    
    solution_t currSolution;
    
    memset(&currSolution, 0, sizeof(solution_t));
    currSolution.numberEdges = MAXLINE;
    
    while (!quit) {

        if (sem_wait(semaphores->used) == -1) {
			if (errno == EINTR) continue;
            fprintf(stderr,"error in semaphore wait");
			exit(EXIT_FAILURE);
        }
        if (sem_wait(semaphores->mutex) == -1) {
			if (errno == EINTR) continue;
            fprintf(stderr,"error in semaphore wait");
			exit(EXIT_FAILURE);
        }
        
        solution_t solution = circBuf->solution[circBuf->readpos & (MAXLINE - 1)];
        circBuf->readpos++;
        
        if (sem_post(semaphores->free) == -1) {
            fprintf(stderr,"error in semaphore post ");
			exit(EXIT_FAILURE);
        }
        if (solution.numberEdges < currSolution.numberEdges) {
            if (solution.numberEdges == 0) {
                printf("The graph is acyclic\n");
                quit = 1;
            } else {
                printf("Solution with %d edges: ", solution.numberEdges);
                for (int i = 0; i < solution.numberEdges; i++) {
                    line_t line = solution.line[i];
                    printf("%d-%d ", line.i, line.j);
                }
                printf("\n");
            }
            currSolution = solution;
        }
        if (quit) {
            circBuf->quit = true;
        }
        if (sem_post(semaphores->mutex) == -1){
			fprintf(stderr,"error in semaphore post ");
			exit(EXIT_FAILURE);
		}
    }
    return EXIT_SUCCESS;
	
	
}