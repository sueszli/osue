#include <stdio.h>
#include <stdlib.h>		//onexit
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>		/* memset */
#include <errno.h>

#define SHM_NAME "/myshm"

#define SEM_1 "/access"		//For all generators
#define SEM_2 "/read"		//For the supervisor
#define SEM_3 "/inaccess"	// for generators th signal that they are reserving a spot.
#define SEM_4 "/loggedIn" 
//generators log themselves in by increasing sem_4 so that the supervisor 
//knows the number of clients

struct myshm
{
  unsigned int exit;
  unsigned int read_pos;
  unsigned int write_pos;
  char data[50][64];
  unsigned int deleted[50];
};

volatile sig_atomic_t quit = 0;

void
handle_signal (int signal)
{
  quit = 1;
}


char *myprog;
int
main (int argc, char *argv[])
{
  myprog = argv[0];

  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = handle_signal;
  sigaction (SIGINT, &sa, NULL);
  sigaction (SIGTERM, &sa, NULL);
  sigaction (SIGKILL, &sa, NULL);



  int bestsolution = 1000;	//very bad solution


  int shmfd = shm_open (SHM_NAME, O_RDWR | O_CREAT, S_IRWXU);
  if (shmfd == -1)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      return EXIT_FAILURE;
    }

  if (ftruncate (shmfd, sizeof (struct myshm)) < 0)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      return EXIT_FAILURE;
    }

  struct myshm *myshm;
  myshm =
    mmap (NULL, sizeof (*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd,
	  0);
  if (myshm == MAP_FAILED)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      return EXIT_FAILURE;
    }

  sem_t *sem_access = sem_open (SEM_1, O_CREAT | O_EXCL, 0600, 29);
  sem_t *sem_read = sem_open (SEM_2, O_CREAT | O_EXCL, 0600, 0);
  sem_t *sem_inAccess = sem_open (SEM_3, O_CREAT | O_EXCL, 0600, 1);	//each time only 1 is allowed to pick a number 
  sem_t *sem_loggedIn = sem_open (SEM_4, O_CREAT | O_EXCL, 0600, 0);

  if (sem_access == SEM_FAILED)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };

  if (sem_read == SEM_FAILED)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };

  if (sem_inAccess == SEM_FAILED)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };

  if (sem_loggedIn == SEM_FAILED)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };

  //sets deleted bigger than possible to avoid a wrong positive due to an error.
  for (int i = 0; i < 50; i++)
    {

      myshm->deleted[i] = 9;	//since we only get solutions with 8 or lower
    };
  myshm->exit = 0;
  while (!quit)
    {
      sem_wait (sem_read);
      if (quit == 0)
	{
	  int solution = myshm->deleted[myshm->read_pos];	//Number of deleted Nodes
	  if (solution == 0)
	    {
	      printf ("The graph is acyclic!\n");
	      quit = 1;

	    }
	  else
	    {
	      if (solution < bestsolution)
		{
		  printf ("Solution with %i edges:%s\n",
			  myshm->deleted[myshm->read_pos],
			  myshm->data[myshm->read_pos]);
		  bestsolution = solution;
		}
	    }
	  sem_post (sem_access);
	  myshm->read_pos = (myshm->read_pos + 1) % 50;
	}
    }

//waiting for the generators to close
  myshm->exit = 1;
  int value = 1;
  while (value != 0)
    {
      sem_post (sem_access);
      sem_getvalue (sem_loggedIn, &value);
    }


  if (close (shmfd) == -1)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };

  if (munmap (myshm, sizeof (*myshm)) == -1)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };

  if (shm_unlink (SHM_NAME) == -1)
    {
      fprintf (stderr, " [%s] ERROR: %s \n", myprog, strerror (errno));
      exit (EXIT_FAILURE);
    };


  sem_close (sem_access);
  sem_close (sem_inAccess);
  sem_close (sem_read);
  sem_close (sem_loggedIn);

  sem_unlink (SEM_1);
  sem_unlink (SEM_2);
  sem_unlink (SEM_3);
  sem_unlink (SEM_4);
  return EXIT_SUCCESS;
}
