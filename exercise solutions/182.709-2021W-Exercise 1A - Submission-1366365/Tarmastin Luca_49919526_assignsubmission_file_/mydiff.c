/*@author: Luca Tarmastin, 01633051
@brief: Compares 2 files and writes out the number of differences
@date: 22.10.2021
*/
#include <stdio.h>
#include "stdlib.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>


char *myprog;

/**@brief Calculates and returns the minimum of the two parameters
**/
size_t min(size_t num1, size_t num2);

/**@brief Prints usage message and exits with failure
  * Uses the global variable myprog for the program-name
  *@return void
**/
void usage(void) {
  fprintf(stderr,"Usage: %s [-i] [-o outfile] file1 file2\n", myprog);
  exit(EXIT_FAILURE);
}

/**@brief Opens the two specified input files, Gets line by line from files
  * Then compares character for character of a line
  * If they don't match, difference-counter is incremented
  * 
**/
int main (int argc, char **argv) {
  FILE *outfile = stdout;
  int case_insensitive = 0;
  int c;
  FILE *file1;
  FILE *file2;
  myprog = argv[0];

  char *line = NULL;
  size_t len = 0;
  ssize_t nread;
  char *line2 = NULL;
  size_t len2 = 0;
  ssize_t nread2;

  while ( (c = getopt(argc, argv, "io:")) != -1 ){
    switch ( c ) {
      case 'i': case_insensitive = 1;
        break;
      case 'o': if( (outfile = fopen(optarg, "w")) == NULL){
          fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
          exit(EXIT_FAILURE);
        }
        break;
      case '?': /* invalid option */
        usage();
      default: assert(0);
    }
  }

  if( (argc-optind) != 2){
    fclose(outfile);
    usage();
  }
  if( (file1 = fopen(argv[optind],"r")) == NULL){
    fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
    fclose(outfile);
    exit(EXIT_FAILURE);
  }
  if( (file2 = fopen(argv[optind+1],"r")) == NULL){
    fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
    fclose(outfile);
    fclose(file1);
    exit(EXIT_FAILURE);
  }

  int linenumber = 1;
  int differences = 0;
  while ((nread = getline(&line, &len, file1)) != -1) {

    int ret;
    if (line[nread - 1] == '\n'){
      line[nread - 1] = '\0';
      nread = nread-2;
    }

    if((nread2 = getline(&line2, &len2, file2)) != -1){

      if (line2[nread2 - 1] == '\n'){
        line2[nread2 - 1] = '\0';
        nread2 = nread2-2;
      }

      if(case_insensitive){
        ret = strncasecmp(line, line2, min(nread, nread2));
      }
      else {
        ret = strncmp(line, line2, min(nread, nread2));
      }

      if(ret != 0){
        for(int i = 0; i < (min(nread, nread2)); ++i){
          if(case_insensitive){
            if(tolower(line[i])!=tolower(line2[i])){
              differences++;
            }
          }else {
            if(line[i]!=line2[i]){
              differences++;
            }
          }
        }
        if(differences>0){
          fprintf(outfile, "Line: %d, characters: %d\n", linenumber, differences);
        }
      }
      ++linenumber;
      differences = 0;
    }
  }

  free(line);
  free(line2);
  fclose(outfile);
  fclose(file1);
  fclose(file2);
  exit(EXIT_SUCCESS);
}

size_t min(size_t num1, size_t num2)
{
    return (num1 > num2 ) ? num2 : num1;
}
