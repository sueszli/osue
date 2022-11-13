/**
 * @author Stefan Brandmair
 * @date 2021-11-21
 *
 * @brief Fast Forier Transform!
 * @details Computes the FFT of a bunch of values. Forks itself twice and gets the child processes to do some of the work.
 **/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <stdbool.h>

char *PROGRAM_NAME;

/**
 * @brief Prints an error and exits
 * @param message message to print
 * @return void
 */
static void panicErr(char *message)
{
  fprintf(stderr, "[%s] %s - %s\n", PROGRAM_NAME, message, strerror(errno));
  exit(EXIT_FAILURE);
}

// TODO: Maybe an "open scope, close scope" design would work better?
struct ValueToFree
{
  void *ptr;
  struct ValueToFree *next;
};

struct ValueToFree *freeHead = NULL;

/**
 * @brief Designed to be called on exit, to free all values
 * @return void
 */
static void freeValues(void)
{
  while (freeHead != NULL)
  {
    struct ValueToFree *value = freeHead;
    freeHead = freeHead->next;

    free(value->ptr);
    free(value);
  }
}

/**
 * @brief Registers a value to be freed
 * @param ptr pointer to be freed when the program exits
 * @return void
 */
static void freeAtExit(void *ptr)
{
  struct ValueToFree *value = malloc(sizeof(struct ValueToFree));
  value->ptr = ptr;
  value->next = freeHead;
  freeHead = value;
}

struct FloatList
{
  float *values;
  int count;
  int capacity;
};

struct ChildProcess
{
  pid_t pid;
  FILE *stdinPipe;
  FILE *stdoutPipe;
};

struct ComplexNumber
{
  float re;
  float im;
};

/**
 * @brief Inserts a value into our list. Potentially resizes the list if it's too small
 * @param list list to use
 * @param value value to insert
 * @param unusedFds file descriptors to close, because fork duplicates the file descriptors
 * @param unusedFdsCount number of unused file descriptors
 * @return void
 */
static struct ChildProcess forkChildProcess(int *unusedFds, size_t unusedFdsCount)
{
  int stdinPipefd[2];
  if (pipe(stdinPipefd) == -1)
  {
    panicErr("Failed to create pipe");
  }
  int stdoutPipefd[2];
  if (pipe(stdoutPipefd) == -1)
  {
    panicErr("Failed to create pipe");
  }

  pid_t processId = fork();
  if (processId == -1)
  {
    panicErr("Failed to fork");
    struct ChildProcess dummy = {0};
    return dummy;
  }
  else if (processId == 0)
  {
    // Child
    if (close(stdinPipefd[1]) == -1)
    {
      panicErr("Failed to close the inputting stdin-pipe end in the child");
    }
    if (close(stdoutPipefd[0]) == -1)
    {
      panicErr("Failed to close the reading stdout-pipe end in the child");
    }

    dup2(stdinPipefd[0], STDIN_FILENO);
    if (close(stdinPipefd[0]) == -1)
    {
      panicErr("Failed to close the pipe");
    }

    dup2(stdoutPipefd[1], STDOUT_FILENO);
    if (close(stdoutPipefd[1]) == -1)
    {
      panicErr("Failed to close the pipe");
    }

    for (size_t i = 0; i < unusedFdsCount; i++)
    {
      if (close(unusedFds[i]) == -1)
      {
        panicErr("Failed to close the unused file descriptor");
      }
    }

    // We need to close the file descriptors and more
    // https://stackoverflow.com/a/31476215

    // No need to free anything in this case because exec
    // https://stackoverflow.com/questions/23440132/fork-after-malloc-in-parent-does-the-child-process-need-to-free-it
    execlp(PROGRAM_NAME, PROGRAM_NAME, NULL);

    panicErr("Exec failed");
    struct ChildProcess dummy = {0};
    return dummy;
  }
  else
  {
    // Parent
    if (close(stdinPipefd[0]) == -1)
    {
      panicErr("Failed to close the pipe");
    }
    if (close(stdoutPipefd[1]) == -1)
    {
      panicErr("Failed to close the pipe");
    }

    struct ChildProcess child = {0};
    child.pid = processId;

    child.stdinPipe = fdopen(stdinPipefd[1], "w");
    if (child.stdinPipe == NULL)
    {
      panicErr("Failed to open child stdin pipe");
    }
    child.stdoutPipe = fdopen(stdoutPipefd[0], "r");
    if (child.stdoutPipe == NULL)
    {
      panicErr("Failed to open child stdout pipe");
    }

    return child;
  }
}

/**
 * @brief Inserts a value into our list. Potentially resizes the list if it's too small
 * @param list list to use
 * @param value value to insert
 * @return void
 */
static void insertFloat(struct FloatList *list, float value)
{
  if (list->count >= list->capacity)
  {
    // Double the list's allocated size
    list->capacity *= 2;
    list->values = realloc(list->values, sizeof(float) * list->capacity);
    if (list->values == NULL)
    {
      panicErr("Realloc failed");
    }
  }

  list->values[list->count] = value;
  list->count += 1;
}

/**
 * @brief Reads a complex number from the text, imaginary part is optional
 * @param complexNumber result gets written here
 * @param text char array containing a number like 1 4
 * @param length length of text
 * @return whether it was successful or not
 */
static bool tryReadComplexNumber(struct ComplexNumber *complexNumber, char *text, int length)
{
  // Read real part
  errno = 0;
  char *endPointer = text + length;
  complexNumber->re = strtof(text, &endPointer);
  if (errno != 0 || endPointer == text)
  {
    return false;
  }
  // Read imaginary part
  char *nextText = endPointer;
  endPointer = text + length;
  complexNumber->im = strtof(nextText, &endPointer);
  if (errno != 0 || endPointer == nextText)
  {
    // No imaginary part
    complexNumber->im = 0;
    errno = 0;
  }

  return true;
}

static struct ComplexNumber addComplex(struct ComplexNumber *valueA, struct ComplexNumber *valueB)
{
  struct ComplexNumber result = {
      .re = valueA->re + valueB->re,
      .im = valueA->im + valueB->im,
  };
  return result;
}

static struct ComplexNumber multiplyComplex(struct ComplexNumber *valueA, struct ComplexNumber *valueB)
{
  struct ComplexNumber result = {
      .re = (valueA->re * valueB->re) - (valueA->im * valueB->im),
      .im = (valueA->re * valueB->im) + (valueA->im * valueB->re),
  };
  return result;
}

int main(int argc, char *argv[])
{
  PROGRAM_NAME = argv[0];
  atexit(freeValues);

  // Argument parsing
  {
    int option;
    while ((option = getopt(argc, argv, "h")) != -1)
    {
      switch (option)
      {
      case 'h':
      {
        fprintf(stdout, "Usage %s\n", PROGRAM_NAME);
        exit(EXIT_SUCCESS);
        break;
      }

      default:
      {
        break;
      }
      }
    }
  }

  struct FloatList values = {0};
  { // Oh damn, I'm writing worse code due to the guidelines. Adding a method involves adding a lot of documentation...
    int initialSize = 8;
    values.values = malloc(initialSize * sizeof(float));
    values.capacity = initialSize;
    if (values.values == NULL)
    {
      panicErr("Malloc returned null");
    }
    freeAtExit(values.values);
  }

  // Read from stdin
  {
    int inputLength = 0;
    size_t inputAllocatedSize = 0;
    char *inputLine = NULL;
    while ((inputLength = getline(&inputLine, &inputAllocatedSize, stdin)) != -1) // Ctr+D
    {
      errno = 0; // Reset errno before calling strtof
      char *endPointer = inputLine + inputLength;
      float value = strtof(inputLine, &endPointer);
      if (errno != 0 || endPointer == inputLine)
      {
        free(inputLine);
        panicErr("Parsing number failed");
      }

      // Now we can use that value
      insertFloat(&values, value);
    }
    free(inputLine);
  }

  if (values.count <= 0)
  {
    panicErr("No values read (error)");
  }

  if (values.count == 1)
  {
    fprintf(stdout, "%f\n", values.values[0]);
  }
  else
  {
    if (values.count % 2 != 0)
    {
      panicErr("Expected an even number of values");
    }

    struct ChildProcess children[2];
    children[0] = forkChildProcess(NULL, 0);

    int child0FileDescriptors[2] = {fileno(children[0].stdinPipe), fileno(children[0].stdoutPipe)};
    children[1] = forkChildProcess(child0FileDescriptors, 2);

    // Now time to feed them the values
    for (int i = 0; i < values.count; i += 2)
    {
      fprintf(children[0].stdinPipe, "%f\n", values.values[i]); // Don't forget the line break
      fprintf(children[1].stdinPipe, "%f\n", values.values[i + 1]);
    }

    // Close the pipes (so that the children get EOF)
    if (fclose(children[0].stdinPipe) == EOF)
    {
      panicErr("Failed to close child pipe");
    }
    if (fclose(children[1].stdinPipe) == EOF)
    {
      panicErr("Failed to close child pipe");
    }

    struct ComplexNumber *evenValues = malloc((values.count / 2) * sizeof(struct ComplexNumber));
    struct ComplexNumber *oddValues = malloc((values.count / 2) * sizeof(struct ComplexNumber));
    freeAtExit(evenValues);
    freeAtExit(oddValues);
    // Read the results from the children
    {
      int inputLength = 0;
      size_t inputCapacity = 0;
      char *inputLine = NULL;
      int halfCount = values.count / 2;

      for (int i = 0; i < halfCount; i++)
      {
        if ((inputLength = getline(&inputLine, &inputCapacity, children[0].stdoutPipe)) == -1)
        {
          free(inputLine);
          fclose(children[0].stdoutPipe);
          fclose(children[1].stdoutPipe);
          panicErr("Child 0 didn't write a value");
        }
        if (tryReadComplexNumber(&evenValues[i], inputLine, inputLength) == false)
        {
          free(inputLine);
          fclose(children[0].stdoutPipe);
          fclose(children[1].stdoutPipe);
          panicErr("Parsing number from child 0 failed");
        }

        if ((inputLength = getline(&inputLine, &inputCapacity, children[1].stdoutPipe)) == -1)
        {
          free(inputLine);
          fclose(children[0].stdoutPipe);
          fclose(children[1].stdoutPipe);
          panicErr("Child 1 didn't write a value");
        }
        if (tryReadComplexNumber(&oddValues[i], inputLine, inputLength) == false)
        {
          free(inputLine);
          fclose(children[0].stdoutPipe);
          fclose(children[1].stdoutPipe);
          panicErr("Parsing number from child 1 failed");
        }
      }
      free(inputLine);

      if (fclose(children[0].stdoutPipe) == EOF)
      {
        panicErr("Failed to close child pipe");
      }
      if (fclose(children[1].stdoutPipe) == EOF)
      {
        panicErr("Failed to close child pipe");
      }
    }

    int childStatus;
    if (waitpid(children[0].pid, &childStatus, 0) == -1)
    {
      panicErr("Waiting for child 0 failed");
    }
    if (childStatus != EXIT_SUCCESS)
    {
      panicErr("Child did not succeed");
    }
    if (waitpid(children[1].pid, &childStatus, 0) == -1)
    {
      panicErr("Waiting for child 1 failed");
    }
    if (childStatus != EXIT_SUCCESS)
    {
      panicErr("Child did not succeed");
    }

    struct ComplexNumber *results = malloc(values.count * sizeof(struct ComplexNumber));
    freeAtExit(results);
    // Do computations
    {
      const float PI = 3.141592654;
      int halfCount = values.count / 2;
      for (int i = 0; i < halfCount; i++)
      {
        float factor = -2 * PI / (float)values.count * i;
        struct ComplexNumber factorComplex =
            {
                .re = cosf(factor),
                .im = sinf(factor),
            };

        struct ComplexNumber multiplicationResult = multiplyComplex(&factorComplex, &(oddValues[i]));

        results[i] = addComplex(&evenValues[i], &multiplicationResult);

        // Subtract
        multiplicationResult.re *= -1;
        multiplicationResult.im *= -1;
        results[i + halfCount] = addComplex(&evenValues[i], &multiplicationResult);
      }
    }

    // Write values
    for (int i = 0; i < values.count; i++)
    {
      fprintf(stdout, "%f %f*i\n", results[i].re, results[i].im);
    }
    fflush(stdout);

    //free(evenValues);
    //free(oddValues);
    //free(results);
  }

  //free(values.values);
  return EXIT_SUCCESS;
}