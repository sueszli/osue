/**
 * @file main.c
 * @brief Main file of 'cpair'
 * @author Matthias Preymann (12020638)
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>

/** Maximum length of line read from stdin **/
#define MAX_INPUT_LINE_LEN (1024)

/** Initial capacity to be allocated for a new point array **/
#define POINT_ARRAY_INIT_CAP (16)


//#define _SHOW_DEBUG_PRINT_

/** Implementation of the debug printer **/
#define _printDebugImpl( ... ) do { fprintf(stderr, __VA_ARGS__ ); fprintf(stderr, "\n" ); } while(0)

#ifdef _SHOW_DEBUG_PRINT_
  /** Print to stderr with new line **/
  #define printDebug( ... ) _printDebugImpl( __VA_ARGS__ )
#else
  /** Don't print anything and don't evaluate the arguments **/
  #define printDebug( ... ) do { if(0) { _printDebugImpl( __VA_ARGS__ ); } } while(0)
#endif


/** @brief A two dimensional point consisting of a x and y component **/
typedef struct {
  /** x position of the point **/
  float x;
  /** y position of the point **/
  float y;
} Point2D;

/** @brief A pair of two points **/
typedef struct {
  /** First point **/
  Point2D a;
  /** Second point **/
  Point2D b;
} Point2DPair;

/** @brief Dynamic array of two dimensional points **/
typedef struct {
  /** Pointer to array of points **/
  Point2D* points;
  /** Number of points stored **/
  int length;
  /** Number of points that could fit into the array **/
  int capacity;
} Point2DArray;

/** @brief Contains all handles of a forked child process **/
typedef struct {
  /** PID of the child process **/
  pid_t pid;
  /** File descriptor of the childs piped stdin **/
  int stdinFd;
  /** File descriptor of the childs piped stdout **/
  int stdoutFd;
} ChildProcess;

/** Path of the executed binary (argv[0]) **/
static char* binaryPath;


/**
 * @brief Prints an error to stderr with two inline message strings
 * @param msg First message to print
 * @param msg2 Second message to print
 */
static void printError2(const char* msg, const char* msg2) {
  fprintf(stderr, "[%s] Error: %s: %s\n", binaryPath, msg, msg2);
}

/**
 * @brief Prints an error to stderr with an inline message string
 * @param msg Message to print
 */
static void printError(const char* msg) {
  printError2(msg, "");
}

/**
 * @brief Prints an error to stderr with the current errno as string
 *        and an inline message string
 * @param msg Custom message to print
 */
static void printWithErrno(const char* msg) {
  printError2(msg, strerror(errno));
}

/**
 * @brief Returns the greater of two numbers
 * @param a First number
 * @param b Second number
 * @return The greater number
 */
static int imax(int a, int b) {
  return a > b ? a : b;
}

/**
 * @brief Writes a specified number of space-characters to stdout
 * @param len Number of spaces-chars to print
 */
static void putWhitspace(int len) {
  while( len-- > 0 ) {
    fputc(' ', stdout);
  }
}

/**
 * @brief Prints a point to a file descriptor in a formatted way
 * @param fd File descriptor to print to
 * @param point Point to print
 * @return < 0 on error
 */
static int printPoint(int fd, Point2D point ) {
  char buffer[256];
  int len= snprintf(buffer, 256, "%f %f\n", point.x, point.y);
  if( len >= 256 ) {
    printError("Could not print point");
    return -1;
  }
  return write(fd, buffer, len);
}

/**
 * @brief Jumps over the all leading empty lines ('\n') in a string
 * @param s String to search through
 * @return Pointer to the first non-empty line in the string
 */
static char* jumpEmptyLines(char* s) {
  while(*s == '\n') {
    s++;
  }

  return s;
}

/**
 * @brief Converts all '\n' in a string into '\0' creating inline
 *        substrings. Counts the full length of the buffer and the
 *        length of the longest substring.
 * @param str String to split
 * @param longestLine Pointer to variable to store
 * @return The full length of the buffer
 */
static int splitLinesIntoSubstrings(char* str, int* longestLine) {
  *longestLine= 0;

  int i= 0, c= 0;
  while( str[i] != '\0' ) {
    if( str[i] == '\n' ) {
      *longestLine= imax( *longestLine, c );

      str[i]= '\0';
      c= 0;
    }

    i++;
    c++;
  }

  return i;
}

/**
 * @brief Parses a float from a string with 'strtof'
 * @param str String to parse
 * @param num Pointer to variable to store the parsed float
 * @return Pointer to the end of the parsed float in the string.
 *         Null on error.
 */
static char* parseFloat(char* str, float* num) {
  errno= 0;

  char* errorChar;
  *num= strtof(str, &errorChar);

  // Parsing error
  if( errno != 0 ) {
    printWithErrno("Bad strof");
    return NULL;
  }

  // Did not find any digits
  if( errorChar == str ) {
    return NULL;
  }

  return errorChar;
}

/**
 * @brief Reads a line from a file stream into a specified buffer.
 *        Fails if the line is too long for the buffer.
 * @param file File stream to read from
 * @param buffer Buffer to write the read line to
 * @param size Size of the buffer
 * @return <0 on error, 0 on success, >1 if the file was closed
 */
static int readLineFromFile(FILE* file, char* buffer, int size) {
  for( int i= 0; i < size; i++ ) {
    errno= 0;
    int c= fgetc(file);

    if( c == EOF ) {
      if( errno != 0 ) {
        printWithErrno("Bad fgetc");
        return -1;
      }

      buffer[i]= '\0';
      return 1;
    }

    if( c == '\n' ) {
      buffer[i]= '\0';
      return 0;
    }

    buffer[i]= c;
  }

  buffer[size]= '\0';

  printError2("The line is too long. \nIt cannot represent two valid single precision floating point numbers or the amount of seperatig whitespace is unreasonable. Data", buffer );
  return -1;
}

/**
 * @brief Frees and resets a point array
 * @param array Array to free
 */
static void freePointArray(Point2DArray* array) {
  if( array->points ) {
    free( array->points );
    array->points= NULL;
    array->length= array->capacity= 0;
  }
}

/**
 * @brief Appends a point to a point array. Resizes the array if
 *        more space is required.
 * @param array Array to append to
 * @param point Point to append to the array
 * @return <0 on allocation error
 */
static int appendPointToArray(Point2DArray* array, Point2D point) {
  if( array->points == NULL ) {
    array->points= malloc(POINT_ARRAY_INIT_CAP* sizeof(Point2D));
    if( array->points == NULL ) {
      return -1;
    }

    array->length= 0;
    array->capacity= POINT_ARRAY_INIT_CAP;
  }

  if( array->length == array->capacity ) {
    int newCapacity= array->capacity*2;
    Point2D* newArray= realloc(array->points, newCapacity* sizeof(Point2D));
    if( newArray == NULL ) {
      freePointArray( array );
      return -1;
    }

    array->points= newArray;
    array->capacity= newCapacity;
  }

  array->points[array->length]= point;
  array->length++;
  return 0;
}

/**
 * @brief Parse a single point from a file stream
 * @param file File stream to read from
 * @param point Pointer to variable to store parsed point
 * @param printErrorOnEmptyLine Enable error message if the parsing
 *        failes due to an empty line
 * @return <0 on error, 0 on success, >1 if the file was closed
 */
static int parsePointFromFile(FILE* file, Point2D* point, bool printErrorOnEmptyLine ) {
  char lineBuffer[MAX_INPUT_LINE_LEN];
  int ret= readLineFromFile( file, lineBuffer, MAX_INPUT_LINE_LEN );
  if( ret < 0 ) {
    return -1;
  }

  if( *lineBuffer == '\0' ) {
    if( ret > 0 ) {
      return 1;
    }

    if( printErrorOnEmptyLine ) {
      printError2("Invalid input data: Expected x-coordinate", "<empty-line>");
    }

    return -1;
  }

  float x, y;
  char* str;

  if( (str= parseFloat( lineBuffer, &x )) == NULL ) {
    printError2("Invalid input data: Expected x-coordinate", lineBuffer);
    return -1;
  }

  if( (str= parseFloat( str, &y )) == NULL ) {
    printError2("Invalid input data: Expected y-coordinate", lineBuffer);
    return -1;
  }

  if( *str != '\0' ) {
    printError2("Invalid input data: Expected end of string", lineBuffer);
    return -1;
  }

  point->x= x;
  point->y= y;

  return ret;
}

/**
 * @brief Parse inputed points from stdin and store them in a point array
 * @param array Array to store the parsed points
 * @return <0 on error
 */
static int parseInput(Point2DArray* array) {

  memset(array, 0, sizeof(Point2DArray));

  while( true ) {
    Point2D point;
    int ret= parsePointFromFile(stdin, &point, true);
    if( ret < 0 ) {
      freePointArray( array );
      return -1;
    }

    if( ret > 0 ) {
      return 0;
    }

    if( appendPointToArray( array, point ) < 0 ) {
      freePointArray( array );
      return -1;
    }

  }
}

/**
 * @brief Swaps the data of two points
 * @param a Point to swap data with
 * @param b Point to swap data with
 */
static void swapPoint2D(Point2D* a, Point2D* b) {
  Point2D x= *a;
  *a= *b;
  *b= x;
}

/**
 * @brief Calculates the average of all x components of all points
 *        in an array
 * @param array Array of points
 * @return Averaged x components
 */
static float calcAverageXOfPointArray(Point2DArray* array) {
  float sum= 0;
  for( int i= 0; i!= array->length; i++ ) {
    sum+= array->points[i].x;
  }

  return sum / array->length;
}

/**
 * @brief Moves all points with a x component less or equal to the
 *        specified limit to the lower half of the array.
 * @param array Array to split into two halfs
 * @param xlimit Maximum value to be stored in the lower half
 * @return Number of elements in the lower half of the array
 */
static int splitPointArrayWithXLimit(Point2DArray* array, float xlimit) {
  // Count how many elements are supposed to be in the lower half
  const int len= array->length;
  int lessThanOrEqualLimitCount= 0;
  for( int i= 0; i!= len; i++ ) {
    if( array->points[i].x <= xlimit ) {
      lessThanOrEqualLimitCount++;
    }
  }

  // Iterate through the lower half and remove any elements that are too large
  int j= lessThanOrEqualLimitCount;
  for( int i= 0; i!= lessThanOrEqualLimitCount; i++ ) {
    if( array->points[i].x > xlimit ) {

      // Search for item to swap with in the upper half
      while( array->points[j].x > xlimit ) {
        j++;
        // This should never happen (fingers crossed)
        if( j >= len ) {
          printError("Internal error: Could not find item with smaller x than limit");
          exit(EXIT_FAILURE);
        }
      }

      swapPoint2D( &(array->points[i]), &(array->points[j]) );
    }
  }

  return lessThanOrEqualLimitCount;
}

/**
 * @brief Calculates the distance between two points inside a pair
 * @param pair Pair of points, of which the distance is calculated
 * @return Distance
 */
static float calcPairDistance( Point2DPair* pair ) {
  float xdiff= pair->a.x- pair->b.x;
  float ydiff= pair->a.y- pair->b.y;
  return sqrt( xdiff* xdiff + ydiff* ydiff );
}

/**
 * @brief Calculates the pair with the smallest distance in the split
 *        array. Compares all points in the lower half with all points
 *        in the upper half and stores the pair with the smallest
 *        distance.
 * @param array Split array of points
 * @param upperIdx Number of elements int the lower half
 * @return Pair of points with the smallest distance
 */
static Point2DPair calcClosestPairInSplitArray( Point2DArray* array, int upperIdx ) {
  Point2DPair pair, bestPair;
  float shortestDist= +INFINITY;

  for( int i= 0; i!= upperIdx; i++ ) {
    pair.a= array->points[i];

    for( int j= upperIdx; j!= array->length; j++ ) {
      pair.b= array->points[j];

      float dist= calcPairDistance(&pair);
      if( dist < shortestDist ) {
        bestPair= pair;
        shortestDist= dist;
      }
    }
  }

  return bestPair;
}

/**
 * @brief Duplicates one end of the pipe as a new file descriptor.
 *        Both ends of the pipe are closed.
 * @param fds Pipe with two file descriptors
 * @param idx End of the pipe to be duplicated
 * @param newFd New file descriptor id to be duplicated as
 * @return <0 on error
 */
static int dupAndClosePipeEnds(int fds[2], int idx, int newFd) {

  int a = idx == 0 ? 0 : 1; // Index of the fd to keep and dup
  int b = idx == 0 ? 1 : 0; // Index of the fd to close

  if( close(fds[b]) < 0 ) {
    return -1;
  }
  if( dup2(fds[a], newFd) < 0 ) {
    return -1;
  }
  if( close(fds[a]) < 0 ) {
    return -1;
  }

  return 0;
}

/**
 * @brief Forks the process into a child and creates two pipes to
 *        it for it's stdin and stdout. Then executes the same binary
 *        again.
 * @param child Child handle to be initialized with the newly forked
 *        child
 * @return <0 on error
 */
static int recursiveFork(ChildProcess* child) {
  fflush(stdout);
  fflush(stderr);

  int stdinFds[2];
  if( pipe(stdinFds) < 0 ) {
    printWithErrno("Could not create stdin pipe");
    return -1;
  }

  int stdoutFds[2];
  if( pipe(stdoutFds) < 0 ) {
    printWithErrno("Could not create stdin pipe");
    close(stdinFds[0]);
    close(stdinFds[1]);
    return -1;
  }

  pid_t pid= fork();
  if( pid < 0 ) {
    printWithErrno("Could not fork");
    close(stdinFds[0]);
    close(stdinFds[1]);
    close(stdoutFds[0]);
    close(stdoutFds[1]);
    return -1;
  }

  // Now running as child process
  if( pid == 0 ) {

    if( dupAndClosePipeEnds(stdinFds, 0, STDIN_FILENO) < 0 ) {
      printWithErrno("Could not dup stdinFd");
      exit(EXIT_FAILURE);
    }

    if( dupAndClosePipeEnds(stdoutFds, 1, STDOUT_FILENO) < 0 ) {
      printWithErrno("Could not dup stdout");
      exit(EXIT_FAILURE);
    }

    execlp(binaryPath, binaryPath, NULL);
    printWithErrno("Could not exec");
    return -1;
  }

  // Still running as parent
  if( (close(stdinFds[0]) < 0) || (close(stdoutFds[1]) < 0) ) {
    printWithErrno("Could not close pipe end");
    return -1;
  }

  child->pid= pid;
  child->stdinFd= stdinFds[1];
  child->stdoutFd= stdoutFds[0];
  return 0;
}

/**
 * @brief Closes pipes of both children if they are still open
 * @param children Array of two children, whose pipes should be closed
 */
static void closeChildPipes(ChildProcess children[2]) {
  for(int i= 0; i!= 2; i++) {
    if(children[i].stdinFd != -1) {
      close(children[i].stdinFd);
      children[i].stdinFd= -1;
    }
    if( children[i].stdoutFd != -1 ) {
      close(children[i].stdoutFd);
      children[i].stdoutFd= -1;
    }
  }
}

/**
 * @brief Wait for both children to exit successfully
 * @param children Child processes to wait for
 * @return <0 on error
 */
static int waitForChildrenToExit(ChildProcess children[2]) {
  int waitingCount= 2;
  int status;

  bool hasError= false;

  while( waitingCount > 0 ) {
    pid_t pid= wait(&status);

    if( pid < 0 ) {
      if( errno == EINTR ) {
        continue;
      }
      printWithErrno("Could not wait for child process");
      return -1;
    }

    if( (pid != children[0].pid) && (pid != children[1].pid) ) {
      printError("Unknown child process exited");
      continue;
    }

    if( WEXITSTATUS(status) != EXIT_SUCCESS ) {
      printError("Child process returned with error code");
      hasError= true;
    }

    waitingCount--;
  }

  closeChildPipes(children);
  return hasError ? -1 : 0;
}

/**
 * @brief Send a slice of points from an array to a child process
 * @param child Child process to send data to
 * @param array Array of points to be sent
 * @param from Index of the first point to be sent
 * @param to Index of the first point not to be sent
 * @return <0 on error
 */
static int sendPointsToChildProcess(ChildProcess* child, Point2DArray* array, int from, int to) {
  for( int i= from; i!= to; i++ ) {
    if( printPoint(child->stdinFd, array->points[i]) < 0 ) {
      return -1;
    }
  }

  return 0;
}

/**
 * @brief Tries to receives two points from a child process and stores
 *        them in a point pair. If not a single point can be read, the
 *        function fails silently. If one point is read, a second one
 *        is expected.
 * @param pipeFile File stream to read from
 * @param child Child process who owns the file stream
 * @param pair Pointer to a variable to store the parsed point pair
 * @return <0 on error, 0 on no pair parsed, >0 on pair parsed
 */
static int receivePointPairFromFile(FILE* pipeFile, ChildProcess* child, Point2DPair* pair) {

  int hasPair= 0;

  // This may fail
  int ret= parsePointFromFile( pipeFile, &(pair->a), false );

  // First read succeeded -> Expect another point to read
  if( ret == 0 ) {
    ret= parsePointFromFile( pipeFile, &(pair->b), true );
    if( ret < 0 ) {
      fclose( pipeFile );
      child->stdoutFd= -1;
      return -1;
    }

    if( ret > 0 ) {
      printError("Child has to send two points");
      fclose( pipeFile );
      child->stdoutFd= -1;
      return -1;
    }

    hasPair= 1;
  }

  return hasPair;
}

/**
 * @brief Reads as many charactes as possible from a file stream into
 *        a dynamically allocated string.
 * @param pipeFile File stream to read from
 * @param str Pointer to a string variable to store the allocated buffer
 * @return <0 on error
 */
static int readFileToDynString(FILE* pipeFile, char** str) {
  if( ferror(pipeFile) != 0 ) {
    printError("Could not read from file");
    fclose(pipeFile);
    return -1;
  }

  *str= NULL;

  int len= 0, capacity= 0;
  char buffer[127];
  int readLen= 0;
  while( (readLen= fread(buffer, 1, 127, pipeFile)) > 0 ) {

    // Resize the string to make space for read characters
    if( capacity < len + readLen ) {
      capacity= capacity == 0 ? 128 : capacity*2;

      char *nstr= realloc(*str, sizeof(char)* capacity);
      if( nstr == NULL ) {
        free(*str);
        *str= NULL;

        printWithErrno("Could not allocate dyn string");
        fclose(pipeFile);
        return -1;
      }

      *str= nstr;
    }

    // Copy read buffer to the end of the string
    memcpy(*str+ len, buffer, readLen);
    len+= readLen;
    (*str)[len]= '\0';
  }

  if( ferror(pipeFile) != 0 ) {
    printError("Could not read from file");
    free(*str);
    fclose(pipeFile);
    return -1;
  }

  return 0;
}

/**
 * @brief Tries to receive a point pair and a process-tree from a
 *        child process. If a point pair was received that is better
 *        than the current one, it is updated with the better one.
 * @param child Child process to receive data from
 * @param bestPair Pointer to the current best pair, might be updated
 *        with new data
 * @param tree Pointer to a string variable where an allocated tree
 *        can be stored
 * @return <0 on error
 */
static int receiveDataFromChildProcessAndUpdateBestPointPair(ChildProcess* child, Point2DPair* bestPair, char** tree) {
  Point2DPair received;

  FILE* pipeFile= fdopen(child->stdoutFd, "r");
  if( pipeFile == NULL ) {
    printWithErrno("Could not open child pipe as file");
    return -1;
  }

  // Try to read two points as a pair from the pipe
  int ret= receivePointPairFromFile(pipeFile, child, &received);
  if( ret < 0 ) {
    return -1;
  }

  // Read as many bytes from the pipe and store them in a dynamic string
  if( readFileToDynString(pipeFile, tree) < 0 ) {
    return -1;
  }

  if( fclose( pipeFile ) < 0 ) {
    printWithErrno("Could not close child read pipe");
    return -1;
  }
  child->stdoutFd= -1;

  // Update the currently best pair, if a pair was received and it's distance is shorter
  if( ret > 0 ) {
    printDebug("Child returned a pair: (%f, %f) (%f, %f)", received.a.x, received.a.y, received.b.x, received.b.y);

    if( calcPairDistance(&received) < calcPairDistance(bestPair) ) {
      *bestPair= received;
    }
  } else {
    printDebug("Child returned no points");
  }

  return 0;
}

/**
 * @brief Prints a process tree to stdout, by combining two received
 *        sub trees and adding the own data configuration as parent
 *        on top.
 * @param treeA Left subtree to print
 * @param treeB Right subtree to print
 * @param points Array of points received as input
 */
static void printProcessTree( char* treeA, char* treeB, Point2DArray* points ) {

  printDebug("BEFORE PRINTING:");
  printDebug("TREE A:\n%s\n\n", treeA);
  printDebug("TREE B:\n%s\n\n", treeB);

  treeA= jumpEmptyLines(treeA);
  treeB= jumpEmptyLines(treeB);

  int longestA= 0, longestB= 0;
  int fullLenA= splitLinesIntoSubstrings(treeA, &longestA);
  int fullLenB= splitLinesIntoSubstrings(treeB, &longestB);

  // Count the number of negative signs that will be printed
  int negativesCount= 0;
  for( int i= 0; i!= points->length; i++ ) {
    negativesCount+= points->points[i].x < 0 ? 1 : 0;
    negativesCount+= points->points[i].y < 0 ? 1 : 0;
  }

  // Print own line
  int minSpacingAB= 6;
  int ownLen= points->length * 12+ 10+ negativesCount;
  int maxLineLen= imax(ownLen, longestA+ longestB+ minSpacingAB);
  int ownPadding= imax(0, (maxLineLen- ownLen) / 2 );
  int paddingAB= imax(0, (maxLineLen- longestA- longestB- minSpacingAB) / 2 );

  putWhitspace(ownPadding);

  printf("MIN-DIST(");
  for( int i= 0; i!= points->length; i++ ) {
    printf("(%.2f, %.2f)", points->points[i].x, points->points[i].y);
  }
  printf(")\n");

  // Print branch lines
  putWhitspace(paddingAB+ longestA/2);
  fputc('/', stdout);

  putWhitspace(longestA/2+ minSpacingAB+ longestB/2 -1);
  printf("\\\n");

  // Print left and right tree interleaved
  char* curA= treeA, *curB= treeB;
  char* endA= treeA+ fullLenA, *endB= treeB+ fullLenB;
  while( curA< endA || curB< endB ) {

    int lenB= curB < endB ? strlen(curB) : 0;
    int lenA= curA < endA ? strlen(curA) : 0;

    putWhitspace(paddingAB);

    if( curA < endA ) {
      printf("%s", curA);
    }

    int spacingAB= minSpacingAB+ (longestA- lenA);
    putWhitspace(spacingAB);

    if( curB < endB ) {
      printf("%s", curB);
    }

    fputc('\n', stdout);

    curA+= lenA+ 1;
    curB+= lenB +1;
  }
}



/**
 * @brief Reads points consisiting of two floating point numbers as
 *        strings from stdin and finds the closest pair, which is
 *        printed to stdout.
 * @detail The points are read from stdin line by line and converted
 *         into a dynamic array of point objects. If there are less
 *         than three points the program returns immediately. Else
 *         the average x component of all points is caclulated and
 *         the array split into two halfs of ones with a lesser and
 *         ones with a greater x component than the average. Each
 *         half is sent to a forked child process running the same
 *         algorithm. The results of both children is collected and
 *         compared to one that is calculated by the parent process,
 *         by comparing each point in the lower half with every point
 *         in the upper one. The best of these three pairs is returned.
 * @param argc Count of CLI arguments. Expected to be 1
 * @param argv Array of string arguments provided by the CLI
 * @return EXIT_SUCCESS on success else EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
  binaryPath= argv[0];

  if(argc > 1) {
    printError("Unexpected input arguments. Usage: cpair");
    return EXIT_FAILURE;
  }

  Point2DArray points;
  if( parseInput( &points ) < 0 ) {
    return EXIT_FAILURE;
  }

  printDebug("Points:");
  for( int i= 0; i!= points.length; i++ ) {
    printDebug("- %02d) Point (%f, %f)", i, points.points[i].x, points.points[i].y);
  }

  // Immediately return with less than three points
  if( points.length < 2 ) {
    if( points.length == 1 ) {
      printf("\n\n(%.2f, %.2f)\n", points.points[0].x, points.points[0].y );
    }
    freePointArray( &points );
    return 0;
  }

  if( points.length == 2 ) {
    fflush(stdout);
    printPoint(STDOUT_FILENO, points.points[0]);
    printPoint(STDOUT_FILENO, points.points[1]);


    printf("\n\nMIN-DIST((%.2f, %.2f)(%.2f, %.2f))\n", points.points[0].x, points.points[0].y, points.points[1].x, points.points[1].y );
    freePointArray( &points );
    return EXIT_SUCCESS;
  }

  // Calc average and split array
  float avgX= calcAverageXOfPointArray( &points );
  int upperIdx= splitPointArrayWithXLimit( &points, avgX );


  if( (upperIdx == 0) || (upperIdx == points.length) ) {
    printError("This dataset will cause infinit recursion as all points are lower or greater than the average");
    freePointArray(&points);
    return EXIT_FAILURE;
  }


  printDebug("\nLower:");

  for( int i= 0; i!= upperIdx; i++ ) {
    printDebug("- %02d) Point (%f, %f)", i, points.points[i].x, points.points[i].y);
  }

  printDebug("Upper:");

  for( int i= upperIdx; i!= points.length; i++ ) {
    printDebug("- %02d) Point (%f, %f)", i, points.points[i].x, points.points[i].y);
  }

  // Fork children and send data
  ChildProcess children[2]= {{.stdinFd= -1, .stdoutFd= -1}, {.stdinFd= -1, .stdoutFd= -1}};
  if( recursiveFork( &(children[0]) ) < 0 ) {
    freePointArray( &points );
    return EXIT_FAILURE;
  }

  if( recursiveFork( &(children[1]) ) < 0 ) {
    freePointArray( &points );
    closeChildPipes( children );
    return EXIT_FAILURE;
  }

  if( sendPointsToChildProcess( &(children[0]), &points, 0, upperIdx ) < 0 ) {
    printWithErrno("Could not send points to child 0");
    freePointArray( &points );
    waitForChildrenToExit( children );
    return EXIT_FAILURE;
  }


  if( sendPointsToChildProcess( &(children[1]), &points, upperIdx, points.length ) < 0 ) {
    printWithErrno("Could not send points to child 1");
    freePointArray( &points );
    waitForChildrenToExit( children );
    return EXIT_FAILURE;
  }

  // Send EOF
  close(children[0].stdinFd);
  close(children[1].stdinFd);

  children[0].stdinFd= -1;
  children[1].stdinFd= -1;

  Point2DPair currentBestPair= calcClosestPairInSplitArray( &points, upperIdx );

  printDebug("Found best point pair (%f, %f) (%f, %f)", currentBestPair.a.x, currentBestPair.a.y, currentBestPair.b.x, currentBestPair.b.y);

  // Receive solutions from children
  char *treeA;
  if( receiveDataFromChildProcessAndUpdateBestPointPair( &(children[0]), &currentBestPair, &treeA ) < 0 ) {
    freePointArray( &points );
    return EXIT_FAILURE;
  }

  char *treeB;
  if( receiveDataFromChildProcessAndUpdateBestPointPair( &(children[1]), &currentBestPair, &treeB ) < 0 ) {
    freePointArray( &points );
    return EXIT_FAILURE;
  }

  if( treeA == NULL || treeB == NULL ) {
    printError("Child did not return a tree");
    free(treeA);
    free(treeB);
    return EXIT_FAILURE;
  }

  if( waitForChildrenToExit( children ) < 0 ) {
    freePointArray( &points );
    return EXIT_FAILURE;
  }

  // Print the resulting point pair
  printPoint( STDOUT_FILENO, currentBestPair.a );
  printPoint( STDOUT_FILENO, currentBestPair.b );

  printProcessTree( treeA, treeB, &points );
  free(treeA);
  free(treeB);

  freePointArray( &points );

  return EXIT_SUCCESS;
}
