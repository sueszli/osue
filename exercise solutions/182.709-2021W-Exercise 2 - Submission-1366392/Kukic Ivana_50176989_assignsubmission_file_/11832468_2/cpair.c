/**
 * @file cpair.c
 * @author Ivana Kukic - 11832468
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <sys/wait.h>

typedef struct point{
  float x;
  float y;
} point;

static void print_pair(point*);
static void forkLeft(point*, int, point*);
static void forkRight(point*, int, point*);
static void compare(point*, point*);

static int fd1[2];
static int fd2[2];
static int fd3[2];
static int fd4[2];

int right_flag = 0;
int left_flag = 0;
char* prog_name;

int main(int argc, char* argv[]){
  prog_name = argv[0];
  char *line = NULL;
  size_t len = 0;
  int char_num;
  point coordinates;
  int index = 0;
  int size = 1;
  int scan_char;

  point* array = (point*)calloc(size, sizeof(point));

if (argc != 1) {
  fprintf(stderr, "[%s] error: Invalid number of arguments\n", prog_name);
  exit(EXIT_FAILURE);
}

//taking input from stdin and storing it in the array (until EOF is reached)
  while ((char_num = getline(&line, &len, stdin)) != -1) {

        if (index == size) {
          size++;
          array = realloc(array, size*sizeof(struct point));
        }
        float h;
        scan_char = sscanf(line, "%f %f %f", &coordinates.x, &coordinates.y, &h );

        if (scan_char != 2) {
          fprintf(stderr, "[%s] error: Invalid coordinates\n", prog_name);
          exit(EXIT_FAILURE);
        }
        array[index] = coordinates;
        index++;
  }
  if (size == 1)
  {
    exit(EXIT_SUCCESS);
  }
  if (size == 2) {
    fprintf(stdout, "%f %f\n", array[0].x, array[0].y);
    fprintf(stdout, "%f %f\n", array[1].x, array[1].y);

    exit(EXIT_SUCCESS);
  }

//calculating arithmetic mean
  float ar_mean_x;
  float sum = 0;
  for (int i = 0; i < index; i++) {
    sum += array[i].x;
  }
  ar_mean_x = sum / size;

//dividing input into two arrays
  point array1[size/2];
  int index1 = 0;
  point array2[size/2];
  int index2 = 0;

  for(int i = 0; i < size; i++){
    if (array[i].x <= ar_mean_x) {
      array1[index1] = array[i];
      index1++;
    }else{
      array2[index2] = array[i];
      index2++;
    }
  }


  if (pipe(fd1) < 0 || pipe(fd2) < 0 || pipe(fd3) < 0 || pipe(fd4) < 0) {
      fprintf(stderr, "[%s] error: Error while pipe making\n", prog_name);
      exit(EXIT_FAILURE);
  }

  point leftPair[2];
  point rightPair[2];
//fork left
forkLeft(array1,index1,leftPair);

//fork right
forkRight(array2,index2,rightPair);

//checking whether the right child has less than 2 points
if (right_flag == 1) {
  print_pair(leftPair);
}
//checking whether the left child has less than 2 points
else if(left_flag == 1){
  print_pair(rightPair);
}
//comparing the results of the children
else{
  compare(leftPair, rightPair);
}
  free(line);
  free(array);


exit(EXIT_SUCCESS);

}

/**
 * @brief Printing the solution to stdout.
 * @param pair - the solution of the program
 */
static void print_pair(point* pair){

  for(int i = 0; i < 2; i++){
    fprintf(stdout, "%f %f\n", pair[i].x, pair[i].y);
  }
}



/**
 * @brief Fork process and recursive execution of programs for the left part.
 * @param array1 - the place where the left part of the input is stored
 * @param index1 -  length of the array1
 * @param array - for storing the resulting pair
 */
static void forkLeft(point* array1, int index1, point* array)
{
  pid_t pid = fork();

//in case of child process duplicate stdin and stdout to pipes and call execlp
  if (pid == 0) {
    if (dup2(fd1[0], fileno(stdin)) < 0) {
      fprintf(stderr, "[%s] error: Error duplicating pipe\n", prog_name);
      exit(EXIT_FAILURE);
    }
    if (dup2(fd2[1], fileno(stdout)) < 0) {
      fprintf(stderr, "[%s] error: Error duplicating pipe\n", prog_name);
      exit(EXIT_FAILURE);
    }

    close(fd1[0]);
    close(fd1[1]);
    close(fd2[0]);
    close(fd2[1]);

    if (execlp("./cpair", "", NULL) < 0) {
      fprintf(stderr, "[%s] error: execlp error\n", prog_name);
      exit(EXIT_FAILURE);
    }
  }

/**
* in case of parent process write the array1 to stdin of child,
* wait for child and read from child stdout
*/
  else if(pid > 0){

    close(fd1[0]);
    close(fd2[1]);


    FILE* write = fdopen(fd1[1],"w");
    for(int i = 0; i < index1; i++){
      if(fprintf(write, "%f %f\n", array1[i].x, array1[i].y) < 0 ){
        fprintf(stderr, "[%s] error: Error writing to child\n", prog_name);
        exit(EXIT_FAILURE);
      }
    }
    if(fclose(write) == EOF){
      fprintf(stderr, "[%s] error: Error while closing file\n", prog_name);
    }
    close(fd1[1]);

    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) != pid) {
        if (wpid != -1) {
            continue;
        }
        if (errno == EINTR) {
            continue;
        }

        fprintf(stderr, "[%s] error: Cannot wait\n", prog_name);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    int char_num;

    FILE* read = fdopen(fd2[0], "r");
    if (read == NULL) {
      fprintf(stderr, "[%s] error: File open error\n", prog_name);
      exit(EXIT_FAILURE);
    }



    for(int i = 0; i < 2; i++){
      char_num = getline(&line, &len, read);
      if (char_num < 0) {
        left_flag = 1;
        break;
      }
      sscanf(line, "%f %f", &array[i].x, &array[i].y);
    }


    if(fclose(read) == EOF){
      fprintf(stderr, "[%s] error: Error while closing file\n", prog_name);
    }
    close(fd2[0]);


  }
  else
  {
    fprintf(stderr, "[%s] error: Fork error\n", prog_name);
    exit(EXIT_FAILURE);
  }
}



/**
 * @brief Fork process and recursive execution of programs for the right part.
 * @param array2 - the place where the right part of the input is stored
 * @param index2 -  length of the array2
 * @param array - for storing the resulting pair
 */
static void forkRight(point* array2, int index2, point* array)
{
  pid_t pid = fork();

//in case of child process duplicate stdin and stdout to pipes and call execlp
  if (pid == 0) {
    if (dup2(fd3[0], fileno(stdin)) < 0) {
      fprintf(stderr, "[%s] error: Error duplicating pipe\n", prog_name);
      exit(EXIT_FAILURE);
    }
    if (dup2(fd4[1], fileno(stdout)) < 0) {
      fprintf(stderr, "[%s] error: Error duplicating pipe\n", prog_name);
      exit(EXIT_FAILURE);
    }

    close(fd3[0]);
    close(fd3[1]);
    close(fd4[0]);
    close(fd4[1]);

    if (execlp("./cpair", "", NULL) < 0) {
      fprintf(stderr, "[%s] error: execlp error\n", prog_name);
      exit(EXIT_FAILURE);
    }
  }

  /**
  * in case of parent process write the array2 to stdin of child,
  * wait for child and read from child stdout
  */
  else if(pid > 0){

    close(fd3[0]);
    close(fd4[1]);

    FILE* write = fdopen(fd3[1],"w");
    for(int i = 0; i < index2; i++){
      if(fprintf(write, "%f %f\n", array2[i].x, array2[i].y) < 0 ){
        fprintf(stderr, "[%s] error: Error writing to child\n", prog_name);
        exit(EXIT_FAILURE);
      }
    }
    if(fclose(write) == EOF){
      fprintf(stderr, "[%s] error: Error while closing file\n", prog_name);
    }
    close(fd3[1]);

    int status;
    pid_t wpid;
    while ((wpid = wait(&status)) != pid) {
        if (wpid != -1) {
            continue;
        }
        if (errno == EINTR) {
            continue;
        }

        fprintf(stderr, "[%s] error: Cannot wait\n", prog_name);
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t len = 0;
    int char_num;

    FILE* read = fdopen(fd4[0], "r");
    if (read == NULL) {
      fprintf(stderr, "[%s] error: File open error\n", prog_name);
      exit(EXIT_FAILURE);
    }


    for(int i = 0; i < 2; i++){
      char_num = getline(&line, &len, read);
      if (char_num < 0) {
        right_flag = 1;
        break;
      }
        sscanf(line, "%f %f", &array[i].x, &array[i].y);

    }

    if(fclose(read) == EOF){
      fprintf(stderr, "[%s] error: Error while closing file\n", prog_name);
    }
    close(fd4[0]);


  }
  else
  {
    fprintf(stderr, "[%s] error: Fork error\n", prog_name);
    exit(EXIT_FAILURE);
  }
}


/**
 * @brief Merging left and right parts, comparing pairs, printing result to stdout
 * @param leftPair the place where the left pair of the left child is stored
 * @param rightPair the place where the right pair of the right child is stored
 */

static void compare(point* leftPair, point* rightPair){
  float minDistance,tempDistance;
  float sub_x,sub_y;
  point minPair[2];

//calculating min distance between different points of the two pairs
  sub_x = leftPair[0].x-rightPair[0].x;
  sub_y = leftPair[0].y-rightPair[0].y;
  minDistance = sqrt(sub_x*sub_x+sub_y*sub_y);
  minPair[0]=leftPair[0];
  minPair[1]=rightPair[0];
  for(int i=0;i<2;i++)
  {
    for(int j=0;j<2;j++)
    {
      sub_x = leftPair[i].x-rightPair[j].x;
      sub_y = leftPair[i].y-rightPair[j].y;
      tempDistance = sqrt(sub_x*sub_x+sub_y*sub_y);
      if(tempDistance<minDistance)
      {
        minDistance=tempDistance;
        minPair[0]=leftPair[i];
        minPair[1]=rightPair[j];
      }
    }
  }

//calculating the distance of the left pair and comparing it to the min distance
  sub_x = leftPair[0].x-leftPair[1].x;
  sub_y = leftPair[0].y-leftPair[1].y;
  tempDistance=sqrt(sub_x*sub_x+sub_y*sub_y);
  if(tempDistance<minDistance)
  {
    minDistance=tempDistance;
    minPair[0]=leftPair[0];
    minPair[1]=leftPair[1];
  }

//calculating the distance of the right pair and comparing it to the min distance
  sub_x = rightPair[0].x-rightPair[1].x;
  sub_y = rightPair[0].y-rightPair[1].y;
  tempDistance=sqrt(sub_x*sub_x+sub_y*sub_y);
  if(tempDistance<minDistance)
  {
    minDistance=tempDistance;
    minPair[0]=rightPair[0];
    minPair[1]=rightPair[1];
  }

//printing the pair with min distance
print_pair(minPair);

}
