/// @file generator.c
#include "arcset.h"

/**
 * @brief if the string has a number after a '-' character with a number returns the number else exits the programm
 * @param str is a string with a number after a '-' character
 * @return the number if successful, else exits the programm
*/

int static getIntOfSndArg(char* str){
	char numberS[strlen(str)];
	int wosamma = 0;
	int numberI;
	char* ptr;
	while(*str != '-'){
		if(*str == '\0'){
			fprintf(stderr,"edges must be divided by a '-' character\n");
			exit(EXIT_FAILURE);
		}
		str++;
	}
	str++;
	while(*str != '\0'){
		numberS[wosamma] = *str;
		wosamma++;
		str++;
	}
	numberS[wosamma] = '\0';
	numberI = strtol(numberS,&ptr,10);
	if (errno == EINVAL){
            fprintf(stderr,"please put numbers as edgedescriptors\n");
            exit(EXIT_FAILURE);
    } 
	return numberI;
}

/**
 * @brief if the string starts with a number returns the number else exits the programm
 * @param str is a string starting with a number
 * @return the number if successful, else exits the programm
*/
int static getIntOfFstArg(char* str){
	char* ptr;
	int numberI = strtol(str,&ptr,10);
	if (errno == EINVAL){
            fprintf(stderr,"please put numbers as edgedescriptors\n");
            exit(EXIT_FAILURE);
    }
	return numberI;
}

/**
 * @brief the generator creates solutions an amount of edges to make a graph 3-colorable
 * @param the parameters are given in the form vertix1OfEdge1-vertix2OfEdge1 vertix1OfEdge2-vertix2OfEdge2 .... for example: generator 0-1 0-2
 *        means we are looking at a graph with 3 vertices and 2 edges 
 * @return EXIT_FAILURE on failure, else EXIT_SUCCESS
*/
int main(int argc, char* argv[]) {
	int returnvalue = EXIT_SUCCESS;
	int maxNode = 0;
	if(argc < 2){
		fprintf(stderr,"please provide one or more Arguments\n");
		exit(EXIT_FAILURE);
	}
	int i;
	int j;
	for(i = 1; i < argc;i++){
		if (getIntOfSndArg(argv[i]) > maxNode)
		   maxNode = getIntOfSndArg(argv[i]);
		else if(getIntOfFstArg(argv[i]) > maxNode)
			maxNode = getIntOfFstArg(argv[i]);
	}
	maxNode++;
	bool edges[maxNode][maxNode];
	for(i = 0;i < maxNode;i++){
		for(j = 0;j < maxNode;j++){
		    edges[i][j] = false;
	    }
	}
	for(i = 1; i < argc;i++){
		edges[getIntOfFstArg(argv[i])][getIntOfSndArg(argv[i])] = true;
	}
	
	// create and/or open the shared memory object:
    int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
    if (shmfd == -1){
    	fprintf(stderr,"error when trying to open filedescriptor of shared memory object\n");
		exit(EXIT_FAILURE);
    }
    
    // map shared memory object:
    struct myshm *myshm;
    myshm = mmap(NULL, sizeof(*myshm), PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (myshm == MAP_FAILED){
    	fprintf(stderr,"error when trying to map shared memory object\n");
		exit(EXIT_FAILURE);
	}
        
    if ((close(shmfd)) == -1){
    	fprintf(stderr,"error when trying to close shared memory object filedescriptor\n");
		exit(EXIT_FAILURE);
	}
	
	// create and/or open the shared memory object:
    int runningfd = shm_open(RUNNING, O_RDWR | O_CREAT, 0600);
    if (runningfd == -1){
    	fprintf(stderr,"error when trying to open filedescriptor of shared memory object\n");
		exit(EXIT_FAILURE);
    }
    
    // map shared memory object:
    bool *running;
    running = mmap(NULL, sizeof(bool),PROT_READ, MAP_SHARED, runningfd, 0);
    if (running == MAP_FAILED){
    	fprintf(stderr,"error when trying to map shared memory object\n");
		exit(EXIT_FAILURE);
	}
        
    if ((close(runningfd)) == -1){
    	fprintf(stderr,"error when trying to close shared memory object filedescriptor\n");
		exit(EXIT_FAILURE);
	}
	
	// create and/or open the shared memory object:
    int whiteposfd = shm_open(WRITE_POS, O_RDWR | O_CREAT, 0600);
    if (whiteposfd == -1){
    	fprintf(stderr,"error when trying to open filedescriptor of shared memory object\n");
		exit(EXIT_FAILURE);
	}
    
    // map shared memory object:
    int *writepos;
    writepos = mmap(NULL, sizeof(int),PROT_WRITE | PROT_READ, MAP_SHARED, whiteposfd, 0);
    if (running == MAP_FAILED){
    	fprintf(stderr,"error when trying to map shared memory object\n");
		exit(EXIT_FAILURE);
	}
        
    if ((close(whiteposfd)) == -1){
    	fprintf(stderr,"error when trying to close shared memory object filedescriptor\n");
		exit(EXIT_FAILURE);
	}
		
	sem_t *s1 = sem_open(SEM_MUX,0);
    sem_t *free_sem = sem_open(FREE_SPACE,0);
    sem_t *used_sem = sem_open(USED_SPACE,0);

	if(s1 == SEM_FAILED || free_sem == SEM_FAILED || used_sem == SEM_FAILED){
		fprintf(stderr,"error when trying to create semaphor %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	int nodes[maxNode];
	int numbers[maxNode];
	for (i=0;i<maxNode;i++){
		numbers[i] = i;
	}
	srand(time(0));
	int collisions[9][2];
	while(*running){
		for (i=0;i<maxNode;i++){
			nodes[i] = numbers[rand() % maxNode];
		}
		for(i=1;i < 9;i++){
			collisions[i][0] = -1;
			collisions[i][1] = -1;
		}
		collisions[0][0] = 0;
		for(i = 0;i < maxNode;i++){
			for(j = i+1;j < maxNode;j++){
				int a = nodes[i];
				int b = nodes[j];
				if(a != b && edges[b][a] == true){
					if(collisions[0][0] == 8){
						continue;
					}
					collisions[0][0]++;
					collisions[collisions[0][0]][0] = i;
					collisions[collisions[0][0]][1] = j;
				}
		    }
		}
		int wrpos;
		while((i = sem_wait(s1)) == -1){
		if (errno == EINTR){
			continue;
		} else {
			fprintf(stderr,"error while waiting %s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
	}


		wrpos = *writepos;
		*writepos = (*writepos) + 1;
		*writepos = (*writepos) % BUF_SIZE;
		sem_post(s1);
		
		while((i = sem_wait(free_sem)) == -1){
			if (errno == EINTR){
				continue;
			} else {
				fprintf(stderr,"error while waiting %s\n",strerror(errno));
				exit(EXIT_FAILURE);
			}
		}
		myshm->shmbuffer[wrpos][0][0] = collisions[0][0];
		for(i=1;i<9;i++){
			myshm->shmbuffer[wrpos][i][0] = collisions[i][0];
			myshm->shmbuffer[wrpos][i][1] = collisions[i][1];
		}
		sem_post(used_sem);
	}
	
	// unmap shared memory:
    if ((munmap(myshm, sizeof(*myshm))) == -1){
    	fprintf(stderr,"error when trying to unmap shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
	
	// unmap shared memory:
    if ((munmap(running, sizeof(bool))) == -1){
    	fprintf(stderr,"error when trying to unmap shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
	
	// unmap shared memory:
    if ((munmap(writepos, sizeof(int))) == -1){
    	fprintf(stderr,"error when trying to unmap shared memory object\n");
		returnvalue = EXIT_FAILURE;
	}
        
	if((sem_close(s1)) < 0){
	    fprintf(stderr,"error when trying to close semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_close(free_sem)) < 0){
	    fprintf(stderr,"error when trying to close semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	if((sem_close(used_sem)) < 0){
	    fprintf(stderr,"error when trying to close semaphor\n");
	    returnvalue = EXIT_FAILURE;
	}
	return returnvalue;
}



