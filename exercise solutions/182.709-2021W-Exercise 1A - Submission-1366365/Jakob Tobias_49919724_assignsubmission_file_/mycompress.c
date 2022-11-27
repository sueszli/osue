#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[]){
  	char* outputname = NULL;
  	char* inputname = NULL;
  	int c;
  	while ((c = getopt (argc, argv, "o:")) != -1)
    switch (c){
      case 'o':
        outputname = optarg;
        break;
      case '?':
        if (optopt == 'o'){
        	fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        	exit(EXIT_FAILURE);
		}
          
        else
          fprintf (stderr,
                   "Unknown option character `%c'.\n",
                   optopt);
        exit(EXIT_FAILURE);
      default:
        assert(0);
      }
      
  	int index;
  	for (index = optind; index < argc; index++){
  		if(inputname == NULL){
  			inputname = argv[index];
	  	} else {
	  		fprintf(stderr,"too many arguments");
	  		exit(EXIT_FAILURE);
	  	}
  	}
  	
  	FILE *inputf;
  	if(inputname == NULL){
  		inputf = stdin;
	} else {
		inputf = fopen(inputname, "r");
  		if (inputf == NULL) {
    		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
    		exit(EXIT_FAILURE);
  		}
	}
	
	FILE *outputf;
  	if(outputname == NULL){
  		outputf = stdout;
	} else {
		outputf = fopen(outputname, "r");
  		if (inputf == NULL) {
    		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
    		exit(EXIT_FAILURE);
  		}
	}

  	char currentChar = fgetc(inputf);
  	fprintf(outputf,"%c",currentChar);
  	char tempChar;
  	int count = 1;
  	int readcount = 0;
  	int wrotecount = 0;
  	while(1){
  		tempChar = fgetc(inputf);
  		readcount++;
		if(currentChar == tempChar){
			count++;
		}	else {
			fprintf(outputf,"%d",count);
			fprintf(outputf,"%c",tempChar);
			wrotecount = wrotecount + 2;
			currentChar = tempChar;
			count = 1;
		}
		if(feof(inputf)){
			break;
		}	
	}
	double ratio = (1.0*wrotecount)/(1.0*readcount)*100;
	fprintf(stderr,"Read:        %d Characters\n",readcount);
	fprintf(stderr,"Written:     %d Characters\n",wrotecount);
	fprintf(stderr,"Compression Ratio: %.1f%%\n",ratio);
	return 0;
}
