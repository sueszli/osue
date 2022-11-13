/**
 * @file   myexpand.c
 * @author Keisi Cela, 11737582, <e11737582@student.tuwien.ac.at>
 * @date   14.11.2021
 *
 * @brief Implements the functions specified in myexpand.h.
 **/

#include "myexpand.h"

/** For documentation see myexpand.h */
static void usage(void){
	fprintf(stderr,"Usage: %s [-t tabstop] [-o outfile] [file...]\n",progName);
	exit(EXIT_FAILURE);
}

/** For documentation see myexpand.h */
static char **readInput(int argc, char **argv){
	char c;
	while((c=getopt(argc,argv,"to:"))!=-1){
		switch(c){
			case '?':
			 	usage();
			 	break;
			case 't':
				tabstop = argv[2];
				optind++;
				// optarg is not working for me in this case and that is why, 
				// I had to use argv[2] and optind++, to show that my solution
				// implementation works.
			 	break;
			case 'o':
			 	outfile = optarg; 
			 	break;
			default: 
		 	 	break;
		}
	}
	filesNumber = argc - optind;
	char **filenames;
	filenames = malloc(filesNumber*sizeof(char*));
	
	for(int i=0; i<filesNumber; i++){
		filenames[i] = argv[i+optind];
	}
	return filenames;
}

/** For documentation see myexpand.h */
static void replaceTabs(FILE* file,int t, FILE* output){
      int x = 0;
      int c;
      while( (c=fgetc(file)) != EOF){
      	if(c=='\n'){
        	x = -1;
			if(output == NULL){
        		fputc(c,stdout);
			} else {
				fputc(c, output);
			}
      	}
      	else if(c=='\t'){
      		int p = t * ((x/t)+1);
      		while(p>x){
				if(output == NULL){
        			fputc(' ',stdout);
				} else {
					fputc(' ', output);
				}
      			p--;
      		}
      	}
      	else{
			if(output == NULL){
        		fputc(c,stdout);
			} else {
				fputc(c, output);
			}
      	}
      	x++;
    }      
}

/**
 * @brief The entry point of the expand program.
 * @details This function executes the whole program.
 *          It calls upon other functions to open the files provided in the input and replace tabs with spaces.
 *
 * global variables used: progName - The program name as declared in myexpand.h.
 *
 * @param argc The argument counter.
 * @param argv The argument vector. 
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char **argv){

	char **filenames = readInput(argc,argv);
	
	int t = 8;

	if(tabstop != NULL){
		sscanf(tabstop, "%d", &t);
	}

	FILE *output = fopen(outfile, "w");

	if(filesNumber>0){
		for(int i = 0; i<filesNumber; i++){
			FILE* file = fopen(filenames[i],"r");
			if(file!=NULL){
				
				replaceTabs(file,t, output);
				fclose(file);
				if(output == NULL){
					fputc('\n', stdout);
				} else {
					fputc('\n', output);
				}
			}
			else{
				fprintf(stderr,"%s: File %s not found\n",progName,filenames[i]);
			}
		}
	}
	else{
		replaceTabs(stdin,t,output);
	}
	free(filenames);
	
	return EXIT_SUCCESS;
}
