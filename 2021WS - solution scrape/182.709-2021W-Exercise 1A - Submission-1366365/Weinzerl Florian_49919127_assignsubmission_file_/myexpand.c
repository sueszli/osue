/**
 * @brief This is the main module for myexpand.
 * It contains everything needed for the program to run.
 * @author Florian Weinzerl (11701313)
 * @date 2021-11-13
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#define USAGE_MSG "[%s] error parsing arguments. usage: myexpand [-t tabstop] [-o outfile] [file...]\n"

#define show_error_and_exit(msg, ...) \
	(void)	fprintf(stderr, msg, ##__VA_ARGS__); \
			exit(EXIT_FAILURE)

/**
 * @brief Expands the incoming stream by turning tabs into spaces.
 * @details Lines read through `in` will get expanded according to the tabstop parameter and written back out onto `out`
 * @param in the input stream to be expanded
 * @param out the output stream, where the results will be accessible
 * @param tabstop the number of characters between fixed tab-stops.
 */
static void expand(FILE *in, FILE *out, int tabstop);

/**
 * @brief Prints the usage message to stdout.
 * @details Prints the synopsis for myexpand to stdout.
 * @param prog_name the name of the calling program
 */
static void print_usage(char *prog_name);

int main(int argc, char **argv){
	// parse arguments
	int tabstop = 8;
	char *out_path = NULL;
	int num_in_paths = 0;
	int c;
	while((c = getopt(argc, argv, "t:o:")) != -1)
		switch(c){
			case 't':
				errno = 0;
				char *end;
				tabstop = strtol(optarg, &end, 0);
				if(errno == ERANGE || end == optarg)
					{ show_error_and_exit("[%s] error parsing arg for opt 't': %s\n", argv[0], strerror(errno)); }
				break;
			case 'o':
				out_path = optarg;
				break;
			case '?':
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			default:
				assert(0);
		}
	
	num_in_paths = argc-optind;
	
	FILE* out = stdout;
	if(out_path != NULL){
		out = fopen(out_path, "w");
		if(out == NULL)
			{ show_error_and_exit("[%s] failed to open out-file %s\n", argv[0], out_path); }
	}
	
	if(num_in_paths == 0){
		expand(stdin, out, tabstop);
	}
	// else
	for(int i = 0; i < num_in_paths; i++){
		static FILE *in;
		in = fopen(argv[optind+i], "r");
		if(in == NULL)
			{ show_error_and_exit("[%s] failed to open in-file %s\n", argv[0], argv[optind+i]); }
		
		expand(in, out, tabstop);
		fclose(in);
	}
	
	if(out_path != NULL)
		fclose(out);
	
	return EXIT_SUCCESS;
}

void expand(FILE *in, FILE *out, int tabstop){
	char *line = NULL;
	size_t len = 0;
	size_t expanded_index;
	while(getline(&line, &len, in) != -1){
		//fputs(line, out);
		expanded_index = 0;
		for(int i = 0; i < len; i++){
			if(line[i] == '\t'){
				int spaces = tabstop - (expanded_index % tabstop);
				expanded_index += spaces;
				for(int s = 0; s < spaces; s++) fputc(' ', out);
			}else{
				if(line[i] == '\0') break;
				
				expanded_index++;
				fputc(line[i], out);
			}
		}
	}
	free(line);
}

void print_usage(char *prog_name){
	fprintf(stderr, USAGE_MSG, prog_name);
}
