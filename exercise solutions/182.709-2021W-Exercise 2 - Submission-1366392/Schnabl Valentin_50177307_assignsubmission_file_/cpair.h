/**
* @file cpair.h
* @author Valentin Schnabl, 11848108
* @date 02.12.2021
* @brief the header to cpair.c. It includes all function heads, the program name and teh definition of the structs.
**/
#include <stdio.h>
#include <stdlib.h> 
#include <regex.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <sys/types.h>
#include <sys/wait.h>
#define PROGRAM_NAME "./cpair"

static char *programName;

typedef struct Point{
    float a;
    float b;
} Point;

typedef struct Pipes{
    int write;
    int read;
} Pipes;

typedef struct ResultSet{
    Point a;
    Point b;
    float dist;
} ResultSet;

/**
* @brief Prints the neaerst result set to stdout. The nearest pair, is the pair which distance is the smallest.
* @param a,b,c the three result sets.
**/
static void printSmallestDist(ResultSet a, ResultSet b, ResultSet c);
/**
* @brief Parses the input from a file and validates it with regex. It must be a valid floating point number. It will be passed into a point array which is dynamically allocated.
* @param file the file pointer from which is beeing read
* @param points the point array which is being filled
* @return the size of the point array, 0 if validation or somthing else failed
**/
static size_t parseInput(FILE* file, Point **points);
/**
* @brief Writes a point array to a file, seperated by blanks.
* @param file the file pointer where it will be written to
* @param points the point array
* @param size the size of the point array
* @return 1 if the writing process was successful, -1 else
**/
static int writeToStdIn(FILE* file, Point* points, int size);
/**
* @brief calculates the mean out of an array of points.
* @param points the array of points
* @param size the size of the array
* @return the mean.
**/
static float mean(Point* points, size_t size);
/**
* @brief calculates the distance of two points
* @param x,y the two points
* @return the distance.
**/
static float dist(Point x, Point y);
/**
* @brief Creates a new Child and the pipes for reading and writing into and from the child.
* @param pipes The pipes array where the pipes are stored
* @return a pid_t of the child just created
**/
static pid_t spawnChild(Pipes** pipes);
/**
* @brief Calculate the nearest pair out of two point arrays. 
* @param points the point array from child1
* @param size the size from points
* @param points1 the point array from child2
* @param size1 the size from points1
* @return p3, a ResultSet, containing the nearest pair and the distance
**/
static ResultSet calcClosestPairMulti(Point* points, Point* points1, int size, int size1);
