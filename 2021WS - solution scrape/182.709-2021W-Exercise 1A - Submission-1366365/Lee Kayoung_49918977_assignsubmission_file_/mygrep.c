#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#define MAXLINE 1000

int processFile(FILE *in_file, FILE *out_file, int opt_i, char* input) 
{
	char *ret;
	char *output;
	while ( fscanf(in_file, "%s", &ret ) == 1 ) { 
		
		int len = strlen(output);
		if (opt_i > 0) { // ignore case
			int i;
			for(i = 0; i < len; i++)
				output[i] = tolower(output[i]);
		}
		
		ret = strstr(output, input);
		if (ret) {
			fprintf(out_file, "%s", output);
		} else {
			fclose(stdin);
		}
	}
}

int main(int argc, char *argv[])
{

    char *o_arg = NULL;
	int opt_i = 0;
	int c;
	
	FILE *out_file = stdout;
	
	// process options
	while ( (c = getopt(argc, argv, "o:i")) != -1 ){
		switch ( c ) {
			case 'o':
			{
				FILE *f = fopen(optarg, "w");
				if (f) {
					out_file = f;
				} else {
					printf("Error: Cannot open output file");
					return -1;
				}
				break;
			}
			case 'i':  
				opt_i++;
				break;
			case '?': /* invalid option */
				break;
			default:
				break;
		}
	}
	
	// process input(=the substring)
	char *input = argv[optind];
	int len = strlen(input);
	int i;
	if (opt_i > 0) { // ignore case
		for(i = 0; i < len; i++)
			input[i] = tolower(input[i]);
	}

	// process output(=the string)
	char *ret;
    char output[MAXLINE];
	int quit = 0;
    while ( fgets(output, MAXLINE, stdin) != NULL ) 
	{	
		FILE *in_file = NULL;

		len = strlen(output);
		for(i = 0; i < len; i++) { // check if output is a file
			if(output[i] == '.') {
				in_file = fopen(output, "r");
			}
		}
		
		if (opt_i > 0) { // ignore case
			for(i = 0; i < len; i++)
				output[i] = tolower(output[i]);
		}
		
		if (! in_file) {
			ret = strstr(output, input);
			if (ret) {
				fprintf(out_file, "%s", output);
			} else {
				fclose(stdin);
			}
		} else {
			processFile(in_file, out_file, opt_i, input);
		}
    } 
	
	return 0;
}
