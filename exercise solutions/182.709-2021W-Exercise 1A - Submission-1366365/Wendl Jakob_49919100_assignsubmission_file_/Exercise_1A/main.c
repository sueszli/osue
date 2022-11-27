
/**
	@author
		Jakob Wendl, 12026658
	@date
		26.10.2021
	@brief
		Name: main.c
		Source file for the implementation of mycompress
	@detail
		Multiple input files are read and a compressed version of all the files is printed to a specific output file.
		The input files are read one after the other. Each input file is compressed by a specific rule.
		The input is compressed by substituting subsequent identical characters by only one occurence of the
		character followed by the number of characters. For example, the input aaa is compressed to a3.
		The compressed versions of the input files are then printed to one specific output file one after the other.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "main.h"

/**
	@brief
		An input file is read and a compressed version is printed to an output file.
	@details
		The input file is read character by character. A compressed version of the input file is then printed to a specific output file.
		The input is compressed by substituting subsequent identical characters by only one occurence of the
		character followed by the number of characters. For example, the input aaa is compressed to a3.
	@param
		FILE *fp_output: The file where the compressed output is printed
		char *infile: The name of the file, which is read
		int *chars_read: A pointer to the total number of chars read
		int *chars_written: A pointer to the total number of chars written
	@return
		status
*/
static int read_write(FILE *fp_output, char *infile, int *chars_read, int *chars_written);

// Name of the program
char *prog_name;

int main(int argc, char **argv){
	prog_name = argv[0];
	int i;
	char *outfile = "stdout";
	int opt;
	while((opt = getopt(argc, argv, "o:")) != -1){
		if(opt == 'o'){
			if(optarg == NULL){
				fprintf(stderr, "[%s] ERROR: getopt failed: %s\n", prog_name, strerror(errno));
				exit(EXIT_FAILURE);
			}
			outfile = optarg;
		} else if(opt == '?'){
			fprintf(stderr, "[%s] ERROR: getopt failed: %s\n", prog_name, strerror(errno));
			exit(EXIT_FAILURE);
		} else {
			assert(0);
		}
	}
	char **infiles = malloc(sizeof(char *) * (argc + 1 - optind));
	int num_infiles = 0;
	if(infiles == NULL){
		fprintf(stderr, "[%s] ERROR: malloc failed: %s\n", prog_name, strerror(errno));
		exit(EXIT_FAILURE);
	}
	if(optind == argc){
		infiles[0] = "stdin";
		num_infiles++;
	} else if (optind < argc) {
		for(i = optind; i < argc; i++){
			infiles[i - optind] = argv[i];
			num_infiles++;
		}
	} else {
		assert(0);
	}
	FILE *fp_output;
	if(strcmp(outfile, "stdout") == 0){
		fp_output = stdout;
	} else {
		fp_output = fopen(outfile, "w");
		if(fp_output == NULL){
			fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
			free(infiles);
			exit(EXIT_FAILURE);
		}
	}
	int chars_read = 0;
	int chars_written = 0;
	for(i = 0; i < num_infiles; i++){
		int status = read_write(fp_output, infiles[i], &chars_read, &chars_written);
		if(status != 0){
			fprintf(stderr, "[%s] ERROR: read_write failed: %s\n", prog_name, strerror(errno));
			free(infiles);
			exit(EXIT_FAILURE);
		}
	}
	fputc('\n', fp_output);
	if(strcmp(outfile, "stdout") != 0){
		int status = fclose(fp_output);
		if(status == EOF){
			fprintf(stderr, "[%s] ERROR: fclose failed: %s\n", prog_name, strerror(errno));
			free(infiles);
			exit(EXIT_FAILURE);
		}
	}
	fprintf(stderr, "Read:    %d\n", chars_read);
	fprintf(stderr, "Written: %d\n", chars_written);
	double compr_ratio = (((double) chars_written) * 100) / ((double) chars_read);
	fprintf(stderr, "Compression ratio: %f%%\n", compr_ratio);
	return 0;
}

static int read_write(FILE *fp_output, char *infile, int *chars_read, int *chars_written){
	FILE *fp_input;
	if(strcmp(infile, "stdin") == 0){
		fp_input = stdin;
	} else {
		fp_input = fopen(infile, "r");
		if(fp_input == NULL){
			fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	char c_current = fgetc(fp_input);
	char c_prev;
	int count = 1;
	while(c_current != EOF){
		(*chars_read)++;
		c_prev = c_current;
		c_current = fgetc(fp_input);
		if(c_current == c_prev){
			count++;
		} else {
			fputc(c_prev, fp_output);
			(*chars_written)++;
			fputc(count + '0', fp_output);
			(*chars_written)++;
			count = 1;
		}
	}
	if(strcmp(infile, "stdin") != 0){
		int status = fclose(fp_input);
		if(status == EOF){
			fprintf(stderr, "[%s] ERROR: fclose failed: %s\n", prog_name, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	return 0;
}














