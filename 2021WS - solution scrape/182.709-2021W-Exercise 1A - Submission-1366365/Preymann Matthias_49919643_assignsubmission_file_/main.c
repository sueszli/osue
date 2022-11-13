/**
 * @file main.c
 * @brief Main file of 'mycompress'
 * @author Matthias Preymann (12020638)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

/**
 * @brief Stores how many bytes were read/written by a function
 */
typedef struct {
  /** Number of bytes read **/
  int read;
  /** Number of bytes written **/
  int written;
} ByteCounter;

/** Globally stores the path to this binary from argv **/
static const char* binaryPath;

/**
 * @brief Prints the usage message to stderr and exits
 * @param exitCode Code to exit the program with
 */
static void printUsageAndExit(int exitCode) {
  fprintf(stderr, "Usage: %s [-o outfile] [infile...]\n", binaryPath);
  exit(exitCode);
}

/**
 * @brief Prints an error message to stderr including the path of
 *        the binary and the current string representation of errno.
 * @param msg Custom string message to print
 */
static void printWithErrnoAndExit(const char* msg) {
  fprintf(stderr, "%s: %s: %s\n", binaryPath, msg, strerror(errno));
  exit(EXIT_FAILURE);
}

/**
 * @brief Parses the command line input arguments with 'getopt'. Leaves
 *        'optind' at the index of the first positional argument.
 * @param argc Number of provided arguments
 * @param argv Array of string arguments
 * @return Path of the output file or null
 */
static const char* parseArguments(const int argc, char *argv[]) {
  // Save the call path of the binary globally
  binaryPath= argv[0];

  const char *outFileName= NULL;

  int c;
  while ( (c = getopt(argc, argv, "o:")) != -1 ){
    switch ( c ) {
      case 'o':
        outFileName= optarg;
        break;

      case '?': /* invalid option */
      default:
        printUsageAndExit(EXIT_FAILURE);
        break;
    }
  }

  return outFileName;
}

/**
 * @brief Reads characters form an input file and does simple run-length
 *        encoding. Writes the "compressed" data to another file and
 *        counts bytes read and written.
 * @param outFile File stream to write compressed data to
 * @param inFile File stream to read uncompressed data from
 * @param bytes Accumulator to store how many bytes were read and written
 * @return Negative number on error, else 0
 */
static int compressFile(FILE* outFile, FILE* inFile, ByteCounter* bytes) {
  int curChar;
  int curCharCounter= 0;

  while( true ) {
    int c= fgetc(inFile);
    if( c == EOF ) {
      // Print last group if it contains any chars
      if( curCharCounter > 0 ) {
        int b= fprintf(outFile, "%c%d", curChar, curCharCounter);
        if( b < 0 ) {
          return b;
        }

        bytes->written+= b;
      }

      return 0;
    }

    bytes->read++;

    // Start first group
    if(curCharCounter == 0) {
      curChar= c;
      curCharCounter= 1;
      continue;
    }

    // Add char to the current group
    if(c == curChar) {
      curCharCounter++;
      continue;
    }

    // Print group and start new one
    int b= fprintf(outFile, "%c%d", curChar, curCharCounter);
    if( b < 0 ) {
      return b;
    }

    bytes->written += b;
    curChar= c;
    curCharCounter= 1;
  }
}

/**
 * @brief Calculates compression ratio and prints statistics to stderr
 * @param bytes Number of bytes read and written
 */
static void printStats(ByteCounter* bytes) {
  fprintf(stderr, "\n\n");
  fprintf(stderr, "Read:    %d characters\n", bytes->read);
  fprintf(stderr, "Written: %d characters\n", bytes->written);
  fprintf(stderr, "Compression ratio: %.1f%%\n", (100.* bytes->written) / bytes->read);
}

/**
 * @brief Entry point of the program
 * @param argc Number of provided arguments
 * @param argv Array of string arguments
 */
int main(int argc, char *argv[]) {
  const char* outFileName= parseArguments(argc, argv);
  FILE* outFile;

  // Open/create file or use stdout
  if(outFileName == NULL) {
    outFile= stdout;
  } else {
    outFile= fopen(outFileName, "w");
  }

  if(outFile == NULL) {
    printWithErrnoAndExit("Could not open output file");
  }

  ByteCounter stats;
  stats.read= 0;
  stats.written= 0;

  // No input file paths provided -> Read from stdin
  if( optind == argc ) {
    // Read from stdin
    if( compressFile(outFile, stdin, &stats) < 0 ) {
      fclose( outFile );
      printWithErrnoAndExit("Could not write out file");
    }
  }

  // For all input files provided as positional args
  for( int i = optind; i!= argc; i++ ) {
    FILE* inFile= fopen(argv[i], "r");
    if( inFile == NULL ) {
      fclose( outFile );
      printWithErrnoAndExit("Could not open input file");
    }

    if( compressFile(outFile, inFile, &stats) < 0 ) {
      fclose( outFile );
      fclose( inFile );
      printWithErrnoAndExit("Could not write out file");
    }

    fclose(inFile);
  }

  fclose(outFile);

  printStats( &stats );

  return EXIT_SUCCESS;
}
