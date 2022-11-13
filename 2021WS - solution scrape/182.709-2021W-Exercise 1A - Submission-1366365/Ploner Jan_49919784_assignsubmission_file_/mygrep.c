/**
 * This program can be used to find a keyword within a text file (.txt).
 * It will return the entire line which contains the keyword to the console or if
 * specified with -o to an external file.
 * With -i the search will be case-insensitive.
 * To work the program needs at least two arguments consisting of the keyword and a file (probably) containing the keyword.
 * It is recomended to use following comiler arguments:
 * gcc −std=c99 −pedantic −Wall −D_DEFAULT_SOURCE −D_BSD_SOURCE −D_SVID_SOURCE −D_POSIX_C_SOURCE=200809L −g −c mygreps.c
 * 
 * @file mygrep.c
 * @author Jan Ploner (12025954)
 * @brief Findes a keyword within a file and prints line containing the keyword.
 * @version 1.0
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

void usage(char *);

/**
 * @brief main function handling arguments, doing IO and calculating functionality
 * 
 * @param argc number of arguments
 * @param argv array with arguments
 * @return Exit_SUCCESS (int 0) or EXIT_FAILURE (int 1)
 */

int main(int argc, char **argv){

  ///@param MYPROG stores the program name
  char* MYPROG;
  MYPROG = argv[0];

  if (argc < 3){ //at least two additional arguments have to be given
    usage(MYPROG);
    return EXIT_FAILURE;
  }
  
  ///@param caseSensitive Variable to store if case-sensitivity is requested (1 equals case sensitive)
  int caseSensitive = 1;
  ///@param outFileName stores the name of the lokation where the results should be written (NULL if not set by user)
  char* outFileName;
  ///@param inFileName stores the name of the file in which the keyword should be searched
  char* inFileName[argc-2];
  ///@param keyword stores the keyword which should be searched
  char* keyword;

  outFileName = NULL;

  //Argument Handling ----

  int option; // for while loop
  while ((option = getopt(argc, argv, "io:")) != -1) { //loops trought all options given to main
    switch (option) {
      case 'i':
        caseSensitive = 0; //if set the program is case insensitive
        //printf("Case Insensitive\n");
        break;
      case 'o':
        outFileName = optarg; //sets the File to which the results should be written
        //printf("outfile set to: %s\n",optarg);
        break;
      case '?':
        //printf("unknown option: %c\n", optopt);
        usage(MYPROG);
        break;
      default:
        assert(0);
        //printf("unreacable code\n");
        usage(MYPROG);
        break;
    }
  }
  
  int counter = 0;
  ///@param files stores the number of files
  int files = ((argc-optind)-2);
  ///@param files2 stores the number of files
  const int files2 = files;

  //cycles trouth all remaining "non-option" arguments
  for(; optind < argc; optind++){
    if (counter == 0){ //first is the keyword
      keyword = argv[optind];
      //printf("Search for %s\n", keyword);
      counter++;
    }

    else {//others are the inputfiles
      inFileName[files] = argv[optind];
      //printf("Input file: %s\n", inFileName);
      counter++;
      files--;
    }
  }

  //IO----
  
  ///@param counter contains how ofthen the keyword was found
  counter = 0;

  ///@param in is the inputstream of IO
  ///@param out is the outputstream of IO
  FILE *in, *out;

  if (outFileName != NULL){//open output stream only if outFileName was set by the user
    if ((out = fopen(outFileName, "w")) == NULL){
      fprintf(stderr, "%s -> fopen output failed: %s\n", MYPROG, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  for (int j = 0; j <= files2; j++){//do the input for all files

  ///@param mom_counter to see if counter has changed
  int mom_counter = counter;

    if ((in = fopen(inFileName[j], "r")) == NULL){
      fprintf(stderr, "%s -> fopen input failed: %s\n", MYPROG, strerror(errno));
      exit(EXIT_FAILURE);
    }

    ///@param line stores each read line
    char* line = NULL;
    ///@param length stores the length of a read line
    ///@param read stores the return value of the function getline()
    size_t length = 0, read;

    while ((read = getline(&line, &length, in) != -1)){

      ///@param save saves the original line so that case insensitivity can be used
      char *save;
      save = strdup(line);
      ///@param keysave stores keyword to a save read and write memory location
      char *keysave;
      keysave = strdup(keyword);

      //code block converts the line and keyword to lowercase letters
      if (!caseSensitive){
        for(int i = 0; line[i]; i++){
          line[i] = tolower(line[i]);
        }
        for(int i = 0; keysave[i]; i++){
          keysave[i] = tolower(keysave[i]);
        }
      }

      ///@param ret returns pointer to where the keyword was found inside the line (NULL if not found)
      char *ret;
      ret = strstr(line, keysave); //searches if the keyword is contained inside a line

      if (ret){//if the keyword was contained in the line
        if (outFileName != NULL){ //write to a external file
          if (fputs(save, out) == EOF){
            fprintf(stderr, "%s -> fputs output failed: %s\n", MYPROG, strerror(errno));
            exit(EXIT_FAILURE);
          }
        }else{//writes to the console
          printf("%s", save);
        }
        counter++;
      }

      free(save);
      free(keysave);
      free(line);
      length = 0;

    }

    if (fclose(in) == -1) {
      fprintf(stderr, "%s -> closing input stream failed: %s\n", MYPROG, strerror(errno));
      exit(EXIT_FAILURE);
    }

    if (mom_counter != counter) //make new line afert each file
    {
      if (outFileName != NULL){
        if (fputs("\n", out) == EOF){
          fprintf(stderr, "%s -> fputs output failed: %s\n", MYPROG, strerror(errno));
          exit(EXIT_FAILURE);
        }
      }else{
        printf("\n");
      }
    }
  }

  //if the keyword was not contained print warning to the user
  if (counter == 0){
    printf("WARNING: The keyword was NOT contained in the file!!!");
  }

  if (outFileName != NULL) {
    if (fclose(out) == -1) {
      fprintf(stderr, "%s -> closing output stream failed: %s\n", MYPROG, strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  return(EXIT_SUCCESS);
}

/**
 * @brief prints usage-message to the console
 * 
 * @param MYPROG stores the program name
 * @return void
 */
void usage(char *MYPROG){
  printf("Usage:%s [-i] [-o outfile.txt] keyword file.txt\n", MYPROG);
}