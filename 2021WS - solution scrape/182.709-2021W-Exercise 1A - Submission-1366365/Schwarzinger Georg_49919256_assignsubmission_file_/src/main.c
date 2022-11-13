#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <memory.h>
#include <ctype.h>
#include <signal.h>

/**
 * @brief checks if the entered data are palidrome
 * 
 * @name Name: ispalindrom_td
 * @author Georg Schwarzinger 12024747
 * @date 11.11.2021
 */


// #define _DEBUG

const char *OPTIONS = "sio:";

// A struct holding the program configuration.
// A bit field has been used to reduce the amount of storage needed
struct
{
	FILE *output;
	unsigned char ignore_whitespace : 1;
	unsigned char ignore_case : 1;
	char *output_file;
	size_t input_file_count;
	char **input_files;
} program_options = { //designated initializer, 0 initialization is strictly speaking redundant
	.ignore_case = 0,
	.ignore_whitespace = 0,
	.output_file = NULL,
	.input_file_count = 0,
	.input_files = NULL};


void parse_command_line_options(int argc, char **argv);


void check_allocation(void *ptr, int line);


void parse_input_stream(FILE *ifstream);

/**
 * @brief Checks whether the given line is a palindrome and prints the 
 *        result to the configured output
 * @param line (char *): The String with the line to be checked
 * @param last_char (size_t): The last character to be checked (to exclude newline characters!)
 */
void check_palindrome(char *line, size_t last_char);


void handle_sigint(int signum);


void split_input_files();


void print_dbg_information();

/**
 * @brief (Attempts to) free up all ressources allocated
 */
void cleanup();

int main(int argc, char **argv)
{
	// register a interrupt signal to handle the ^C
	signal(SIGINT, handle_sigint);

	// parse the argvs
	parse_command_line_options(argc, argv);

#ifdef _DEBUG
	print_dbg_information();
#endif //_DEBUG

	split_input_files();

	cleanup();
	exit(EXIT_SUCCESS);
}

/**
 * @brief Parses the argument line options
 * @details reads the input of argv and argc
 */
void parse_command_line_options(int argc, char **argv)
{
	program_options.output = stdout;
	opterr = 0; // avoid getopt() to clutter our stderr
	int c;
	while ((c = getopt(argc, argv, OPTIONS)) != -1)
	{
		switch (c)
		{
		case 's':
			program_options.ignore_whitespace = 1;
			break;

		case 'i':
			program_options.ignore_case = 1;
			break;

		case 'o':
		{

			size_t filename_length = strlen(optarg) + 1;								  // BUGBUG MAYBE check if file name is valid
			program_options.output_file = (char *)malloc(filename_length * sizeof(char)); //dynamic memory management for the size of the file
			check_allocation(program_options.output_file, __LINE__);

			snprintf(program_options.output_file, filename_length, "%s", optarg);

			program_options.output = fopen(program_options.output_file, "w");

			if (program_options.output == NULL)
			{
				fprintf(stderr, "Unable to open output file! Sorry\n");
				cleanup();
				exit(EXIT_FAILURE);
			}

			break;
		}

		case '?': //handling incorrect user input
			if (optopt == 'o')
				fprintf(stderr, "Option -o(outputfile) requires one argument!\n");
			if (isprint(optopt))
				fprintf(stderr, "Unknown option: %c\n", optopt);
			else
				fprintf(stderr, "Unprintable option character: %x\n", optopt);
			break;
		default:
			cleanup();
			exit(EXIT_FAILURE);
		}
	}

	program_options.input_file_count = argc - optind;
	program_options.input_files = (char **)malloc(program_options.input_file_count * sizeof(char *)); //size of char pointer
	check_allocation(program_options.input_files, __LINE__);

	size_t file_name_length = 0;
	for (int i = 0; i < program_options.input_file_count; i++)
	{
		file_name_length = strlen(argv[i + optind]) + 1;
		program_options.input_files[i] = (char *)malloc(file_name_length * sizeof(char));
		check_allocation(program_options.input_files[i], __LINE__);
		snprintf(program_options.input_files[i], file_name_length, "%s", argv[i + optind]);
	}
}
/**
 * @brief checks if the allocation is free
 * 
 * @param ptr: pointer to the allocation 
 * @param line: line should be replaced by __line__ 
 */
void check_allocation(void *ptr, int line)
{
	if (ptr == NULL)
	{
		fprintf(stderr, "Unable to allocate memory in line %d \n", line);
		cleanup();
		exit(EXIT_FAILURE);
	}
}
/**
 * @brief file which reads the stream and splits every line
 * 
 * @param ifstream: file which can be read in line by line
 */
void parse_input_stream(FILE *ifstream)
{
	char *line = NULL;
	size_t llength = 0;
	ssize_t read = 0;
	while ((read = getline(&line, &llength, ifstream)) != -1)
	{
		if (line[read - 1] == '\n')
			check_palindrome(line, read - 2);
		else
			check_palindrome(line, read - 1);
	}
}


/**
 * @brief check if a the input line is a palidrom 
 * 
 * @param line: which line is chossen 
 * @param last_char: index of the last charcter  
 */
void check_palindrome(char *line, size_t last_char)
{
	char *i, *j;
	i = line;
	j = i + last_char;

	if (program_options.ignore_whitespace)
	{
		while (isspace(*(++i)))
			;

		while (isspace(*(--j)))
			;
	}

	while (i < j)
	{
		if (program_options.ignore_case)
		{
			if (tolower(*i) != tolower(*j))
				break;
		}
		else if (*i != *j)
			break;

		if (program_options.ignore_whitespace)
		{
			while (isspace(*(++i)))
				;

			while (isspace(*(--j)))
				;
		}
		else
		{
			i++;
			j--;
		}
	}

	if (i >= j)
		fprintf(program_options.output, "%.*s is a palindrom\n", (int)last_char + 1, line);
	else
		fprintf(program_options.output, "%.*s is not a palindrom\n", (int)last_char + 1, line);
}
/**
 * @brief handels the signals
 * 
 * @param signum: Signal parameter 
 */
void handle_sigint(int signum)
{
	fprintf(stderr, "\nProcess interrupted by user!\n");
	cleanup();
	exit(EXIT_SUCCESS);
}

/**
 * @brief Takes the input files and transfrom them into inputstreams
 * 
 */
void split_input_files()
{
	if (program_options.input_file_count)
		for (int i = 0; i < program_options.input_file_count; i++)
		{
			FILE *ipf = fopen(program_options.input_files[i], "r");

			if (ipf == NULL)
			{
				fprintf(stderr, "Unable to open input file %s \n", program_options.input_files[i]);
				cleanup();
				exit(EXIT_FAILURE);
			}
			parse_input_stream(ipf);

			if (fclose(ipf) != 0)
			{
				fprintf(stderr, "Unable to close output file stream\n");
				exit(EXIT_FAILURE);
			}
		}
	else
	{
		parse_input_stream(stdin);
	}
}
/**
 * @brief prints debug information
 * 
 */
void print_dbg_information()
{
	printf(
		"Program Options:\n-Ignore Whitespace = %s \n-Ignore Case = %s\n-Use Output File = %s\n",
		program_options.ignore_whitespace ? "True" : "False",
		program_options.ignore_case ? "True" : "False",
		program_options.output != stdout ? "True" : "False");

	if (program_options.output != stdout)
		printf("-Output File = %s\n", program_options.output_file);

	if (program_options.input_file_count)
	{
		printf("-Input files:\n");
		for (int i = 0; i < program_options.input_file_count; i++)
			printf("\t- %s\n", program_options.input_files[i]);
	}
}
/**
 * @brief frees our alloceted memory
 * 
 */
void cleanup()
{
	free(program_options.output_file);

	for (int i = 0; (program_options.input_files != NULL) && (i < program_options.input_file_count); i++)
		free(program_options.input_files[i]);

	free(program_options.input_files);

	if (program_options.output != stdout)
		if (fclose(program_options.output) != 0)
		{
			fprintf(stderr, "Unable to close output file stream\n");
			exit(EXIT_FAILURE);
		}
}