/**
*@file generator.h
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief generator program module
*
* This program receives edges of a graph as positional arguments, calculates random solutions which edges
* have to be deleted to make the graph 3-colorable and stores the solutions in a shared memory.
* It is dependent on the supervisor process running before one or multiple generators are started.
*
**/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

struct vertex
{
  int index;
  int color;
};
