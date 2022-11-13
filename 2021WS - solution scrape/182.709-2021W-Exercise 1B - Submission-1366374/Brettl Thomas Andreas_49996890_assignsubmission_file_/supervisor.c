#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include "graph.h"

struct myshm {
	int readPos;
	int writePos;
	char data[MAX_ENTRIES][MAX_EDGES][4];
};

void shmError() {
	printf("Error creating or removing shared memory!\n");
	exit(1);
}

void semError() {
	printf("Error creating or removing semaphore!\n");
	exit(1);
}

int main() {
	int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0600);
	
	if(fd == -1)
		shmError();
	
	if(ftruncate(fd,sizeof(struct myshm)) == -1)
		shmError();

	struct myshm *myshm;
	myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(myshm == MAP_FAILED)
		shmError();

	if(close(fd) == -1)
		shmError();

	sem_t *semFree = sem_open(SEM_FREE, O_CREAT | O_EXCL, 0600, MAX_ENTRIES);
	sem_t *semUsed = sem_open(SEM_USED, O_CREAT | O_EXCL, 0600, 0);
	sem_t *semWrite = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0600, 1);

	myshm->readPos = 0;
	myshm->writePos = 0;

	char ch;
	scanf("%c",&ch);

	if(sem_close(semFree) == -1)
		semError();
	if(sem_close(semUsed) == -1)
		semError();
	if(sem_close(semWrite) == -1)
		semError();

	if(sem_unlink(SEM_FREE) == -1)
		semError();
	if(sem_unlink(SEM_USED) == -1)
		semError();
	if(sem_unlink(SEM_WRITE) == -1)
		semError();

	if(munmap(myshm, sizeof(*myshm)) == -1)
		shmError();

	if(shm_unlink(SHM_NAME) == -1)
		shmError();
}
