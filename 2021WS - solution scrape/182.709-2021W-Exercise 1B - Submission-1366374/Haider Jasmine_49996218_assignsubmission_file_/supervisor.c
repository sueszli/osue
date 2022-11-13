/**
*@file supervisor.c
*@author Jasmine Haider <e11943664@student.tuwien.ac.at>
*@date 14.11.2021
*
*@brief supervisor program module
*
* This program creates shared memory and semaphores for one or multiple generators to store solutions for the
* 3-color problem in a circular buffer. It reads solutions from the buffer, storing the best solution that has
* been found so far and prints to stdout if a new solution has fewer edges than the last stored solution.
*
**/

#include "supervisor.h"
/** program name.
 * @brief this name will be used for error messages.
 **/
char *prog_name;
/** terminate.
 * @brief this variable will be set by the signal handler to end the main program.
 **/
volatile sig_atomic_t terminate = FALSE;

/**
*  signal handler.
* @brief In case SIGINT or SIGTERM occur, the global variable terminate will be set so the loop in the main function stops.
* @details global variables: terminate
**/
static void
handle_signal (int signal)
{
  terminate = TRUE;
}

/**
*  usage function.
* @brief In case the arguments when calling the program were invalid, this function prints what they should be.
* @details global variables: prog_name
**/
static void
usage (void)
{
  fprintf (stderr, "Usage %s takes no arguments! \n", prog_name);
  exit (EXIT_FAILURE);
}

/**
 * Starting point of the program.
 * @brief The main function provides the semaphores and shared memory, compares and prints solutions.
 * @details The signal handler is started first, then the semaphores and shared memory are created.
 * at any error, the program exits, freeing all semaphores and shared memory that have been created so far.
 * If a solution with no edges is stored in the shared memory by a generator, the supervisor main will print a message
 * and exit, otherwise it runs until SIGINT or SIGTERM occur.
 * global variables: prog_name, terminate
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 **/
int
main (int argc, char *argv[])
{

  struct solution best_solution;

  //configure signal handler
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));	//set all entries in struct to 0
  sa.sa_handler = handle_signal;	//set function pointer for handler
  sigaction (SIGINT, &sa, NULL);	//if SIGINT call handler
  sigaction (SIGTERM, &sa, NULL);	//if SIGTERM call handler

  prog_name = argv[0];
  //test if arguments were given
  if (argc != 1)
    {
      usage ();
    }

  memset (&best_solution, 0, sizeof (best_solution));	//initialise with zeros
  best_solution.number = SOLUTION_SIZE + 1;	//set initial number of edges in solution to invalid size, so first solution with SOLUTION_SIZE will smaller

  //create semaphores to coordinate access to circular buffer
  sem_t *sem_buffer = sem_open (SEM_BUFFER_ACCESS, O_CREAT | O_EXCL, 0600, 1);
  if (sem_buffer == SEM_FAILED)
    {
      fprintf (stderr, "%s: open of semaphore failed %s \n", prog_name,
	       strerror (errno));
      exit (EXIT_FAILURE);
    }
  sem_t *sem_free =
    sem_open (SEM_BUFFER_FREE, O_CREAT | O_EXCL, 0600, CIRCBUF_SIZE);
  if (sem_free == SEM_FAILED)
    {
      fprintf (stderr, "%s: open of semaphore failed %s \n", prog_name,
	       strerror (errno));
      //sem_close
      if (sem_close (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: closing of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }
  sem_t *sem_used = sem_open (SEM_BUFFER_USED, O_CREAT | O_EXCL, 0600, 0);
  if (sem_used == SEM_FAILED)
    {
      fprintf (stderr, "%s: open of semaphore failed %s \n", prog_name,
	       strerror (errno));
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //shm_open to create shared memory object
  int shared_open = shm_open (SHARED_MEMORY, O_RDWR | O_CREAT | O_EXCL, 0600);	//readwrite access and create if it doesn't exist yet, readwrite permission for current user
  if (shared_open == -1)
    {
      fprintf (stderr, "%s: open shared memory failed %s \n", prog_name,
	       strerror (errno));
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }
  //ftruncate to set size of shared memory object
  if (ftruncate (shared_open, sizeof (struct shared_struct)) < 0)
    {
      fprintf (stderr, "%s: setting size of shared memory failed %s \n",
	       prog_name, strerror (errno));
      //shm_unlink
      if (shm_unlink (SHARED_MEMORY) == -1)
	{
	  fprintf (stderr, "%s: unlink of shared memory failed %s \n",
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //mmap to create mapping to shared object
  struct shared_struct *shm_mapped;
  shm_mapped =
    mmap (NULL, sizeof (struct shared_struct), PROT_READ | PROT_WRITE,
	  MAP_SHARED, shared_open, 0);
  if (shm_mapped == MAP_FAILED)
    {
      fprintf (stderr, "%s: mapping of shared memory failed %s \n", prog_name,
	       strerror (errno));
      //shm_unlink
      if (shm_unlink (SHARED_MEMORY) == -1)
	{
	  fprintf (stderr, "%s: unlink of shared memory failed %s \n",
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }
  //close file descriptor
  if (close (shared_open) == -1)
    {
      fprintf (stderr, "%s: closing of file descriptor failed %s \n",
	       prog_name, strerror (errno));
      //munmap
      if (munmap (shm_mapped, sizeof (struct shared_struct)) == -1)
	{
	  fprintf (stderr, "%s: unmap of shared memory failed %s \n",
		   prog_name, strerror (errno));
	}
      //shm_unlink
      if (shm_unlink (SHARED_MEMORY) == -1)
	{
	  fprintf (stderr, "%s: unlink of shared memory failed %s \n",
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //set termination flag to false for generators
  if (sem_wait (sem_buffer) == -1)
    {
      fprintf (stderr, "%s: increase of semaphore failed %s \n", prog_name,
	       strerror (errno));
      //munmap
      if (munmap (shm_mapped, sizeof (struct shared_struct)) == -1)
	{
	  fprintf (stderr, "%s: unmap of shared memory failed %s \n",
		   prog_name, strerror (errno));
	}
      //shm_unlink
      if (shm_unlink (SHARED_MEMORY) == -1)
	{
	  fprintf (stderr, "%s: unlink of shared memory failed %s \n",
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }
  shm_mapped->terminate = FALSE;
  shm_mapped->generator_count = 0;
  for (int i = 0; i < SOLUTION_SIZE; i++)
    {
      shm_mapped->solutions[i].number = SOLUTION_SIZE + 1;
    }
  shm_mapped->generator_count = 0;
  if (sem_post (sem_buffer) == -1)
    {
      fprintf (stderr, "%s: increase of semaphore failed %s \n", prog_name,
	       strerror (errno));
      //munmap
      if (munmap (shm_mapped, sizeof (struct shared_struct)) == -1)
	{
	  fprintf (stderr, "%s: unmap of shared memory failed %s \n",
		   prog_name, strerror (errno));
	}
      //shm_unlink
      if (shm_unlink (SHARED_MEMORY) == -1)
	{
	  fprintf (stderr, "%s: unlink of shared memory failed %s \n",
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //sem_open to allow clients to access the shared memory
  if (sem_post (sem_free) == -1)
    {
      fprintf (stderr, "%s: increase of semaphore failed %s \n", prog_name,
	       strerror (errno));
      //munmap
      if (munmap (shm_mapped, sizeof (struct shared_struct)) == -1)
	{
	  fprintf (stderr, "%s: unmap of shared memory failed %s \n",
		   prog_name, strerror (errno));
	}
      //shm_unlink
      if (shm_unlink (SHARED_MEMORY) == -1)
	{
	  fprintf (stderr, "%s: unlink of shared memory failed %s \n",
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

      //sem_unlink
      if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_FREE) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      if (sem_unlink (SEM_BUFFER_USED) == -1)
	{
	  fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
		   strerror (errno));
	}
      exit (EXIT_FAILURE);
    }

  //read circular buffer until SIGINT or SIGTERM is received
  while (terminate == FALSE)
    {

      //wait for a solution
      if (sem_wait (sem_used) == -1)
	{
	  fprintf (stderr, "%s: wait for semaphore failed %s \n", prog_name,
		   strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}
      //wait for access to buffer
      if (sem_wait (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: wait for semaphore failed %s \n", prog_name,
		   strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}

      if (shm_mapped->solutions[shm_mapped->readindex].number <
	  best_solution.number)
	{
	  best_solution = shm_mapped->solutions[shm_mapped->readindex];
	  //if solution has 0 edges terminate
	  if (best_solution.number == 0)
	    {
	      fprintf (stdout, "The graph is 3-colorable!\n");
	      //tell generators to terminate
	      shm_mapped->terminate = TRUE;
	      if (sem_post (sem_free) == -1)
		{
		  fprintf (stderr, "%s: increase of semaphore failed %s \n",
			   prog_name, strerror (errno));
		  break;	//exit while loop to free semaphores and memory
		}
	      if (sem_post (sem_buffer) == -1)
		{
		  fprintf (stderr, "%s: increase of semaphore failed %s \n",
			   prog_name, strerror (errno));
		  break;	//exit while loop to free semaphores and memory
		}
	      break;
	    }
	  fprintf (stdout, "Solution with %d edges: ", best_solution.number);
	  for (int i = 0; i < best_solution.number; i++)
	    {
	      fprintf (stdout, "%d-%d ", best_solution.edges[i].vertex1,
		       best_solution.edges[i].vertex2);
	    }
	  printf ("\n");
	}
      shm_mapped->readindex += 1;
      shm_mapped->readindex %= CIRCBUF_SIZE;
      //end access to buffer
      if (sem_post (sem_buffer) == -1)
	{
	  fprintf (stderr, "%s: increase of semaphore failed %s \n",
		   prog_name, strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}

      //set another slot free
      if (sem_post (sem_free) == -1)
	{
	  fprintf (stderr, "%s: increase of semaphore failed %s \n",
		   prog_name, strerror (errno));
	  break;		//exit while loop to free semaphores and memory
	}
    }

  //generators should terminate as well, set free so they don't block
  sem_wait (sem_buffer);
  shm_mapped->terminate = TRUE;
  if (sem_post (sem_free) == -1)
    {
      fprintf (stderr, "%s: increase of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }
  if (sem_post (sem_buffer) == -1)
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
  //shm_unlink
  if (shm_unlink (SHARED_MEMORY) == -1)
    {
      fprintf (stderr, "%s: unlink of shared memory failed %s \n", prog_name,
	       strerror (errno));
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

  //sem_unlink
  if (sem_unlink (SEM_BUFFER_ACCESS) == -1)
    {
      fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }
  if (sem_unlink (SEM_BUFFER_FREE) == -1)
    {
      fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }
  if (sem_unlink (SEM_BUFFER_USED) == -1)
    {
      fprintf (stderr, "%s: unlink of semaphore failed %s \n", prog_name,
	       strerror (errno));
    }

  exit (EXIT_SUCCESS);

}
