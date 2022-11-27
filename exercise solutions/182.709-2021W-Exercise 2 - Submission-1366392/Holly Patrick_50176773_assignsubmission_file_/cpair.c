#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <regex.h>
#include <sys/wait.h>
#include <math.h>

#define BUFFERSIZE 100

struct Position
{
  float x;
  float y;
};

struct Position *
add (struct Position *array, float x, float y, int counter)
{
  if (counter == 0)
    {
      array[0].x = x;
      array[0].y = y;
    }
  else
    {
      struct Position *newpointer =
	realloc (array, (counter + 1) * sizeof (struct Position));
      array = newpointer;
      array[counter].x = x;
      array[counter].y = y;
    }
  return array;
}

void
concatPointers (char *dest, char *src)
{
  while (*dest)
    {
      dest++;
    }
  while (*src)
    {
      *dest = *src;
      src++;
      src++;
    }
  *dest = '\0';
}

char *
getString (struct Position pos[], int counter)
{

  char *s = (char *) malloc (200 * sizeof (char));

  sprintf (s, "%f %f\n", pos[0].y, pos[0].x);
  ;

  for (int i = 1; i < counter; i++)
    {
      sprintf (s, "%s%f %f\n", s, pos[i].y, pos[i].x);
    }
  return s;
}


double
getMeanX (struct Position *positionArray, int numberOfEntries)
{
  float x = 0.0;

  for (int i = 0; i < numberOfEntries; i++)
    {
      x = positionArray[i].x + x;
    };

  return x / numberOfEntries;
}

float
distance (int x1, int y1, int x2, int y2)
{
  // Calculating distance
  return sqrt (pow (x2 - x1, 2) + pow (y2 - y1, 2) * 1.0);
}


int
main (int argc, char *argv[])
{
  char buffer[BUFFERSIZE];
  struct Position *positionArray = malloc (sizeof (struct Position));
  int counter = 0;
  int reg;
  regex_t regex;

  reg =
    regcomp (&regex, "^[+-]?([0-9]*[.])?[0-9]+ [+-]?([0-9]*[.])?[0-9]+[\n]?$",
	     REG_EXTENDED);
  if (reg)
    {
      fprintf (stderr, "Could not compile regex\n");
      exit (EXIT_FAILURE);
    };
  if (isatty (STDIN_FILENO))
    {
      fprintf (stderr, "No input in stdin\n");
      exit (EXIT_FAILURE);
    };

  char *next;

  while (fgets (buffer, BUFFERSIZE, stdin))
    {
      reg = regexec (&regex, buffer, 0, NULL, 0);
      if (reg == REG_NOMATCH)
	{

	  fprintf (stderr, "Wrong input: %s", buffer);
	  regfree (&regex);
	  return EXIT_FAILURE;
	}


      float x = strtof (buffer, &next);
      float y = strtof (next, NULL);
      positionArray = add (positionArray, x, y, counter);

      counter++;
    };


  regfree (&regex);
  if (counter == 1)
    {
      free (positionArray);
      return EXIT_SUCCESS;
    };
  if (counter == 2)
    {				//Prints when only 2 pairs exist
      for (int i = 0; i < counter; i++)
	{
	  printf ("X:%f Y:%f\n", positionArray[i].x, positionArray[i].y);
	}
      fflush (stdout);
      return EXIT_SUCCESS;
    };

  struct Position *biggerArray = malloc (sizeof (struct Position));
  struct Position *smallerArray = malloc (sizeof (struct Position));


  float meanX = getMeanX (positionArray, counter);

  int counterBigger = 0;
  int counterSmaller = 0;

  int even = 0;

  for (int i = 0; i < counter; i++)
    {

      if (positionArray[i].x > meanX)
	{
	  biggerArray =
	    add (biggerArray, positionArray[i].x, positionArray[i].y,
		 counterBigger);
	  counterBigger++;
	};

      if (positionArray[i].x < meanX)
	{
	  smallerArray =
	    add (smallerArray, positionArray[i].x, positionArray[i].y,
		 counterSmaller);
	  counterSmaller++;
	};

      if (positionArray[i].x == meanX)
	{
	  if (even % 2 == 0)
	    {
	      biggerArray =
		add (biggerArray, positionArray[i].x, positionArray[i].y,
		     counterBigger);
	      counterBigger++;
	    }
	  else
	    {
	      smallerArray =
		add (smallerArray, positionArray[i].x, positionArray[i].y,
		     counterSmaller);
	      counterSmaller++;
	    };
	  even++;
	};

    };


  //Creates 2 new Forcs ans saves the process id's


  //fd = open("path",0600)
  //f = fdopen(fd,'r')


  int pipefd[2];
  pipe (pipefd);

  pid_t firstfork = fork ();

  switch (firstfork)
    {
    case -1:
      fprintf (stderr, "Cannot fork!\n");
      exit (EXIT_FAILURE);
    case 0:
      dup2 (pipefd[0], STDIN_FILENO);
      dup2 (pipefd[1], STDOUT_FILENO);
      execlp ("./cpair", "cpair", NULL);

      break;
    default:;

      char *result = getString (smallerArray, counterSmaller);
      int length = sprintf (result, "%s", result);
      write (pipefd[1], result, length);
      close (pipefd[1]);
    };

  int pipefd2[2];
  pipe (pipefd2);
  pid_t secondfork = fork ();

  switch (secondfork)
    {
    case -1:
      fprintf (stderr, "Cannot fork!\n");
      exit (EXIT_FAILURE);
    case 0:
      dup2 (pipefd2[0], STDIN_FILENO);
      dup2 (pipefd2[1], STDOUT_FILENO);
      execlp ("./cpair", "cpair", NULL);

      break;
    default:;
      char *result = getString (biggerArray, counterBigger);
      int length = sprintf (result, "%s", result);
      write (pipefd2[1], result, length);
      close (pipefd2[1]);
    };

  float bestdistance = 10000000;
  float dist = 0;

  float bestx1 = 0;
  float besty1 = 0;

  float bestx2 = 0;
  float besty2 = 0;


  for (int i = 0; i < counterSmaller; i++)
    {
      for (int ii = 0; ii < counterBigger; ii++)
	{
	  dist =
	    distance (smallerArray[i].x, smallerArray[i].y, biggerArray[ii].x,
		      biggerArray[ii].y);
	  if (dist < bestdistance)
	    {
	      bestdistance = dist;
	      bestx1 = smallerArray[i].x;
	      besty1 = smallerArray[i].y;
	      bestx2 = biggerArray[ii].x;
	      besty2 = biggerArray[ii].y;
	    }
	};
    };




  char buf[4096];
  ssize_t count = read (pipefd[0], buf, sizeof (buf));
  close (pipefd[0]);
  if (count == -1)
    {
      perror ("read");
      exit (EXIT_FAILURE);
    };

  float x1 = strtof (buf, &next);
  float y1 = strtof (next, &next);

  float x2 = strtof (next, &next);
  float y2 = strtof (next, NULL);

  dist = distance (x1, y1, x2, y2);
  if (dist < bestdistance)
    {
      bestx1 = x1;
      besty1 = y1;
      bestx2 = x2;
      besty2 = y2;
    }


  char buf2[4096];
  ssize_t count2 = read (pipefd2[0], buf2, sizeof (buf2));
  close (pipefd2[0]);
  if (count2 == -1)
    {
      perror ("read");
      exit (EXIT_FAILURE);
    };

  x1 = strtof (buf2, &next);
  y1 = strtof (next, &next);
  x2 = strtof (next, &next);
  y2 = strtof (next, NULL);

  dist = distance (x1, y1, x2, y2);
  if (dist < bestdistance)
    {
      bestx1 = x1;
      besty1 = y1;
      bestx2 = x2;
      besty2 = y2;
    }


  int status;
  waitpid (firstfork, &status, WNOHANG);

  waitpid (secondfork, &status, WNOHANG);


  printf ("%f %f\n%f %f\n", bestx1, besty1, bestx2, besty2);

  free (positionArray);
  free (biggerArray);
  free (smallerArray);

  return EXIT_SUCCESS;
}
