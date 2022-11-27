/**
 * @file myexpand.c
 * @author Malik Tuwebti <e11927172@student.tuwien.ac.at>
 * @date 07/11/21
 *
 * @brief myexpand tab expansion
 *
 *myexpand [-t tabstop] [-o outfile] [file...] 
 * the myexpand program expands tabs to spaces aligned to boundaries determined by the -t option, 
 *the output file can be specified with the -o option (or defaults to stdout) . 
 *Multiple input files may be specified as arguments.
 **/


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>


/**
 * tabreplace
 * @brief expand tabs to spaces for a given input file, output file, and tabstop
 * @param inputFile : file containing text to be expanded
 * @param outpuFile : file to expand text to
 * @param tabstop : boundary to align tab expansion to
 **/
static void tabreplace(FILE *inputFile, FILE *outputFile, long tabstop){
  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  int linePos; // tracks the current position in the outputed line
  int nextPos; // the position up to which tabs should be expanded to

  while ((nread = getline(&line, &len, inputFile)) != -1){
    // standin
    linePos = 0;
    for (int i = 0 ; i < nread ; i++){
      if (line[i] == '\t'){
	nextPos = tabstop * ((linePos / tabstop) + 1);
	while (linePos < nextPos){
	  fputs(" ", outputFile);
	  linePos ++;
	}
      }
      else {
	fprintf(outputFile, "%c" , line[i]);
	linePos++ ;
      }
    }
  }
  
  free(line);
}

static void printError(char *message, char *name){
  fprintf(stderr, "%s\n", message);
  fprintf(stderr, "Usage : %s [-t tabstop] [-o outfile] [file...]\n", name);
}


/**
 * @brief The main function deals with setting up IO and error checking, it offloads the actual expansion to the tabreplace function
 **/

int
main(int argc, char *argv[])
{
  int opt;
  long tabstop = 8;
  FILE *outputFile = stdout;
  FILE *inputFile ;

  //argument checking code goes here
  
  while (( opt = getopt(argc, argv, "t:o:")) != -1) {
    switch (opt) {
    case 't' :
      errno = 0;
      tabstop = strtol(optarg, NULL, 0);
      if (errno != 0){
	perror("strtol");
	exit(EXIT_FAILURE);
      }
      if (tabstop == 0){
	printError(" tabstop must be greater or equal to 1", argv[0]);
	exit(EXIT_FAILURE);
      }
      break;
    case 'o' :
      outputFile = fopen(optarg, "w");
      if (outputFile == NULL){
	perror("fopen");
	printError(" argument to -o is invalid", argv[0]);
	exit(EXIT_FAILURE);	
      }
      break;
    default:
      // getopt automatically handles erroneous options
      assert(0);
    }
  }

  
  if (optind == argc)  // if no input files specified default to stdin
    tabreplace(stdin, outputFile, tabstop);
  
  for (int i = optind ; i < argc ; i++){
    inputFile = fopen(argv[i],"r");
    if (inputFile == NULL){
      perror("fopen");
      printError("", argv[0]);
      exit(EXIT_FAILURE);
    }
    
    tabreplace(inputFile, outputFile, tabstop);
    fclose(inputFile);
  }
  

  fclose(outputFile);
  exit(EXIT_SUCCESS);
}


