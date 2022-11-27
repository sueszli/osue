/**
 * @file main.c
 * @author Gernot Hahn 01304618
 * @date 13.11.2021
 *
 * @brief This program compresses files.
 *
 * @detail This program reads any number of files, compresses them and
 * writes the result to an optional specified file. The compression function
 * as follows: Consecutive occurences of the same character are counted and then
 * written into the output file. If no input file is given, stdin will be used
 * instead. The ouput is defaulting to stdout.
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

char  *prog_name;

/**
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: prog_name
 */
void usage(void) {
	fprintf(stderr, "usage: %s [-o outfile] [file...]\n", prog_name);
	exit(EXIT_FAILURE);
}

/**
 * @brief This function performs the compression. 
 * @details The function parses the arguments. Then it loops through each
 * given input file and performs the compression. Therefor consecutive occurences
 * of the same character are counted and then written inot the output file, when
 * another character is read.
 * global variables: prog_name 
 * 
 * @param argc argument counter.
 * @param argv argument vector.
 * @return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]) {

	prog_name = argv[0];
	int c;
	int opt_o = 0;
	FILE *outfile = stdout;
	FILE *infile = stdin;

	while ((c = getopt(argc, argv, "o:")) != -1) {
		switch (c) {
			case 'o':
				opt_o++;
				if (opt_o > 1) {
					usage();
				}
				outfile = fopen(optarg, "w");
				if (outfile == NULL) {
					fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
					exit(EXIT_FAILURE);
				}
				break;
			case '?':
			default:
				usage();
		}
	}

	int infile_count = argc - optind;
	bool infile_arg = true;
	int next_ch;
	int ch_count = 1;
	int read = 0;
	int written = 0;

	if (infile_count == 0) {
		infile_arg = false;
		infile_count++;
	}

	while (infile_count > 0) {
		if (infile_arg) {
			infile = fopen(argv[optind++], "r");
		}

		if (infile == NULL) {
			fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if ((c = fgetc(infile)) != EOF) {
			do {
				read++;
				next_ch = fgetc(infile);
				if (c == next_ch) {
					ch_count++;
					continue;
				} else {
					fprintf(outfile, "%c%d", c, ch_count);
					c = next_ch;
					ch_count = 1;
					written += 2;
				}
			} while (next_ch != EOF);
		}

		if (ferror(infile) != 0) {
			fprintf(stderr, "[%s] ERROR: read error: %s\n", prog_name, strerror(errno));
			exit(EXIT_FAILURE);
		}

		fclose(infile);
		infile_count--;
	}

	fclose(outfile);

	fprintf(stderr, "\nRead:	   %d characters\n", read);
	fprintf(stderr, "Written:   %d characters\n", written);
	fprintf(stderr, "Compression ratio: %0.1f%%\n", ((float)written / (float)read) * 100);

	return EXIT_SUCCESS;
}

