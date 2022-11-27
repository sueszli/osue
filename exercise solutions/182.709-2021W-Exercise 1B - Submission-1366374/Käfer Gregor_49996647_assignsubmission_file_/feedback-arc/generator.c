#include "generator.h"

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
static updateC_t *circBuf ;
static sema_t *semaphores;
void usage(char *c){
    fprintf(stderr, "Usage: %s EDGE1.....", c);
    exit(EXIT_FAILURE);
}
void handle_signal(int signal) { quit = 1; }

void saveEdges(char *input,line_t *line,char *filename){
	char *delim = "-";
	char *temp ;
	int iBool = 0;
	temp = strtok(input,delim);
	while(temp != NULL){
		if(isalpha(temp[0])!= 0){
			printf("its a char");
			usage(filename);
		}
		else{
			if(!iBool){
				sscanf(temp,"%d",&(line -> i));
				iBool = 1;
			}
			else{
				sscanf(temp,"%d",&(line -> j));
			}
		}
		temp = strtok(NULL,delim);
	}
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
	circular = mmap(NULL,sizeof(updateC_t),PROT_WRITE | PROT_READ,MAP_SHARED,sharedmem,0);
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
sema_t *createSema(void){
	sema_t *temp = malloc(sizeof(sema_t));
	if(temp == NULL){
		fprintf(stderr,"error in creating semaphores");
		exit(EXIT_FAILURE);
	}
	if((temp -> free = sem_open(SEM_FREE,0)) == SEM_FAILED){
		fprintf(stderr,"error in creating free semaphores");
		exit(EXIT_FAILURE);
	}
	if((temp -> used = sem_open(SEM_USED,0)) == SEM_FAILED){
		fprintf(stderr,"error in creating used semaphores");
		exit(EXIT_FAILURE);
	}
	if((temp -> mutex = sem_open(SEM_MUTEX,1)) == SEM_FAILED){
		fprintf(stderr,"error in creating used semaphores");
		exit(EXIT_FAILURE);
	}
	return temp;
}
void free_resources(updateC_t *circular,sema_t *semaphores){
	if (munmap(circular, sizeof *circular) == -1) {
        fprintf(stderr, "Unmap shared memory");
    }
	if(sem_close(semaphores->free) == -1){
		fprintf(stderr,"error in closing semaphores");
	}
	if(sem_close(semaphores->used) == -1){
		fprintf(stderr,"error in closing semaphores");
	}
	if(sem_close(semaphores ->mutex) == -1){
		fprintf(stderr,"error in closing semaphores");
	}
}
void free_resources2(void){
	free_resources(circBuf,semaphores);
}
	
void randomPermutation1(int *permutationArray,line_t *line,int *index){
	for(int i = 0;i<*index;i++){
		if(permutationArray[i] == (line -> i)){
			return;
		}
	}
	permutationArray[*index] = line -> i;
	*index = *index+1;
}
void randomPermutation2(int *permutationArray,line_t *line,int *index){
	for(int i = 0;i<*index;i++){
		if(permutationArray[i] == (line -> j)){
			return;
		}
	}
	permutationArray[*index] = line -> j;
	*index = *index+1;
}
void randomPermutation3(int permutationArray[],int index){
	srand(time(NULL));
	for(int i = index-1;i>0;i--){
		int randomize = rand() % (i+1);
		swap(&permutationArray[i],&permutationArray[randomize]);
	}
}
void swap (int *a, int *b) 
{ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
} 
void getSolution(line_t *line,int *arr,solution_t *solution,int arrLength,int inputEdgesLength){
	int count = 0;
	for(int i = 0;i<inputEdgesLength;i++){
		if(count > MAXLINE){
			solution -> numberEdges = MAXLINE;
			return;
		}
		for(int j = 0;i<arrLength;j++){
			line_t *temp =  &line[i];
			if(arr[j] == temp -> i){
				solution->line[count] = *temp;
				count++;
				break;
			}
			else if(arr[j] == temp -> j){
				break;
			}
			else{
				continue;
			}
		}
		if(i == inputEdgesLength -1){
			break;
		}
	}
	solution -> numberEdges = count;
}
int main(int argc,char** argv){
	char *filename = argv[0];
	if(argc < 2){
		usage(filename);
	}
	line_t *line = malloc(sizeof(line_t) * (argc-1));
	int *permutationArray = malloc(sizeof(int) *2 *(argc-1));
	solution_t *solution = malloc(sizeof(solution_t)*(argc-1));
	if(atexit(free_resources2) != 0){
		fprintf(stderr,"error in registering free resources");
		exit(EXIT_FAILURE);
	}
	int count = 0;
	int index = 0;
	for(int i = 1;i<argc;i++){
		saveEdges(argv[i],&line[count],filename);
		count++;
	}
	struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
	circBuf = createCircular();
	semaphores = createSema();
	for(int i = 0;i<count;i++){
		randomPermutation1(permutationArray,&line[i],&index);
	}
	for(int i = 0;i<count;i++){
		randomPermutation2(permutationArray,&line[i],&index);
	}
	while (!quit) {
		randomPermutation3(permutationArray,index);
		getSolution(line,permutationArray,solution,index,count);
        if (sem_wait(semaphores->free) == -1) {
            fprintf(stderr,"Wait failed");
			exit(EXIT_FAILURE);
        }

        if (sem_wait(semaphores->mutex) == -1) {
			fprintf(stderr,"Wait failed");
			exit(EXIT_FAILURE);  
		}        
        if (circBuf->quit) {
            quit = 1;
			free(permutationArray);
			free(line);
			free(solution);
        }
        circBuf->solution[circBuf->writepos & (CIRCULARBUFFER - 1)] = *solution;
        circBuf->writepos++;
        
        if (sem_post(semaphores->mutex) == -1){
            fprintf(stderr,"error in Semaphore post");
			exit(EXIT_FAILURE);
        }
        if (sem_post(semaphores->used) == -1){
            fprintf(stderr,"error in Semaphore post");
			exit(EXIT_FAILURE);
        }
    }
	return EXIT_SUCCESS;
	
}