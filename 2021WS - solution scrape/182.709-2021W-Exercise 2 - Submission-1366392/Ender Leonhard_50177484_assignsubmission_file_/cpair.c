/**
 * @file cpair.c
 * @author Leonhard Ender (12027408)
 * @date 10.12.2021
 *
 * @brief A simple algorithm for finding the closest pair of points.
 * 
 * The program reads a list of points from stdin. Each line has to contain
 * excactly one point, represented by two floating point numbers (the x and
 * the y coordinate) seperated by one or more whitespaces. THe program takes
 * no arguments.
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include "cpair.h"
#include <math.h>

/* A global variable holding the points passed to the program. */
point_t * points=NULL;

/* A global variable holding the first group of points */
point_t * p1=NULL;

/* A global variable holding the second group of points */
point_t * p2=NULL;

/* A helper variable for reading input */
char * lineptr=NULL;

/**
 * @brief  Free up the global variables holding lists of points, if
 * space has been allocated for them.
 */
static void free_resources(void) {
  if (p1!=NULL) {
    free(p1);
    p1 = NULL;
  }
  if (p2!=NULL) {
    free(p2);
    p2 = NULL; 
  }
  if (points != NULL) {
    free(points);
    points=NULL;
  }
  if (lineptr != NULL) {
    free(lineptr);
    lineptr = NULL;
  }
}

/**
 * @brief  Calculate the distance between two points.
 *
 * @param[in]  p1    the first point
 * @param[in]  p2    the second point
 *
 * @return  The (euclidean) distance between the two points.
 */
static double pointdst(point_t p1, point_t p2) {
  return sqrt( pow((p1.x-p2.x),2) + pow((p1.y-p2.y),2));
}


/**
 * @brief      Copies the x- and y-coordinates of the source point
 * into the destination point.
 * 
 * @param      dest  The destination point.
 * @param      src   The source point.
 */
static void pointcpy(point_t * dest, point_t * src) {
  dest->x = src->x;
  dest->y = src->y;
}

/**
 * @brief      Print the specified error message, free up resources and exit.
 *
 * @param      message  THe error message to be printed.
 */
static void error_exit(char * message) {
  perror(message);
  free_resources();
  exit(EXIT_FAILURE);
}

/**
 * @brief      Process a line that was read from stdin. 
 * 
 * The function accepts any number of whitespaces followed by
 * excactly one floating point number followed one or more
 * whitespaces followed by excactly one floating point number
 * followed by any number of whitespaces. The floating point
 * numbers are written into the specified point, the first
 * number as the x- and the second as the y-coordinate.
 *
 * @param      line   A string containing one line of input.
 * @param      point  The point where the values are stored.
 */
static void process_line(char * line, point_t * point) {

  if (sscanf(line," %lf %lf ", &(point->x), &(point->y)) != 2)
    error_exit("wrong point format");
}

/**
 * @brief      Calculates the average of the x-coordinates
 * of the given list of points.
 *
 * @param      points     A pointer to an array of points.
 * @param[in]  point_cnt  The number of points in the array.
 *
 * @return     The average of the x-coordinates.
 */
static double calc_avg(point_t * points, int point_cnt) {
  
  double sum = 0;

  for (int i=0; i<point_cnt; i++) {
    sum += points[i].x;
  }
  return ((double) sum)/point_cnt;  
}

/**
 * @brief      Splits the given points into two groups: One with lower than average
 * x-coordinates, one with high than average x-coordinates. The number of points
 * in the first group is returned.
 * 
 * The parameters p1 and p2 are pointers to pointers. After the (succesful) execution
 * of the function, they point to pointers which in turn point to the respective 
 * arrays of pointers. These arrays are allocated during execution of the function
 * and have to be freed by the user afterwards.
 *
 * @param      points     A pointer to an array of points.
 * @param[in]  point_cnt  The number of points in the array.
 * @param      p1         A pointer to the first group.
 * @param      p2         A pointer to the second group.
 *
 * @return     The number of points in the first group.
 */
static int split_points(point_t * points, int point_cnt, point_t ** p1, point_t ** p2) {
  
  /* calculate average */
  double avg = calc_avg(points, point_cnt);

  /* allocate resources */
  int p1_cnt = 0, p2_cnt = 0;
  point_t * p1tmp = malloc(sizeof(point_t));
  point_t * p2tmp = malloc(sizeof(point_t));
  if (p1tmp == NULL || p2tmp == NULL)
    error_exit("malloc failed");
  
  /* split the points into two groups */
  for (int i = 0; i<point_cnt; i++) {
    if (points[i].x <= avg) {
      p1_cnt++;
      p1tmp = realloc(p1tmp, p1_cnt*sizeof(point_t));
      if (p1tmp == NULL)
	     error_exit("realloc failed");
      p1tmp[p1_cnt-1].x = points[i].x;
      p1tmp[p1_cnt-1].y = points[i].y;
    }
    else {
      p2_cnt++;
      p2tmp = realloc(p2tmp, p2_cnt*sizeof(point_t));
      if (p2tmp == NULL)
	     error_exit("realloc failed");
      p2tmp[p2_cnt-1].x = points[i].x;
      p2tmp[p2_cnt-1].y = points[i].y;
    }
  }

  *p1 = p1tmp;
  *p2 = p2tmp;
  
  return p1_cnt;
}
      
  
/**
 * @brief      The main function. Parses the input and finds the closest pair. 
 * Takes no arguments.
 *
 * @param[in]  argc  The count of arguments
 * @param      argv  The arguments array
 *
 * @return     Returns 0 on success, an error code on failure.
 */
int main(int argc, char **argv) {
  char * prog_name = argv[0];
 
  /* no arguments allowed */
  if (argc>1) {
    fprintf(stderr,"Usage: %s\n",prog_name);
    exit(EXIT_FAILURE);
  } 
  
  /* resources for reading stdin */
  size_t size = 0;

  int point_cnt=0;
  int buffer_size = 10;
  
  points = malloc(buffer_size*sizeof(point_t));
  if (points == NULL)
    error_exit("malloc failed");
  
  /* read stdin line by line and process each one */
  while(getline(&lineptr, &size, stdin) != -1) {
    process_line(lineptr, &points[point_cnt]);
    point_cnt++;
    if (point_cnt >= buffer_size-1) {
      buffer_size += 10;
      points = realloc(points, buffer_size*sizeof(point_t));
      if (points==NULL)
	      error_exit("realloc failed");
    }
  }

  free(lineptr);
  lineptr = NULL;
  
  /* only two points read */
  if (point_cnt == 2){
    fprintf(stdout,"%lf %lf\n", points[0].x, points[0].y);
    fprintf(stdout,"%lf %lf\n", points[1].x, points[1].y);
    free_resources();
    exit(EXIT_SUCCESS);
  }
  /* only one point read */
  else if (point_cnt <= 1) {
    free_resources();
    exit(EXIT_SUCCESS);
  }


  /* split points into two groups */
  int p1_cnt = split_points(points, point_cnt, &p1, &p2);
  int p2_cnt = point_cnt - p1_cnt;

  /* filedescriptors for the pipes */
  int pipeToA[2];	
  int pipeToB[2];	
  int pipeFromA[2];	
  int pipeFromB[2];	

  int child_pid_A=-1;
  int child_pid_B=-1;


  /* create pipes for child A */
  if (pipe(pipeToA)==-1) {
    error_exit("pipe failed");
  }

  if (pipe(pipeFromA)==-1) {
    error_exit("pipe failed");
  }


  /* create child A */
  if ( (child_pid_A = fork()) == -1) {
    error_exit("fork failed");
  }

  if (child_pid_A==0) { /* this is child a */

    close(pipeToA[1]); /* close unused write end */
    close(pipeFromA[0]); /* close unused read end */

    /* set pipes as stdout and stdin */
    dup2(pipeToA[0],STDIN_FILENO);
    dup2(pipeFromA[1],STDOUT_FILENO);

    /* recursion */
    execlp(prog_name, prog_name, NULL);

    /* exec has failed */
    error_exit("exec failed");
  }
  
  /* this is the parent */

  close(pipeToA[0]); /* close unused read end */
  close(pipeFromA[1]); /* close unused write end */

  /* create pipes for child B */
  if (pipe(pipeToB)==-1) {
    error_exit("pipe failed");
  }

  if (pipe(pipeFromB)==-1) {
    error_exit("pipe failed");
  }

  /* create  child B */
  if ( (child_pid_B = fork()) == -1) {
    error_exit("fork failed");
  }
  if (child_pid_B==0) { /* this is child B */ 

    close(pipeToB[1]); /* close unused write end */
    close(pipeFromB[0]); /* close unused read end */
  
    /* set pipes as stdout and stdin */
    dup2(pipeToB[0],STDIN_FILENO);
    dup2(pipeFromB[1],STDOUT_FILENO);

    /* recursion */
    execlp(prog_name, prog_name, NULL);

    /* exec has failed */
    error_exit("exec failed");
  }

  /* still the parent */

  close(pipeToB[0]); /* close unused read end */
  close(pipeFromB[1]); /* close unused write end */


  /* write to stdin of children */ 

  FILE * outA = fdopen(pipeToA[1],"w");
  if (outA == NULL)
    error_exit("fdopen failed");
  FILE * outB = fdopen(pipeToB[1],"w");
  if (outB == NULL)
    error_exit("fdopen failed");

  for (int i = 0; i< p1_cnt;i++) {
    fprintf(outA, "%f %f\n", p1[i].x, p1[i].y);
  } 

  for (int i = 0; i< p2_cnt;i++) {
    fprintf(outB, "%f %f\n", p2[i].x, p2[i].y);
  } 

  fclose(outA);
  fclose(outB);



/*  fprintf(stderr, 
    "Launched children A (%d) and B (%d) with %d  and %d points respectively.\n", 
    child_pid_A, 
    child_pid_B, 
    p1_cnt, 
    p2_cnt); */

  /* wait for children to terminate */
  int st = 0;
//  int child_pid = 0;
//  child_pid = wait(&st);
  wait(&st);
//  fprintf(stderr,"child with pid %d terminated with status %d\n", child_pid, st);
  if (st != 0)
    error_exit("child failed");
//  child_pid = wait(&st);
  wait(&st);
//  fprintf(stderr,"child with pid %d terminated with status %d\n", child_pid, st);
  if (st != 0)
    error_exit("child failed");


  /* read output from children */     

  FILE * inA = fdopen(pipeFromA[0],"r");
  if (inA == NULL)
    error_exit("fdopen failed");
  FILE * inB = fdopen(pipeFromB[0],"r");
  if (inB == NULL)
    error_exit("fdopen failed");


  lineptr = NULL;
  size = 0;

  point_t pointA[2] = {{0}}, pointB[2]={{0}};

  if (p1_cnt >= 2 ) { /* only read if the group has two or more points */
    for (int i = 0; i<2; i++) {
      if (getline(&lineptr, &size, inA) != -1) {
        process_line(lineptr, &pointA[i]);
      }
      else {
        fprintf(stderr, "no result from child A (%d)\n",child_pid_A);
        exit(EXIT_FAILURE);
      }
    }
  }
 
  if (p2_cnt >= 2) { /* only read if the group has two or more points */
    for (int i = 0; i<2; i++) {
      if (getline(&lineptr, &size, inB) != -1) {
        process_line(lineptr, &pointB[i]);
      }
      else {
        fprintf(stderr, "no result from child B (%d)\n",child_pid_B);
        exit(EXIT_FAILURE);
      }
    }
  } 

  if (lineptr!=NULL) {
    free(lineptr);
    lineptr = NULL;
  }

  fclose(inA);
  fclose(inB);


  /* calculate best solution */
 
  point_t pointC[2] = {{0}};


  if (p1_cnt <2 && p2_cnt<2) { /* shouldn't be possible */
    fprintf(stderr, "illegal division of points\n");
    exit(EXIT_FAILURE);
  }
  else if (p1_cnt < 2) { /* group one contains only one point */
    pointcpy(&pointC[0], &pointB[0]);
    pointcpy(&pointC[1], &pointB[1]);
  }
  else if (p2_cnt <2) { /* group two contains only one point */
    pointcpy(&pointC[0], &pointA[0]);
    pointcpy(&pointC[1], &pointA[1]);
  }
  else {
    pointcpy(&pointC[0], &pointA[0]);
    pointcpy(&pointC[1], &pointB[0]);  

    /* check combinations */
    for (int i=0; i<2; i++) {
      for (int j=0; j<2; j++) {
        if (pointdst(pointA[i],pointB[j])<pointdst(pointC[0],pointC[1])) {
          pointcpy(&pointC[0], &pointA[i]);
          pointcpy(&pointC[1], &pointB[j]);
        }
      }
    }

    /* compare points */
    if (pointdst(pointC[0],pointC[1]) > pointdst(pointA[0],pointA[1])) {
      pointcpy(&pointC[0],&pointA[0]);
      pointcpy(&pointC[1],&pointA[1]);
    }
    
    if (pointdst(pointC[0],pointC[1]) > pointdst(pointB[0],pointB[1])) {
      pointcpy(&pointC[0],&pointB[0]);
      pointcpy(&pointC[1],&pointB[1]);
    }
  }

  /* output to stdout */
  for (int i = 0; i<2; i++) {
    fprintf(stdout,"%lf %lf\n",pointC[i].x, pointC[i].y);
  }

  /* close up shop */
  free_resources();
  exit(EXIT_SUCCESS);
}




