#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_SIZE 50;
#define SHM_NAME "/myshm"
#define SEM_ME "/mutual_exclusion"
#define SEM_WRITE "/write"

struct T_File {
  int[MAX_SIZE] puffer;
  int writePosition;
  int countElem;
}

struct T_File myshm *;

int put(const int data) {
  if (myshm->countElem == MAX_SIZE) {
    return -1;
  }
  myshm->writePosition = (writePosition + 1) % MAX_SIZE;
  myshm->puffer[writePosition] = data;
  myshm->countElem++;
  return myshm->countElem;
}

int get(int *data) {
  if (myshm->countElem == 0) {
    return -1;
  }
  data = myshm->puffer[writePosition];
  myshm->writePosition = (writePosition - 1) % MAX_SIZE;
  myshm->countElem--;
  return myshm->countElem;
}

int main(argc, char *argv[]) {}