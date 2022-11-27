/**
*@file generator.c
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief generator program module
*
* This program receives edges of a graph as positional arguments, calculates random solutions which edges
* have to be deleted to make the graph 3-colorable and stores the solutions in a shared memory.
* It is dependent on the supervisor process running before one or multiple generators are started.
**/

#include "supervisor.h"
#include "generator.h"
/** program name.
 * @brief this name will be used for error messages.
 **/
char *prog_name;

/**
*  usage function.
* @brief In case the arguments when calling the program were invalid, this function prints what they should be.
* @details global variables: prog_name
**/
static void
usage (void)
{
  fprintf (stderr,
	   "Usage %s call with edges of the graph as positional arguments like: 0-1 0-2 0-3 \n",
	   prog_name);
  exit (EXIT_FAILURE);
}

/**
 * Starting point of the program.
 * @brief The main function reads the graph as arguments, creates random 3-colorings and stores solutions of edges that need to be deleted in the shared memory.
 * @details The generator needs a running supervisor, otherwise the semaphores and shared memory are not available and the program will exit.
 * The edges of the graph that have been given as input parameters are stored first, then semaphores and shared memory are opened,
 * on any error the programm will free all resources that have been opened so far. In a loop, the vertices are assigned a random 3-coloring,
 * and edges that need to be deleted for the 3-coloring to be valid are found. The solution is stored in the shared memory.
 * The loop only stops when the terminate flag in the shared memory has been set by the supervisor.
 * global variables: prog_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 **/
int
main (int argc, char *argv[])
{

  prog_name = argv[0];
  char *vertex1_string = NULL;
  char *vertex2_string = NULL;
  int vertex1;
  int vertex2;
  int max_vertex = 0;
  int best_solution_number = SOLUTION_SIZE + 1;
  int generator_index = 0;
  struct edge alledges[argc - 1];
  struct solution current_solution;
  regex_t regex_expression;

  if (argc == 1)
    {
      usage ();
    }

  if (regcomp (&regex_expression, "^[0-9]*-[0-9]*$", 0) != 0)
    {				//[:digit:]+[-][:digit:]+
      fprintf (stderr, "%s: regex compile failed %s \n", prog_name,
	       strerror (errno));
      exit (EXIT_FAILURE);
    }

  for (int i = 1; i < argc; i++)
    {
      int regex_result = regexec (&regex_expression, argv[i], 0, 0, 0);
      if ((regex_result == 0) || (regex_result == REG_NOMATCH))
	{			//regexec returns 0 on successful match
	  if (regex_result == REG_NOMATCH)
	    {
	      usage ();
	    }
	}
      else
	{
	  fprintf (stderr, "%s: regex compare failed %s \n", prog_name,
		   strerror (errno));
	  exit (EXIT_FAILURE);
	}
      vertex1_string = strtok (argv[i], "-");
      vertex2_string = strtok (NULL, "-");
      vertex1 = strtol (vertex1_string, NULL, 10);	//base 10 since the indices are decimal values
      vertex2 = strtol (vertex2_string, NULL, 10);	//base 10 since the indices are decimal values
      alledges[i - 1].vertex1 = vertex1;
      alledges[i - 1].vertex2 = vertex2;
      if (vertex1 > max_vertex)
	{
	  max_vertex = vertex1;
	}
      if (vertex2 > max_vertex)
	{
	  max_vertex = vertex2;
	}
    }
  regfree (&regex_expression);

  //create array of vertices
  struct vertex vertices[max_vertex + 1];

  //open semaphores to coordinate access to circular buffer
  sem_t *sem_buffer = sem_open (SEM_BUFFER_ACCESS, 0);
  if (sem_buffer == SEM_FAILED)
    {
      fprintf (stderr, "%s: open of semaphore failed %s \n", prog_name,
	       strerror (errno));
      exit (EXIT_FAILURE);
    }
  sem_t *sem_free = sem_open (SEM_BUFFER_FREE, 0);
  if (sem_free == SEM_FAILED)
    {
      fprintf (stderr, "%s: open of semaphore failed %s \n", prog_name,
	       strerror (errno));
      if (sem_close (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }
  sem_t *sem_used = sem_open (SEM_BUFFER_USED, 0);
  if (sem_used == SEM_FAILED)
    {
      fprintf (stderr, "%s: open of semaphore failed %s \n", prog_name,
	       strerror (errno));
      if (sem_close (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_free) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //wait for server to open shared memory object before shm open
  sem_wait (sem_free);

  //shm_open
  int shared_open = shm_open (SHARED_MEMORY, O_RDWR, 0600);	//readwrite access, readwrite permission for current user
  if (shared_open == -1)
    {
      fprintf (stderr, "%s: open shared memory failed %s \n", prog_name,
	       strerror (errno));
      if (sem_close (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_free) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_used) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //mmap
  struct shared_struct *shm_mapped;
  shm_mapped =
    mmap (NULL, sizeof (struct shared_struct), PROT_READ | PROT_WRITE,
	  MAP_SHARED, shared_open, 0);
  if (shm_mapped == MAP_FAILED)
    {
      fprintf (stderr, "%s: mapping of shared memory failed %s \n", prog_name,
	       strerror (errno));

      //close(shm)
      if (close (shared_open) == -1)
	{
	  fprintf (stderr, "%s: closing of file descriptor failed %s \n",
		   prog_name, strerror (errno));
	}

      //sem_close
      if (sem_close (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_free) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_used) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }
  if (sem_post (sem_free) == -1)
    {
      fprintf (stderr, "%s: wait for semaphore failed %s \n", prog_name,
	       strerror (errno));
      //munmap
      if (munmap (shm_mapped, sizeof (struct shared_struct)) == -1)
	{
	  fprintf (stderr, "%s: unmap of shared memory failed %s \n",
		   prog_name, strerror (errno));
	}

      //close(shm)
      if (close (shared_open) == -1)
	{
	  fprintf (stderr, "%s: closing of file descriptor failed %s \n",
		   prog_name, strerror (errno));
	}

      //sem_close
      if (sem_close (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_free) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_close (sem_used) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //generate solutions
  while (1)
    {

      //color vertices randomly
      for (int i = 0; i <= max_vertex; i++)
	{
	  vertices[i].index = i;
	  vertices[i].color = random () % 3;	//random number, but we need only 3 colors
	}

      //generate solution
      int solutioncounter = 0;
      memset (&current_solution, 0, sizeof (current_solution));	//set all entries in struct to 0
      for (int i = 0; i < argc - 1; i++)
	{
	  if (vertices[alledges[i].vertex1].color ==
	      vertices[alledges[i].vertex2].color)
	    {
	      current_solution.edges[solutioncounter].vertex1 =
		alledges[i].vertex1;
	      current_solution.edges[solutioncounter].vertex2 =
		alledges[i].vertex2;
	      current_solution.number += 1;
	      if (current_solution.number >= best_solution_number);
	      {			//this solution is at most as good as the current best, stop
		break;
	      }
	      solutioncounter += 1;
	    }
	}

      //wait for access to buffer
      if (sem_wait (sem_free) == -1)
	{
	  fprintf (stderr, "%s: wait for semaphore failed %s \n", prog_name,
		   strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}
      if (sem_wait (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: wait for semaphore failed %s \n", prog_name,
		   strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}
      //check if termination is requested
      if (shm_mapped->terminate == TRUE)
	{
	  if (sem_post (sem_buffer) == -1)
	    {
	      fprintf (stderr, "%s: increase of semaphore failed %s \n",
		       prog_name, strerror (errno));
	      break;		//exit while loop to free semaphores and memory
	    }
	  break;
	}
      if (generator_index == 0)
	{
	  generator_index = ++(shm_mapped->generator_count);
	  srandom (generator_index);
	}

      //post solution
      shm_mapped->solutions[shm_mapped->writeindex] = current_solution;

      shm_mapped->writeindex += 1;
      shm_mapped->writeindex %= CIRCBUF_SIZE;

      if (sem_post (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: increase of semaphore failed %s \n",
		   prog_name, strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}
      //set another write slot
      if (sem_post (sem_used) == -1)
	{
	  fprintf (stderr, "%s: increase of semaphore failed %s \n",
		   prog_name, strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}
    }

  //increase free so next generator can terminate
  if (sem_post (sem_free) == -1)
    {
      fprintf (stderr, "%s: increase of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }

  //munmap
  if (munmap (shm_mapped, sizeof (struct shared_struct)) == -1)
    {
      fprintf (stderr, "%s: unmap of shared memory failed %s \n", prog_name,
	       strerror (errno));
    }

  //close(shm)
  if (close (shared_open) == -1)
    {
      fprintf (stderr, "%s: closing of file descriptor failed %s \n",
	       prog_name, strerror (errno));
    }

  //sem_close
  if (sem_close (sem_buffer) == -1)
    {
      fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }
  if (sem_close (sem_free) == -1)
    {
      fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }
  if (sem_close (sem_used) == -1)
    {
      fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }

  exit (EXIT_SUCCESS);
}
