/**
 * @file mydiff.c
 * @name mydiff
 * @author Krumay Clemens (e11831160@student.tuwien.ac.at)
 * @brief compares two files line by line, usage: mydiff [-i] [-o outfile] file1 file2
 * @details compares each line with number of characters equal to the shorter len, arguments: [-i] means cas insensitive, [-o outfile] changes the output to outfile instead of stdout
 * @date 2021-11-14
 */
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<ctype.h>

#define BUFFLEN 1024 //line read buffer length

char *name; // name of program
FILE *outstream = NULL, *diff1 = NULL, *diff2 = NULL; // file pointer of output and input streams

/**
 * @brief exits program and closes files
 * @details outstream, diff1 and diff2 are closed
 * 
 * @param code exit code
 */
static void terminate(int code) {
	if (outstream) fclose(outstream);
	if (diff1) fclose(diff1);
	if (diff2) fclose(diff2);
	exit(code);
}

/**
 * @brief compares two strings
 * @details compares each character in a string up to the length of the shorter string
 * 
 * @param a String 1
 * @param b String 2
 * @param sensitive 0 = case sensitive, 1 = case insensitive
 * @return int number of differences
 */
static int strcountcmp(char *a, char *b, int sensitive) {
	int count = 0;

	while (*a && *b && *a != '\n' && *b != '\n') {
		if (!sensitive) {
			if (*a != *b) count++;
		}
		else {
			if (tolower(*a) != tolower(*b)) count++;
		}

		a++;
		b++;
	}

	return(count);
}

/**
 * @brief main
 * @details handles arguments, open files, compares lines of files, prints number of differences per line
 * 
 * @param argc number of arguments
 * @param argv -i makes comparison cas insensitive, -o defines an outputfile instead of stdout, file1 and file2 to be compared
 * @return int EXIT_SUCCESS on success, EXIT_ERROR on error 
 */
int main(int argc, char *argv[]) {
	int opt, sensitive = 0, linenr = 1, count = 0, eol1 = 0, eol2 = 0, finished = 0;
	char line1[BUFFLEN], line2[BUFFLEN];

	outstream = stdout;
	name = argv[0];

	/* argument handling */
	while ((opt = getopt(argc, argv, "io:")) != -1) {
		switch(opt) {
			case 'i':
				sensitive = 1;
				break;
			case 'o':
				if (outstream != stdout) {
					fprintf(stderr, "[%s] mydiff [-i] [-o outfile] file1 file2\n", name);
					terminate(EXIT_FAILURE);
				}
				outstream = fopen(optarg, "w");
				if (outstream == NULL) {
					fprintf(stderr, "[%s] can't open %s: %s\n", name, optarg, strerror(errno));
					terminate(EXIT_FAILURE);
				}
				break;
			default:
				fprintf(stderr, "[%s] mydiff [-i] [-o outfile] file1 file2\n", name);
				terminate(EXIT_FAILURE);
		}
	}

	if (argc-optind != 2) {
		fprintf(stderr, "[%s] mydiff [-i] [-o outfile] file1 file2\n", name);
		terminate(EXIT_FAILURE);
	}
	
	argv += optind;
	argc -= optind;

	/* opening input files */
	diff1 = fopen(argv[0], "r");
	if (diff1 == NULL) {
		fprintf(stderr, "[%s] can't open %s: %s\n", name, argv[0], strerror(errno));
		terminate(EXIT_FAILURE);
	}
	diff2 = fopen(argv[1], "r");
	if (diff2 == NULL) {
		fprintf(stderr, "[%s] can't open %s: %s\n", name, argv[1], strerror(errno));
		terminate(EXIT_FAILURE);
	}

	/* main algorithm */
	while(!finished) { // loops until one file reaches eof
		eol1 = eol2 = count = 0;
		while (!eol1 || !eol2) { // loops until both files reach an eol
			if (!eol1) {
				if (fgets(line1, BUFFLEN-1, diff1) == NULL) {
					if (feof(diff1)) eol1 = finished = 1;
					else {
						fprintf(stderr, "[%s] couldn't read %s\n", name, argv[0]);
						terminate(EXIT_FAILURE);
					}
				}
			}
			if (!eol2) {
				if (fgets(line2, BUFFLEN-1, diff2) == NULL) {
					if (feof(diff2)) eol2 = finished = 1;
					else {
						fprintf(stderr, "[%s] couldn't read %s\n", name, argv[1]);
						terminate(EXIT_FAILURE);
					}
				}
			}
			if (!eol1 && !eol2) count += strcountcmp(line1, line2, sensitive);
			if (line1[strlen(line1)-1] == '\n') eol1 = 1;
			if (line2[strlen(line2)-1] == '\n') eol2 = 1;
		}
		if (count>0) {
			if (fprintf(outstream, "Line: %d, characters: %d\n", linenr, count)<0) {
				fprintf(stderr, "[%s] can't write: %s\n", name, strerror(errno));
				terminate(EXIT_FAILURE);
			}
		}
		linenr++;
	}

	terminate(EXIT_SUCCESS);
}
