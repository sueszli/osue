/**
 * @file mycompress.c
 * @author Lucia Lechner <e0701149@student.tuwien.ac.at>
 * @date 26.10.2021
 *
 * @brief a programm, which reads and compresses input files with a simple algorithm
 *
 * SYNOPSIS
 *   mycompress [-o outfile] [file...]
 *       -o      specify file for the output file
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PROGNAME "mycompress"

/**
 * @brief print usage message and terminate program with EXIT_FAILURE
 */
void usage(void) {
	fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", PROGNAME);
	exit(EXIT_FAILURE);
}

/**
 * @brief Print error message and terminate program with EXIT_FAILURE
 * @param msg Message to print to stderr
 */
void exit_error(char* msg) {
	fprintf(stderr, "%s: %s\n", PROGNAME, msg);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv){

	char *outfile = NULL; // char pointer to outfile argv
	FILE *fp_outfile = NULL;

	/* parse options */
	int opt;
	while ( (opt = getopt(argc, argv, "o:")) != -1 ) {
		switch(opt){
			case 'o':
				outfile=optarg;
				break;
			case '?':
				usage();
				break;
			default:
				usage();
		}
	}

	/* if outfile was specified, open it for writing */
	if (outfile != NULL){
		fp_outfile = fopen(outfile,"w");
		if (fp_outfile == NULL){
			exit_error("Unable to open outputfile.\n");
		}
	}

	/* loop over remaining non-option arguments (= files to process) */
	int index;
  	for (index = optind; index <= argc; index++){

		FILE *fp = NULL;
		char *line = NULL;
		size_t length = 0;
		size_t read;

		int count_read=0;
		int count_write=0;
		char current_char = '\0';
		int current_count = 0;

		if(optind == argc) { // read from commandline
			fp = stdin;
		
		} else { // open file for reading
			if(index < argc){
			  fp = fopen(argv[index], "r");
		
			  if (fp == NULL) {
			    //close outfile and exit
			    if(outfile != NULL) fclose(fp_outfile);
			    exit_error("Unable to open file.\n");
			  }
			}
		}

		if (fp == NULL) {
//			fprintf(stdout, "why did this happen?");
			continue;
		}

		while ( (read=getline(&line, &length, fp)) != -1) {

			//handle new lines
			if(read == 1){
				current_char = '\n';
				current_count += 1;
				continue;
			}

			//process each line of the file
			int i=0;
			for (i=0; i<read; ++i){
				if (current_char == '\0'){
					current_char=line[i];
					current_count = 1;
				} else {
					if (current_char == line[i]){
						current_count++;
					} else {
						if (current_char != line[i]){
							count_read+=current_count;
							//TODO: count chars of integer > 9
							count_write+=2;
							// if outfile is set write to fp_outfile otherwise to stdout
							if(outfile != NULL) fprintf(fp_outfile, "%c%d", current_char, current_count);
							else fprintf(stdout, "%c%d", current_char, current_count);
							current_char=line[i];
							current_count=1;
						}
					}
				}
			}
		}
		count_read+=current_count;
		//TODO: count chars of integer > 9
		count_write+=2;
		if(outfile != NULL) fprintf(fp_outfile, "%c%d", current_char, current_count);
		else fprintf(stdout, "%c%d", current_char, current_count);

		fflush(stdout);

		//calculate compression ration
		float ratio = (((count_write+count_read)/(float)count_read)-1)*100.00;
		ratio = ((int)((ratio+0.05)*100))/100.00;

		//print stats to stderr
		fprintf(stderr, "Read:\t%d characters\n", count_read);
		fprintf(stderr, "Written:\t%d characters\n", count_write);
		fprintf(stderr, "Compression ration:\t%.1f%%\n", ratio);

		//check for error with input file
		if (ferror(fp)){
			free(line);
			//close outfile and exit with an error
			if(outfile != NULL) fclose(fp_outfile);
			exit_error("error with input file");
		}

		free(line);
		fclose(fp);
	}

	if(outfile != NULL) {
		fclose(fp_outfile);
	}

	exit(EXIT_SUCCESS);
}
