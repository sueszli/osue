/**
	@author Michael Winter 01426162
	@brief custom Unix expand command
	@date 14.11.2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

static void print(FILE *in, FILE *out, int ts);

/**
* Program entry point.
* @param argc The argument counter
* @param argv The argument vector
* @return Returns EXIT_SUCCESS
*/
int main(int argc, char **argv) {

	int ts = 8, opt;
	char *outfile;
	int outfile_flag = false;

	/* argument parsing  */
	while ((opt = getopt(argc, argv, "t:o:")) != -1) {
		switch (opt) {
			case 't':
				ts = strtol(optarg, NULL, 10);
				break;
			case 'o':
				outfile = optarg; 
				outfile_flag = true;
				break;
			default:
				fprintf(stderr, "%s: Usage: myexpand [-t tabstop] [-o outfile] [file...]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (ts <= 0) {
		fprintf(stderr, "tabstop must be greater than 0\n");
		fprintf(stderr, "%s Usage: myexpand [-t tabstop] [-o outfile] [file...]\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	FILE *out = stdout;
	if (outfile_flag)
		out = fopen(outfile, "w+");
	if (out == NULL) {	
		fprintf(stderr, "%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (optind < argc) {
        while (optind < argc) {
            FILE *in = fopen(argv[optind], "r");
            print(in, out, ts);
            optind++;
        }
    } else {
        int c;
        while ((c = getc(stdin)) != EOF) {      
            print(stdin, out, ts);
        }
    }

	return EXIT_SUCCESS;
}

/**
* @brief replaces found tabs with spaces
* @param in file to read from
* @param out file to write to
* @param ts specifies the tabstop distance (default is 8)
*/
static void print(FILE *in, FILE *out, int ts) {
	int c, p, i = 0, o = 0;

	while ((c = fgetc(in)) != EOF) {
		if (c == '\t') {
			p = ts * ((o/ts) + 1);
			if (o == 0) {
				fputc(' ', out);
				o++;
			}
			while (o < p) {
				fputc(' ', out);
				o++;
			}
			i++;
		} 
		else {
			fputc(c, out);
			i++;	
			o++;
			if (c == '\n')
				o = 0;
		}	
	}
}

// tested on inf140
