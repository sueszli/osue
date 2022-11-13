#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>

#define BUFF_SIZE (2024)
/**global program name*/
static const char *PROGRAM_NAME;

void remove_spaces(char *s);
static void isPalindrom(int opt_i, int opt_s, char work_str_line[], char org_str_line[], FILE *output);
static void read_input_from_file(int opt_i, int opt_s, char *output_path, char *input_files[], int file_size);
static void read_input_from_stdin(int opt_i, int opt_s, char *output_path);
static void USAGE();
static void exit_error(char *errorMessage);

int main(int argc, char *argv[])
{

	PROGRAM_NAME = argv[0];
	// used to track user inputs parameter via getopt
	int opt = 0;
	// check if the option with export to a file is chosen
	int opt_o = 0;
	// check if the option to ignore lower and upper cases is chosen
	int opt_i = 0;
	// check if the option to ignore white spaces is chosen
	int opt_s = 0;

	char *output_path = NULL;

	while ((opt = getopt(argc, argv, "sio:")) != -1)
	{

		switch (opt)
		{
		case 's':
			opt_s++;
			break;

		case 'i':
			opt_i++;
			break;

		case 'o':
			output_path = optarg;
			opt_o++;
			break;

		case '?':
			USAGE();
			break;

		default:
			USAGE();
			break;
		}
	}

	// none of the option can be used more than once
	if (argc < 1 || opt_o > 1 || opt_i > 1 || opt_s > 1)
	{
		USAGE();
	}
	// if the option to export solutions is chosen, output file name must be defined as well
	if (opt_o == 1 && output_path == NULL)
	{
		USAGE();
	}

	// if input files are added to read input from otherwise read from stdin
	if ((argc - optind) > 0)
	{
		// create string array to hold each input file path
		char *input_files[argc - optind];

		for (int i = 0; argv[optind + i] != NULL; i++)
		{
			input_files[i] = argv[optind + i];
		}

		// define soze of input file
		int file_size = sizeof(input_files) / sizeof(char *);
		read_input_from_file(opt_i, opt_s, output_path, input_files, file_size);
	}
	else
	{
		// if no input files are defined, read input from stdin
		read_input_from_stdin(opt_i, opt_s, output_path);
	}

	exit(EXIT_SUCCESS);
}

/** 
 * @brief function to send the seleted options and list or input files to read from and send them to ispalndrim function
 * @par int opt_i holds flag for the -i option
 * @par int opt_s holds flag for the -s option
 * @par if defined it holds the path for the export file path
 * @par list of file pathes to read from
 * @par number of files to read from
*/
static void read_input_from_file(int opt_i, int opt_s, char *output_path, char *input_files[], int file_size)
{

	FILE *fP;
	FILE *output = NULL;

	if (output_path != NULL)
	{
		output = fopen(output_path, "w+");
		if (output == NULL)
		{
			exit_error("Failed to open output file");
		}
	}

	char *buffer, *original;
	buffer = (char *)malloc(BUFF_SIZE * sizeof(char));
	original = (char *)malloc(BUFF_SIZE * sizeof(char));

	for (int i = 0; i < file_size; i++)
	{
		fP = fopen(input_files[i], "r");
		if (fP == NULL)
		{
			exit_error("input file not found");
		}
		else
		{

			while (fgets(original, BUFF_SIZE, fP) != NULL)
			{

				strcpy(buffer, original);
				char org[BUFF_SIZE * sizeof(char) + 1];
				strcpy(org, buffer);
				org[BUFF_SIZE * sizeof(char)] = '\0';

				isPalindrom(opt_i, opt_s, buffer, org, output);
			}
			{
			}
			fclose(fP);
		}
	}
	if (output != NULL)
	{
		fclose(output);
	}
	free(original);
	free(buffer);
}

/** 
 * @brief function to send the seleted options and path to the export file
 * @par int opt_i holds flag for the -i option
 * @par int opt_s holds flag for the -s option
 * @par if defined it holds the path for the export file path
*/
static void read_input_from_stdin(int opt_i, int opt_s, char *output_path)
{

	FILE *output = NULL;

	// check if output file is defined
	if (output_path != NULL)
	{
		output = fopen(output_path, "w+");
		if (output == NULL)
		{
			exit_error("Failed to open output file");
		}
	}

	// buffer to holds the orginal value and a copy of it
	char *buffer, *original;
	buffer = (char *)malloc(BUFF_SIZE * sizeof(char));
	original = (char *)malloc(BUFF_SIZE * sizeof(char));

	// as long an input is available in stdin contiue in the loop and prase the input
	while (fgets(original, BUFF_SIZE, stdin) != NULL)
	{
		// make a copy of the input from stdin
		strcpy(buffer, original);
		char org[BUFF_SIZE * sizeof(char) + 1];
		strcpy(org, buffer);
		// define end of line for the copy
		org[BUFF_SIZE * sizeof(char)] = '\0';
		//call isPalindrom for this line
		isPalindrom(opt_i, opt_s, buffer, org, output);
	}
	// close and free the used resources
	if (output != NULL)
	{
		fclose(output);
	}
	free(original);
	free(buffer);
}

static void isPalindrom(int opt_i, int opt_s, char work_str_line[], char org_str_line[], FILE *output)
{

	bool write_in_output_flag = false;

	if (opt_s)
	{
		remove_spaces(work_str_line);
	}

	if (output != NULL)
	{
		write_in_output_flag = true;
	}

	int i = 0;
	int j = strlen(work_str_line) - 1;

	if (work_str_line[j] == '\n')
	{
		work_str_line[j] = ' ';
	}

	int k = strlen(org_str_line) - 1;
	if (org_str_line[k] == '\n')
	{
		org_str_line[k] = ' ';
	}

	while (work_str_line[j] == ' ' || work_str_line[j] == '\n' || work_str_line[j] == '\0')
	{
		j = j - 1;
	}

	while (i <= j)
	{

		if (work_str_line[i] != work_str_line[j])
		{
			// check if the option i is chosen to ignore the empty spaces
			if (opt_i)
			{
				if (tolower(work_str_line[i]) != work_str_line[j])
				{
					if (tolower(work_str_line[j]) != work_str_line[i])
					{ // check where to write the results
						if (write_in_output_flag)
						{
							fprintf(output, "%s is not a palindrom \n", org_str_line);
						}
						else
						{
							fprintf(stdout, "%s is not a palindrom \n", org_str_line);
						}

						return;
					}
				}
			}
			else
			{ // check where to write the results
				if (write_in_output_flag)
				{
					fprintf(output, "%s is not a palindrom \n", org_str_line);
				}
				else
				{
					fprintf(stdout, "%s is not a palindrom \n", org_str_line);
				}
				return;
			}
		}

		i = i + 1;
		j = j - 1;
	}
	// check where to write the results
	if (write_in_output_flag)
	{
		fprintf(output, "%s is a palindrom \n", org_str_line);
	}
	else
	{
		fprintf(stdout, "%s is a palindrom \n", org_str_line);
	}
}

/** 
* @brief when the option -s is selected, white spaces should be ignored 
*
* @param char *s strings to remove the empty space from
*/
void remove_spaces(char *s)
{
	const char *j = s;
	while ((*s++) = *j++)
	{
		while (*j == ' ')
		{
			j = j + 1;
		}
	}
}

/** 
* @brief to exit the program in case of an error and to display the required error message
*
* @param char *errorMessage holds error message to be printed on stderr
*/
static void exit_error(char *errorMessage)
{
	fprintf(stderr, "%s Error: %s\n", PROGRAM_NAME, errorMessage);
	exit(EXIT_FAILURE);
}

static void USAGE()
{
	fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", PROGRAM_NAME);
	exit(EXIT_FAILURE);
}
