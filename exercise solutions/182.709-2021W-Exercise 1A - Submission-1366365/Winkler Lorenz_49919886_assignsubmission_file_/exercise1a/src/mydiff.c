/**
 * @file mydiff.c
 * @author Lorenz Winkler (e12020650@student.tuwien.ac.at)
 * @brief This file is a tool for comparing a file's lines per character.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief This function outputs the synopsis of the program and then terminates it.
 * It should be used when invalid input (args) are detected.
 * 
 */
void output_synops()
{
	printf("SYNOPSIS: \n\tmydiff [-i] [-o outfile] file1 file2\n");
	exit(EXIT_FAILURE);
}

/**
 * @brief Get the number of different characters of two strings up to the end of the shorter string.
 * 
 * @param line1 the first line to compare
 * @param line2 the second line to compare
 * @param ignoreCase if the function should compare case-insensitive
 * @return the number of different characters 
 */
int get_dif_for_lines(char *line1, char *line2, bool ignoreCase)
{
	size_t str1len = strlen(line1);
	size_t str2len = strlen(line2);
	int difcount = 0;

	for (int i = 0; i < str1len && i < str2len; i++)
	{
		if (line1[0] == '\n' || line2[0] == '\n')
		{
			break;
		}
		if (ignoreCase)
		{
			if (strncasecmp(line1, line2, 1) != 0)
			{
				difcount++;
			}
		}
		else
		{
			if (strncmp(line1, line2, 1) != 0)
			{
				difcount++;
			}
		}
		line1++;
		line2++;
	}
	return difcount;
}

/**
 * @brief This function takes at least 2 positional arguments, which are paths to the files to compare.
 * There are two optional arguments: -o which takes a parameter for an outputfile and -i with which the comparison is no longer
 * case sensitive.
 * 
 * @param argc Argument counter
 * @param argv Arguments
 * @return int 
 */
int main(int argc, char **argv)
{

	char *output_file = NULL;
	int opt_i = 0;
	int c;

	while ((c = getopt(argc, argv, "o:i")) != -1)
	{
		switch (c)
		{
		case 'o':
			output_file = optarg;
			break;
		case 'i':
			opt_i++;
			break;
		default:
			output_synops();
		}
	}

	char **pospoint = &argv[optind];
	char *file1 = NULL;
	char *file2 = NULL;
	for (int i = 0; *pospoint != NULL; pospoint++, i++)
	{
		switch (i)
		{
		case 0:
			file1 = *pospoint;
			break;
		case 1:
			file2 = *pospoint;
			break;
		default:
			output_synops();
		}
	}
	if (file1 == NULL || file2 == NULL)
	{
		output_synops();
	}

	FILE *input1, *input2;
	FILE *output = NULL;

	if ((input1 = fopen(file1, "r")) == NULL)
	{
		fprintf(stderr, "%s: File not found: %s\n", argv[0], file1);
		output_synops();
	}
	if ((input2 = fopen(file2, "r")) == NULL)
	{
		fprintf(stderr, "%s: File not found: %s\n", argv[0], file2);
		output_synops();
	}
	if ((output_file != NULL) && (output = fopen(output_file, "w")) == NULL)
	{
		fprintf(stderr, "%s: Unable to open file for writing: %s\n", argv[0], output_file);
		output_synops();
	}
	if (output == NULL)
	{
		output = stdout;
	}

	char *line1 = NULL;
	char *line2 = NULL;
	size_t len1, len2;
	ssize_t read1, read2;
	int linecount = 1;

	while ((read1 = getline(&line1, &len1, input1) != -1) && (read2 = getline(&line2, &len2, input2) != -1))
	{
		int difcount = get_dif_for_lines(line1, line2, opt_i > 0);

		if (difcount > 0)
		{
			fprintf(output, "Line: %d, characters: %d\n", linecount, difcount);
		}
		linecount++;
	}

	return EXIT_SUCCESS;
}
