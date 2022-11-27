/**
 * @file cpair.c
 * @author Maximilian Scharizer 01604779
 * @date 11.12.2021
 * @brief Calculating the closest pair from a set of given Points
 **/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <regex.h>
#include <stdlib.h>

#define BUFFER_SIZE 1023 //used for reading and writing through the pipes

char *prog_name;

typedef struct point
{
  long double x;
  long double y;
}Point;

typedef struct pair
{
  Point point1;
  Point point2;
  long double distance;
  int size;
} Pair;
/**
 * @brief A function to determine if the input is valid.
 * @details The input is validated with regex.
 * @param input String to be verified.
 * @return Return value is 0 if input is valid.
 */
int validateInput(char *input)
{
  regex_t regex;
  if (regcomp(&regex, "^-?[0-9]+\\.?[0-9]*\\ -?[0-9]+\\.?[0-9]*", REG_EXTENDED) != 0)
    return -1;
  int matches = regexec(&regex, input, 0, 0, 0);
  regfree(&regex);
  return matches;
}

/**
 * @brief A function that exits the program.
 * @details Exits with EXIT_FAILURE and prints a description of the problem to stderr.
 * @param message A message describing the error.
 */
void exit_fail(char *message)
{
  fprintf(stderr, "[%s] Error: %s\n%s\n", prog_name, strerror(errno), message);
  exit(EXIT_FAILURE);
}

/**
 * @brief A function that calculates the distance between two points.
 * @details Calulation of the euclidian distance.
 * @param point1 The first point. 
 * @param point2 The second point.
 * @return Return value is the euclidian distance between the two points.
 */
long double distance(Point *point1, Point *point2)
{
  long c = ((point1->x)-(point2->x))*((point1->x)-(point2->x)) + ((point1->y)-(point2->y))*((point1->y)-(point2->y));
  return sqrt(c);
}

/**
 * @brief A function that reads from a child.
 * @details Reading from child through a File descriptor and write the result to a Pair struct.
 * @param fd The File descriptor.
 * @param pair The Pair struct for writting.
 */
void readFromChild(int fd, Pair *pair)
{
  char *buffer = NULL;
  size_t len = 0;
  FILE *readFromChildFile;
  if ((readFromChildFile = fdopen(fd, "r")) == NULL)
    exit_fail("File descriptor could not be opened");

  while (getline(&buffer, &len, readFromChildFile) != -1)
  {
    if (buffer[0] != '\0')
    {
      if (validateInput(buffer)==0)
      {
        char *end_ptroint2;
        if (pair->size == 0)
        {
          long double x = strtold(buffer, &end_ptroint2);
          long double y = strtold(end_ptroint2 + 1, NULL);
          pair->point1.x = x;
          pair->point1.y = y;
        }
        else
        {
          long double x = strtold(buffer, &end_ptroint2);
          long double y = strtold(end_ptroint2 + 1, NULL);
          pair->point2.x = x;
          pair->point2.y = y;
          pair->distance = distance(&pair->point1, &pair->point2);
        }
        pair->size++;
      }
      else
      {
        exit_fail("Input could not be parsed");
      }
    }
  }

  if (fclose(readFromChildFile) != 0)
    exit_fail("File descriptor could not be closed");
}

/**
 * @brief A function that calculates the mean of Points.
 * @details The function calculates the arithmetic mean of all x-values of the given Points.
 * @param point1 The first point.
 * @param point2 The second point.
 * @return The return value is the arithmetic mean of the x-values of the Points.
 */
long double arithmetic_mean(Point *point, int length)
{
  long sum = 0;
  for (int i = 0; i < length; i++)
  {
    sum = sum + point[i].x;
  }
  return sum / length;
}

/**
 * @brief A function that finds a potentially closest pair.
 * @details The function finds the closes pair among both point array halves.
 * @param point The array containing all points.
 * @param pair3 The closest pair found.
 * @param size The number of points in the point array.
 * @param mean The mean to determine if a point belongs to child1 or child2.
 */
void findClosestPairCrossing(Point *point, Pair *pair3, int size, long double mean)
{
  for (int i = 0; i < size; i++)
  {
    for (int j = 0; j < size; j++)
    {
      if (point[i].x <= mean && point[j].x > mean)
      { //if both points were assigned to different children
        long double d = distance(&point[i], &point[j]);
        if (pair3->size == 0)
        { //if no closest pair3 has been found before
          pair3->size = 2;
          pair3->distance = d;
          pair3->point1.x = point[i].x;
          pair3->point1.y = point[i].y;
          pair3->point2.x = point[j].x;
          pair3->point2.y = point[j].y;
        }
        else
        {
          if (d < pair3->distance)
          { // if a closer pair has been found
            pair3->distance = d;
            pair3->point1.x = point[i].x;
            pair3->point1.y = point[i].y;
            pair3->point2.x = point[j].x;
            pair3->point2.y = point[j].y;
          }
        }
      }
    }
  }
}

/**
 * @brief A function that prints a Pair.
 * @details The function writes to stdout.
 * @param pair The Pair that is printed to stdout.
 */
void print(Pair *pair)
{
  fprintf(stdout,"%Lf %Lf\n", pair->point1.x, pair->point1.y);
  fprintf(stdout,"%Lf %Lf\n", pair->point2.x, pair->point2.y);
}


/**
 * @brief The main function of this program.
 * @details This function serves as an entry and exit point. It reads from stdin
 * and creates 2 child processes, to whom the Points (from stdin) are distributed
 * according to the arithmetic mean of the x-values of these Points.
 * Then the closest pair from both children is read 
 * procesess and splites the points evenly to both children. This happens
 * with the help of recursion. If there are only 2 Points remaining in a child the
 * result is printed to stdout. The parent process will read the closest pair from
 * both children and also determine if there is an even closer point between those
 * two children.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Return value is EXIT_SUCCESS upon success, else EXIT_FAILURE.
 */
int main(int argc, char *argv[])
{
  if(argc!=1){
    exit(EXIT_FAILURE);
  }

  prog_name = argv[0];

  Point *points  = malloc(sizeof(Point)) ;
  char *buffer = NULL;
  char *end_ptr;
  size_t len = 0;
  long double x;
  long double y;
  int numberOfPoints = 0;

  while (getline(&buffer, &len, stdin) != -1)
  {
    if (buffer[0] != '\0')
    {
      if (validateInput(buffer)==0)
      {
        if (numberOfPoints != 0)
        {
          points = realloc(points, sizeof(Point) * (numberOfPoints + 1));
          if (points == NULL)
            exit_fail("Reallocation of Array was unsuccessful");
        }
        x = strtold(buffer, &end_ptr);
        y = strtold(end_ptr + 1, NULL);
        points[numberOfPoints].x = x;
        points[numberOfPoints].y = y;
        numberOfPoints++;
      }
      else
      {
        exit_fail("Input could not be parsed");
      }
    }
  }


  if(numberOfPoints==0){
    exit(EXIT_SUCCESS);
  }
  if(numberOfPoints==1){
    exit(EXIT_SUCCESS);
  }
  if(numberOfPoints==2){
    char buf[BUFFER_SIZE];
    sprintf(buf, "%Lf %Lf\n", points[0].x, points[0].y);
    fputs(buf, stdout);
    sprintf(buf, "%Lf %Lf\n", points[1].x, points[1].y);
    fputs(buf, stdout);
    exit(EXIT_SUCCESS);
  }


  int writePipeChild1[2];
  int readPipeChild1[2]; 
  int writePipeChild2[2];
  int readPipeChild2[2];


  if (pipe(writePipeChild1))
    exit_fail("writePipeChild1 could not be created");
  if (pipe(readPipeChild1))
    exit_fail("readPipeChild1 could not be created");

  pid_t child1 = fork();
  switch (child1)
  {
  case -1:
    exit_fail("Fork failed");
    break;
  //child process  
  case 0: 
    //closing unused ends
    close(writePipeChild1[1]);
    dup2(writePipeChild1[0], STDIN_FILENO);
    close(writePipeChild1[0]);

    //closing unused ends
    close(readPipeChild1[0]);
    dup2(readPipeChild1[1], STDOUT_FILENO);
    close(readPipeChild1[1]);
    execlp(prog_name, prog_name, NULL);
    break;
  //parent process
  default:
    //closing unused ends
    close(writePipeChild1[0]);
    close(readPipeChild1[1]);
    break;
  }


  if (pipe(writePipeChild2))
    exit_fail("writePipeChild2 could not be created");
  if (pipe(readPipeChild2))
    exit_fail("readPipeChild2 could not be created");

  pid_t child2 = fork();
  switch (child2)
  {
  case -1:
    exit_fail("Forking unsuccessful");
    break;
  //child process
  case 0:
    //closing unused ends
    close(writePipeChild2[1]);
    dup2(writePipeChild2[0], STDIN_FILENO);
    close(writePipeChild2[0]);

    //closing unused ends
    close(readPipeChild2[0]);
    dup2(readPipeChild2[1], STDOUT_FILENO);
    close(readPipeChild2[1]);
    execlp(prog_name, prog_name, NULL);
    break;
  //parent process
  default:
    //closing unused ends
    close(writePipeChild2[0]);
    close(readPipeChild2[1]);
    break;
  }
  //Create streams to write to the chidren
  FILE *writeToChild1 = fdopen(writePipeChild1[1], "w");
  FILE *writeToChild2 = fdopen(writePipeChild2[1], "w");
  if (writeToChild1 == NULL || writeToChild2 == NULL)
    exit_fail("File descriptor could not be opened");

  //calculate arithmetic mean of all points
  long double mean = arithmetic_mean(points, numberOfPoints);
  char points_buffer[BUFFER_SIZE];

  for (int i = 0; i < numberOfPoints; i++)
  {
    sprintf(points_buffer, "%Lf %Lf\n", points[i].x, points[i].y);
    //if point x-value is smaller than or equal to mean write to child1
    if (points[i].x <= mean)
    {
      if (fputs(points_buffer, writeToChild1) == EOF){
        exit_fail("fputs was unsuccessful");
      }
    }
    //else write to child2
    else
    {
      if (fputs(points_buffer, writeToChild2) == EOF){
        exit_fail("fputs was unsuccessful");
      }
    }
    
  }
  //Close both streams when writing is finished
  if (fclose(writeToChild1) != 0)
    exit_fail("File descriptor could not be closed");
  if (fclose(writeToChild2) != 0)
    exit_fail("File descriptor could not be closed");

  //Waiting for both children
  int status;
  wait(&status);
  if (WEXITSTATUS(status) != EXIT_SUCCESS)
    exit_fail("WEXITSTATUS EXIT_FAILURE");
  wait(&status);
  if (WEXITSTATUS(status) != EXIT_SUCCESS)
    exit_fail("WEXITSTATUS EXIT_FAILURE");

  //Variables to store results, when reading from children
  Pair pair1;
  Pair pair2;
  Pair pair3;
  pair1.size = 0;
  pair2.size = 0;
  pair3.size = 0;
  pair1.distance = LDBL_MAX;
  pair2.distance = LDBL_MAX;
  pair3.distance = LDBL_MAX;

  //reading from child1
  readFromChild(readPipeChild1[0], &pair1);
  //reading from child2
  readFromChild(readPipeChild2[0], &pair2);

  //calcuate closest between children
  findClosestPairCrossing(points, &pair3, numberOfPoints, mean);
  if (pair1.size == 2 && pair2.size == 2)
  {
    if (pair1.distance < pair2.distance && pair1.distance < pair3.distance)
    {
      print(&pair1);
    }
    else if (pair2.distance < pair1.distance && pair2.distance < pair3.distance)
    {
      print(&pair2);
    }
    else
      print(&pair3);
  }
  else if (pair1.size == 2)
  {
    if (pair1.distance < pair3.distance)
    {
      print(&pair1);
    }
    else
      print(&pair3);
  }
  else if (pair2.size == 2)
  {
    if (pair2.distance < pair3.distance)
    {
      print(&pair2);
    }
    else
      print(&pair3);
  }
  else
  {
    free(points);
    exit_fail("Children did not respond");
  }

  free(points);

  exit(EXIT_SUCCESS);
}
