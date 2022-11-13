/**
 * @file mycompress.c
 * @author Elisa Wang <e12023138@student.tuwien.ac.at>
 * @date 13.11.2021
 *
 * @brief Module compress input and output compress content
 * 
 * This program inputfiles, compresses them and writes to the outputfile, if no input oder outputfile are specified, stdin and stdout are used
 * A short report on the compression details are presented through stderr
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/** @details total written characters*/
int readChars = 0;
/** @details total read characters*/
int writtenChars = 0;

/**
 * Compulsory usage function
 * @brief Prints helpful information on how to use this program
 */
static void usage(void)
{
	fprintf(stderr, "SYNOPSIS: mycompress [-o outfile] [infile...] \n");
	exit(EXIT_FAILURE);
}

/**
 * Compression function
 * @brief reads from input file and compresses content with run length encoding
 * @param input file to read from
 * @param output file to write compressed content
 */
static void compress(FILE *input, FILE *output)
{
	int c = fgetc(input);
	while (c != EOF)
	{
		int occurence = 1;
		int tmp = c;
		do
		{
			tmp = fgetc(input);
			if (c == tmp)
			{
				occurence++;
			}
		} while (tmp == c);
		fprintf(output, "%c", c);
		fprintf(output, "%d", occurence);
		readChars += occurence;
		int digits = 0;
		do
		{
			occurence /= 10;
			digits++;
		} while (occurence != 0);
		writtenChars += digits + 1;
		c = tmp;
	}
}

/**
 * Entry point function
 * @brief reads from input file and compresses content with run length encoding, writes compressed content to outfile 
 * @param argc argument count
 * @param argv argument vector
 * @return EXIT_FAILURE on error, having printed error to stderr, EXIT_SUCCESS on succuess
 */
int main(int argc, char **argv)
{
	char *outfile = NULL;
	FILE *input = stdin;
	FILE *output = stdout;
	int c;

	while ((c = getopt(argc, argv, "o:")) != -1)
	{
		switch (c)
		{
		case 'o':
			outfile = optarg;
			break;
		default:
			usage();
			break;
		}
	}

	if (outfile != NULL)
	{
		output = fopen(outfile, "w");
		if (output == NULL)
		{
			fprintf(stderr, "[%s] Error opening outputfile: %s %s \n", argv[0], outfile, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if (optind == argc)
	{
		compress(input, output);
		if (fclose(input) == EOF)
		{
			fprintf(stderr, "[%s] Error in closing stdin: \n %s", argv[0], strerror(errno));
		}
	}
	else
	{
		for (int i = optind; i < argc; i++)
		{
			char *infile = argv[optind];
			input = fopen(infile, "r");
			if (input == NULL)
			{
				fprintf(stderr, "[%s] Error opening inputfile: %s %s \n", argv[0], argv[optind], strerror(errno));
				if (fclose(output) == EOF)
				{
					fprintf(stderr, "[%s] Error in closing output: %s \n", argv[0], strerror(errno));
				}
				exit(EXIT_FAILURE);
			}
			compress(input, output);
			if (fclose(input) == EOF)
			{
				fprintf(stderr, "[%s] Error in closing input %s: \n %s \n", argv[i], argv[0], strerror(errno));
			}
		}
	}
	fprintf(stderr, "\nRead: \t\t %d characters \n", readChars);
	fprintf(stderr, "Written: \t %d characters \n", writtenChars);
	fprintf(stderr, "Compression Ratio: \t %.1f%% \n", 100.0 * writtenChars / (double)readChars);
	if (fclose(output) == EOF)
	{
		fprintf(stderr, "[%s] Error in closing output: %s \n", argv[0], strerror(errno));
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}
