/**
 * @file mygrep.c
 * @author Aiden Foster 11910604
 * @date 11.11.2021
 *
 * @brief program module to check for matches to a given string in files
 * 
 * This program is used to check for matches to a given string
 *  this can either be done via stdin if no input file is specified or via input files
 *  the output can either be a file (option -o) or stdout
 *  in case a match is found in a line is output
 *  in case no match is found in a line the line is not output
**/

#include "mygrep.h"

/**
 * usage function
 * @brief output a usage message to stderr and terminate
**/
static void usage(char* programname){
	fprintf(stderr, "Usage: %s [-i] [-o outfile] keyword [file...]\n", programname);
	exit(EXIT_FAILURE);
}

/**
 * @brief check for full line
 * @param buffer The buffer in which to check for a full line
 * @param length The length of the buffer in which to search
 * @return length if buffer contains a full line, -1 otherwise
**/
static int is_full_line(char* buffer, int length){
	for(int i=0; i<length; i++){
		if(buffer[i] == '\n') return i;
	}
	return -1;
}

/**
 * replace a char in a string with another char
 * @brief replaces any occurance of toReplace in buffer[0] to buffer[length-1] with replaceWith
 * @param buffer The buffer in which to replace chars
 * @param length The length of the buffer in which to replace
 * @param toReplace The char to replace
 * @param replaceWith The char to replace with
**/
static void replace_char(char* buffer, int length, char toReplace, char replaceWith){
	for(int i=0; i<length; i++){
		if(buffer[i] == toReplace) buffer[i] = replaceWith;
	}
}

/**
 * @brief checks if the string in buffer contains the keyword
 *
 * @param buffer       The buffer in which to search
 * @param real_length  The length of the string in the buffer (excluding '\0')
 * @param keyword      The keyword to match
 * @param ignoreCase   Disables case sensitivity if > 0
 * @return             0 if the given string matches the keyword
**/
static int matchesKeyword(char* buffer, int real_length, char* keyword, int ignoreCase) {
    /*
    if (ignoreCase > 0) {
        if (strcasestr(buffer, keyword) == NULL) {
            return 1;
        } else {
            return 0;
        }
    } else {
        if (strstr(buffer, keyword) == NULL) {
            return 1;
        } else {
            return 0;
        }
    }
    */
    int keyword_length = strlen(keyword);
    for (int i=0; i <= real_length-keyword_length; i++) {
        int matches = 0;
        for (int j=0; j<keyword_length; j++) {
            if ((buffer[i+j] != keyword[j] && ignoreCase == 0) || (tolower(buffer[i+j]) != tolower(keyword[j]))) {
                matches = 1;
                break;
            }
        }
        if (matches == 0) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief output buffer to fp if fp != NULL or to stdout otherwise
 * outputs ("%s %s\n", buffer, toAppend)
 * @param buffer    The buffer to output
 * @param fp        A file pointer to which to output, if NULL is given the output is directed to stdout
 * @return 0 if output was successful, -1 otherwise
**/
static int output(char* buffer, FILE* fp){
	if(fp == NULL) {
		printf("%s\n", buffer);
	} else {
		char* end = "\n";
		//write buffer space toAppend end to fp in this order while checking for errors
		if(fputs(buffer, fp) == EOF || fputs(end, fp) == EOF){
			return -1;
		}
		fflush(fp);
	}
	return 0;
}

/**
 * Program entry point
 * @brief mygrep checks files or stdin for matches to the given keyword and outputs a line if it finds a match. The output is written to an output file if one is given and stdout otherwise
 * @param argc The amount of arguments passed
 * @param argv[] The arguments passed
 *
 * argv can contain the following:
 *  -i		disable case sensitivity
 *  -o file file to which to output
 *  file...	files to read, if none are given use stdin
 *
 *  @return EXIT_FAILURE if program encountered an error, EXIT_SUCCESS otherwise
**/
int main(int argc, char * argv[]) {
	char *outfile = NULL;
	int opt_o = 0;
	int opt_i = 0;
	int c;
	while((c = getopt(argc, argv, "iso:")) != -1) {
		switch(c) {
			case 'o':
				outfile = optarg;
				opt_o++;
				break;
			case 'i':
				opt_i++;
				break;
			case '?':
			default:
				usage(argv[0]);
				break;
		}
	}
	if(opt_i > 1 || opt_o > 1 || (argc - optind) <= 0 || (opt_o > 0 && outfile == NULL) || (opt_o > 0 && outfile[0] == '-')){
		usage(argv[0]);
	}

    char *keyword = argv[optind];

    FILE *fp_out = NULL;
	int file_num = argc - 1 - optind;
	if(outfile != NULL) {
		for(int i=0; i<file_num; i++){
			if(strcmp(outfile, argv[optind+i+1]) == 0){
				fprintf(stderr, "%s is output and input file!\n", outfile);
				exit(EXIT_FAILURE);
			}
		}
		if( (fp_out = fopen(outfile, "w")) == NULL ){
			fprintf(stderr, "fopen for %s failed: %s\n", outfile, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	FILE *fp_in;
	int read_from_file = 1;
	if(file_num == 0) {
		file_num = 1;
		fp_in = stdin;
		read_from_file = 0;
	}
	int buffer_length = DEFAULT_BUFFER_LENGTH;
	char* buffer = (char*) malloc(buffer_length * sizeof(char));
	if(buffer == NULL){
		fprintf(stderr, "not enough memory available!\n");
		exit(EXIT_FAILURE);
	}
	for(int i=0; i<file_num; i++) {
		if(read_from_file != 0) {
			if( (fp_in = fopen(argv[optind + i + 1], "r")) == NULL ) {
				fprintf(stderr, "fopen for %s failed: %s\n", argv[optind + i + 1], strerror(errno));
				if(fp_out != NULL) {
                    fclose(fp_out);
                }
				exit(EXIT_FAILURE);
			}
		}
		int length;
		char* current_buffer = buffer;
		int current_buffer_length = buffer_length;
		while(fgets(current_buffer, current_buffer_length, fp_in) != NULL) {
			if( (length = is_full_line(buffer, buffer_length)) >= 0) {
				current_buffer = buffer;
				current_buffer_length = buffer_length;
				replace_char(buffer, length+1, '\n', '\0');
				if(matchesKeyword(buffer, length, keyword, opt_i) == 0) {
					if(output(buffer, fp_out) <0 ) {
						fprintf(stderr, "output to %s failed: %s\n", outfile, strerror(errno));
						free(buffer);
						if(fp_out != NULL) fclose(fp_out);
						if(read_from_file != 0) fclose(fp_in);
						exit(EXIT_FAILURE);
					}
                }
			} else {
				buffer_length *= 2;
				buffer = realloc(buffer, buffer_length);
				//overwriting '\n' in buffer to add new string to buffer
				current_buffer_length = buffer_length/2 + 1;
				current_buffer = &buffer[buffer_length/2 -1];
			}
		}
		if(ferror(fp_in)){
			fprintf(stderr, "fgets for %s failed: %s\n", argv[optind + i + 1], strerror(errno));
			free(buffer);
			if(fp_out != NULL) fclose(fp_out);
			if(read_from_file != 0) fclose(fp_in);
			exit(EXIT_FAILURE);
		}
		if(fclose(fp_in) < 0) {
			fprintf(stderr, "fclose for %s failed: %s\n", argv[optind + i + 1], strerror(errno));
			free(buffer);
			if(fp_out != NULL) fclose(fp_out);
			exit(EXIT_FAILURE);
		}
	}
	if(fp_out != NULL && fclose(fp_out) < 0) {
		fprintf(stderr, "fclose for %s failed: %s\n", outfile, strerror(errno));
		free(buffer);
		exit(EXIT_FAILURE);
	}
	free(buffer);
	exit(EXIT_SUCCESS);
}
