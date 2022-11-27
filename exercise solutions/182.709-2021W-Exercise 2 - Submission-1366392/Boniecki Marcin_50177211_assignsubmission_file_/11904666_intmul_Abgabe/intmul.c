#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
/**
 * @brief Indices for the pipe. READ is where the pipe from which the child reads and WRITE is the indice where the child writes into.
 */
#define READ 0
#define WRITE 1
/**
 * @brief Indices for the pipe.
 */
#define IN 0
#define OUT 1
/**
 * @brief Indices for the result array.
 */
#define AH 0
#define AL 1
#define BH 2
#define BL 3
/**
 * @brief Constant for the length of the children.
 */
#define CHL_LEN 4
/**
 * @brief Macro for printing an error message and exiting the program.
 */
#define HANDLE_ERR(msg)                                                                            \
{                                                                                                  \
 fprintf(stderr, "[%s](%d): %s: %s\n", __FILE__, __LINE__, msg, strerror(errno));                  \
 exit(EXIT_FAILURE);                                                                               \
}

size_t get_numbers (char **a, char **b);
void ipc (size_t numlen, char *a, char *b);
void strhalf (char **s1, char **s2, char *s, size_t len);
void write_pipe (int pfd, char *a, char *b);
void handle_wait ();
void read_pipes (char *result[]);
char *calc_mul (char *result[], size_t len);
void add_zeros (char **hex, size_t zeros);
char *add_hex (char *hex1, char *hex2);
void strrev (char **s);
/**
 * @brief Array for storing pipes.
 */
int pipefd[CHL_LEN][2][2];

/**
 * @brief the main function of the program
 *
 * @param argc the argument counter
 * @param argv the arcuments for the program
 * @return the exit code (EXIT_SUCCESS if the program ends successfully, EXIT_FAILURE if the program terminates due to an error)
 */
int
main (int argc, char *argv[])
{
  char *a, *b;
  size_t numlen = get_numbers (&a, &b);

//If the input consists of 1 hexadecimal digit
  if (numlen == 1)
    {
      fprintf (stdout, "%x\n",
	       ((unsigned int) strtol (a, NULL, 16)) *
	       ((unsigned int) strtol (b, NULL, 16)));

      free(b);
      free(a);

      fflush (stdout);
    }
  else
    {
      ipc (numlen, a, b);

      handle_wait ();

      char *inputs[CHL_LEN];

      read_pipes (inputs);
      char *result = calc_mul (inputs, numlen);

      printf ("%s\n", result);
      
      free(result);

      fflush (stdout);
    }
    exit (EXIT_SUCCESS);
}
/**
 * @brief The method gets the two hexadecimal numbers from the input and writes them into a and b. Both numbers have to have the same length.
 * 
 * @param h1 The first hexadecimal number.
 * @param h2 The second hexadecimal number.
 * @return size_t The length of the hexadecimal numbers.
 */
size_t
get_numbers (char **a, char **b)
{
  size_t len = 0;

  char *line;
  size_t n = 0;
  ssize_t linelen;
  while ((linelen = getline (&line, &n, stdin)) != -1)
    {

      if (linelen == 1)
	HANDLE_ERR ("The line is empty");
      if (len != 0 && linelen - 1 != len)
	HANDLE_ERR ("Both numbers don't have the same length");

      if (len == 0)
	{
	  *a = malloc (linelen);
	  strcpy (*a, line);
	  len = linelen - 1;
	  if (len % 2 != 0 && len != 1)
	    HANDLE_ERR ("The length of the number is not even");
	}
      else
	{
	  *b = malloc (linelen);
	  strcpy (*b, line);
	  break;
	}
    }

  return len;
}

/**
 * @brief Four child processes are created and pipes for the input and output are created.
 * 
 * @param numlen Lenght of each of the numbers.
 * @param a First hexadecimal number.
 * @param b Second hexadecimal number.
 */
void
ipc (size_t numlen, char *a, char *b)
{
  //splitted A and B
  char *splitted[CHL_LEN];

  strhalf (&splitted[AH], &splitted[AL], a, numlen);
  strhalf (&splitted[BH], &splitted[BL], b, numlen);
  
  free (b);
  free (a);

  pid_t pid[CHL_LEN];

  for (int i = 0; i < CHL_LEN; i++)
    {
      pipe (pipefd[i][READ]);
      pipe (pipefd[i][WRITE]);

      pid[i] = fork ();

      switch (pid[i])
	{
	case -1:
	  HANDLE_ERR ("Cannot fork!");
	case 0:
	  // child tasks
	  close (pipefd[i][READ][OUT]);
	  close (pipefd[i][WRITE][IN]);

	  if (dup2 (pipefd[i][READ][IN], STDIN_FILENO) == -1
	      || dup2 (pipefd[i][WRITE][OUT], STDOUT_FILENO) == -1)
	    HANDLE_ERR ("Cannot redirect the pipe!");
	  close (pipefd[i][READ][IN]);
	  close (pipefd[i][WRITE][OUT]);

	  execlp ("./intmul", "./intmul", NULL);
	  HANDLE_ERR ("Cannot exec intmul!");
	  break;
	default:
	  // parent tasks
	  close (pipefd[i][READ][IN]);
	  close (pipefd[i][WRITE][OUT]);

	  switch (i)
	    {
	    case 0:
	      write_pipe (pipefd[i][READ][OUT], splitted[AH], splitted[BH]);
	      break;
	    case 1:
	      write_pipe (pipefd[i][READ][OUT], splitted[AH], splitted[BL]);
	      break;
	    case 2:
	      write_pipe (pipefd[i][READ][OUT], splitted[AL], splitted[BH]);
	      break;
	    default:
	      write_pipe (pipefd[i][READ][OUT], splitted[AL], splitted[BL]);
	      break;
	    }
	  break;
	}
    }

  for (int i = 0; i < CHL_LEN; i++)
    {
      free (splitted[i]);
    }
}
/**
 * @brief Seperates a string into two substrings s1 and s2.
 * 
 * @param s1 First half of the string.
 * @param s2 Second half of the string.
 * @param s String that has will be seperated.
 * @param len Length of the string.
 */
void
strhalf (char **s1, char **s2, char *s, size_t len)
{
  *s1 = malloc (len / 2 + 2);
  *s2 = malloc (len / 2 + 2);

  strncpy (*s1, s, len / 2);
  strcpy (*s2, &s[len / 2]);

  (*s1)[len / 2] = '\n';
  (*s2)[len / 2] = '\n';

  (*s1)[len / 2 + 1] = '\0';
  (*s2)[len / 2 + 1] = '\0';
}
/**
 * @brief Funktion that writes a string into a path.
 * 
 * @param pfd The path id for the input file.
 * @param a The first string that is going to be written into the file.
 * @param b The second string that is going to be written into the file.
 */
void
write_pipe (int pfd, char *a, char *b)
{
  FILE *input;

  if ((input = fdopen (pfd, "w")) == NULL)
    HANDLE_ERR ("Cannot open pipe!");

  if ((fputs (a, input)) == -1)
    HANDLE_ERR ("Cannot write to pipe!");

  if ((fputs (b, input)) == -1)
    HANDLE_ERR ("Cannot write to pipe!");

  fflush (input);

  if (fclose(input) < 0)
    HANDLE_ERR("Cannot close pipe!");
}
/**
 * @brief Funktion that reads the output of the children.
 * 
 * @param result Array where the result of the children is written into.
 */
void
read_pipes (char *result[4])
{
  FILE *output;

  for (int i = 0; i < CHL_LEN; i++)
    {
      if((output = fdopen (pipefd[i][WRITE][IN], "r")) == NULL)
        HANDLE_ERR ("Cannot open pipe!");

      char *line;
      size_t n = 0;
      ssize_t linelen;

      if ((linelen = getline (&line, &n, output)) == -1)
	      HANDLE_ERR ("Cannot read the result");

      result[i] = malloc (strlen (line));
      strcpy (result[i], line);
      result[i][strlen (line) - 1] = '\0';

      if (fclose(output) < 0)
        HANDLE_ERR("Cannot close pipe!");
    }
}
/**
 * @brief Handles the waiting for the children.
 * 
 */
void
handle_wait ()
{
  int status;

  while (wait (&status) != -1)
    {
      if (errno == EINTR)
	continue;
      if (WEXITSTATUS (status) != EXIT_SUCCESS)
	HANDLE_ERR ("Cannot wait!");
    }
}
/**
 * @brief Adds the results from the children coreesponding to the assignment.
 * 
 * @param result Results from the children.
 * @param len Length of the children.
 * @return char* Result from the calculation.
 */
char *
calc_mul (char *result[], size_t len)
{
  char *rv;

  add_zeros (&result[0], len);
  add_zeros (&result[1], len / 2);
  add_zeros (&result[2], len / 2);

  rv = add_hex (result[0], result[1]);
  rv = add_hex (rv, result[2]);
  rv = add_hex (rv, result[3]);

  for (int i = 0; i < CHL_LEN; i++)
  {
    free (result[i]);
  }

  return rv;
}
/**
 * @brief Adds zeros at the tail of the string.
 * 
 * @param hex Character where the strings have to be added.
 * @param zeros The amount of the zeros that have to be added.
 */
void
add_zeros (char **hex, size_t zeros)
{
  int hex_len = strlen (*hex);
  char *rv = malloc (hex_len + zeros + 1);

  memset (rv, '0', hex_len + zeros);
  memcpy (rv, *hex, hex_len);
  rv[hex_len + zeros] = '\0';

  *hex = rv;
}
/**
 * @brief Funktion that adds two hexadecimal numbers
 * 
 * @param hex1 First hexadecimal number.
 * @param hex2 Second hexadecimal number.
 * @return char* Result of the addition.
 */
char *
add_hex (char *hex1, char *hex2)
{
  char *rv, temp[3];
  char hex1_b, hex2_b;
  size_t len;

  strrev (&hex1);
  strrev (&hex2);

  if (strlen (hex1) > strlen (hex2))
    {
      len = strlen (hex1) + 1;
    }
  else
    {
      len = strlen (hex2) + 1;
    }
  rv = malloc (len + 1);

  for (int i = 0; i < len; i++)
    {
      if (i >= strlen (hex1))	//one string is empty
	hex1_b = '0';
      else
	hex1_b = hex1[i];
      if (i >= strlen (hex2))
	hex2_b = '0';
      else
	hex2_b = hex2[i];
      sprintf (temp, "%x", ((unsigned int) strtol ((char[2])
						   {
						   hex1_b, '\0'}, NULL,
						   16)) +
	       ((unsigned int) strtol ((char[2])
				       {
				       hex2_b, '\0'}, NULL,
				       16)) +
	       ((unsigned int) strtol ((char[2])
				       {
				       temp[0], '\0'}, NULL, 16)));
      if (strlen (temp) == 1)
	{
	  rv[i] = temp[0];
	  temp[0] = 0;
	}
      else
	{
	  rv[i] = temp[1];
	}
    }
  if (rv[len - 1] == '0')
    {
      rv[len - 1] = '\0';
      rv = realloc (rv, len);
    }
  else
    rv[len] = '\0';
  strrev (&rv);
  return rv;
}
/**
 * @brief Function that reverses a string.
 * 
 * @param s String that will be reversed.
 */
void
strrev (char **s)
{
  char *rv = malloc ((strlen (*s) + 1) * sizeof (char));
  strcpy (rv, *s);

  char temp;
  size_t len = strlen (rv);

  for (int i = 0; i < len / 2; i++)
    {
      temp = rv[i];
      rv[i] = rv[len - i - 1];
      rv[len - i - 1] = temp;
    }

  *s = rv;
}
