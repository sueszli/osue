/**
 * @author Peter Kabelik (01125096)
 * @file ispalindrom.c
 * @date 09.11.2021
 *
 * @brief A program to check for palindromes.
 * 
 * @details This program checks various inputs if they contain palindromes.
 * These checks are performed line by line.
 * Inputs can come from stdin or files, outputs can go to stdout or a file.
 * The palindrome checks can also be made ignoring letter case and/or whitespaces.
 **/

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct option_infos
{
	char* output_file_name;
	bool i_occurred;
	bool s_occurred;		
};

typedef struct option_infos option_infos_t;

/**
 * @brief Prints the usage description and exits.
 *
 * @details Prints the usage description including the program's name to stdout and exits with EXIT_FAILURE.
 *
 * @param program_name The program's name.
 */
static void print_usage_and_exit(char* program_name)
{
	fprintf(stdout, "Usage: %s [-s] [-i] [-o outfile] [file...]\n", program_name);
	
	exit(EXIT_FAILURE);
}

/**
 * @brief Prints a custom error message.
 *
 * @details Prints a custom error message including the program's name to stderr.
 *
 * @param program_name The program's name.
 * @param custom_message The custom error message.
 */
static void print_custom_error(char* program_name, char* custom_message)
{
	fprintf(stderr, "%s: %s\n", program_name, custom_message);
}

/**
 * @brief Prints an error message.
 *
 * @details Prints an error message including the program's name to stderr.
 * It says that a function has failed and prints an error description.
 *
 * @param program_name The program's name.
 * @param failed_function_name The name of the function that failed.
 */
static void print_error(char* program_name, char* failed_function_name)
{
	fprintf(stderr, "%s: %s has failed: %s\n", program_name, failed_function_name, strerror(errno));
}

/**
 * @brief Handles the program arguments.
 *
 * @details Handles the program arguments by parsing them and checking
 * if they are valid. Multiple occurrences are also not allowed.
 * The variable option_infos points to is filled with necessary infos for
 * completing the program's task.
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @param option_infos The pointer to the infos the program needs.
 */
static void parse_arguments(int argc, char* argv[], option_infos_t* option_infos)
{
	bool o_occurred = false;

	int option_character;
	
	while ((option_character = getopt(argc, argv, "io:s")) != -1)
	{
		switch (option_character)
		{
			case 'i':
			{
				if (option_infos->i_occurred)
				{
					print_custom_error(argv[0], "option has occurred more than once -- 'i'");
					print_usage_and_exit(argv[0]);
				}

				option_infos->i_occurred = true;
				
				break;
			}
			case 'o':
			{
				if (o_occurred)
				{
					print_custom_error(argv[0], "option has occurred more than once -- 'o'");
					print_usage_and_exit(argv[0]);
				}
							
				option_infos->output_file_name = optarg;
				
				o_occurred = true;
				
				break;
			}
			case 's':
			{
				if (option_infos->s_occurred)
				{
					print_custom_error(argv[0], "option has occurred more than once -- 's'");
					print_usage_and_exit(argv[0]);
				}
				
				option_infos->s_occurred = true;
				
				break;
			}
			default:
			{			
				print_usage_and_exit(argv[0]);
				
				break;
			}
		}
	}
}

/**
 * @brief Removes the line break from a string.
 *
 * @details Removes the line break, if there is one, from a given string.
 * The change is made on the original string.
 * Also, the line length is updated.
 *
 * @param line The line.
 * @param line_length The pointer to the line length.
 */
static void remove_line_break(char* line, ssize_t* line_length)
{
	if (line[*line_length - 1] == '\n')
	{
		(*line_length)--;
	
		line[*line_length] = '\0';
	}
}

/**
 * @brief Removes the whitespaces from a string.
 *
 * @details Removes all the whitespaces from a given string.
 * The change is made on the original string.
 * Also, the line length is updated.
 *
 * @param line The line.
 * @param line_length The pointer to the line length.
 */
static void remove_spaces(char* line, ssize_t* line_length)
{
	unsigned int writing_index = 0;
			
	for (unsigned int i = 0; i < *line_length; i++)
	{
		line[writing_index] = line[i];
	
		if (isspace((unsigned char)line[i]) == 0)
		{
			writing_index++;
		}
	}
	
	line[writing_index] = '\0';
	
	*line_length = writing_index;
}

/**
 * @brief Converts a string to lower case.
 *
 * @details Converts a given string to lower case.
 * The change is made on the original string.
 *
 * @param line The line.
 * @param line_length The line length.
 */
static void to_lower_case(char* line, ssize_t line_length)
{
	for (unsigned int i = 0; i < line_length; i++)
	{
		line[i] = (char)tolower((unsigned char)(line)[i]);
	}
}

/**
 * @brief Checks if a string is a palindrome.
 *
 * @details Checks if a given string is a palindrome.
 * No changes are made to the original string.
 *
 * @param line The line.
 * @param line_length The line length.
 *
 * @return Returns true if line is a palindrome, false otherwise.
 */
static bool is_palindrome(char* line, ssize_t line_length)
{
	for (int i = 0; i < line_length / 2; i++)
	{
		if (line[i] != line[line_length - 1 - i])
		{
			return false;
		}
	}
	
	return true;
}

/**
 * @brief Checks the input for palindromes.
 *
 * @details Checks the input for palindromes line
 * by line. If a line needs correction, then
 * a duplicate of the line is created to make
 * sure that the original line can be printed
 * as part of the result.
 * For each line a result of the palindrome-check
 * is printed to the output.
 *
 * @param space_ignored Whitespaces should be ignoored.
 * @param case_insensitive Letter case should be ignored.
 * @param input The given input.
 * @param output The given output.
 *
 * @return Returns true if no errors occurred, false otherwise.
 */
static bool check_input(bool space_ignored, bool case_insensitive, FILE* input, FILE* output)
{
	char* line = NULL;
	size_t buffer_length = 0;
	ssize_t line_length = 0;
	
	bool line_correction_needed = space_ignored | case_insensitive;
	
	while ((line_length = getline(&line, &buffer_length, input)) != -1)
	{
		remove_line_break(line, &line_length);
		
		char* line_duplicate = line;
		
		if (line_correction_needed)
		{
			line_duplicate = strdup(line);
			
			if (line_duplicate == NULL)
			{
				free(line);
				
				return false;
			}
			
			if (space_ignored)
			{
				remove_spaces(line_duplicate, &line_length);
			}
			
			if (case_insensitive)
			{
				to_lower_case(line_duplicate, line_length);
			}
		}
		
		if (is_palindrome(line_duplicate, line_length))
		{
			fprintf(output, "%s is a palindrom\n", line);
		}
		else
		{
			fprintf(output, "%s is not a palindrom\n", line);
		}
		
		if (line_correction_needed)
		{
			free(line_duplicate);
		}
	}
	
	free(line);
	
	return true;
}

/**
 * @brief Closes the output.
 *
 * @details Closes the given output if no errors occurred.
 * Otherwise it calls the 'print_error' function and exits
 * with EXIT_FAILURE.
 *
 * @param output The given output.
 * @param program_name The program's name.
 */
static void close_output(FILE* output, char* program_name)
{				
	if (fclose(output) != 0)
	{
		print_error(program_name, "fclose");
		
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief The program's' entry point.
 *
 * @details Calls all the functions neccssary to perform
 * the program's tasks. If an error occurs, the program
 * exits with EXIT_FAILURE after cleaning up.
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 *
 * @return Returns EXIT_SUCCESS, if no errors occurred.
 */
int main(int argc, char* argv[])
{
	option_infos_t option_infos = { NULL, false, false };

	parse_arguments(argc, argv, &option_infos);

	int positional_argument_count = argc - optind;
	
	FILE* output = stdout;
	
	if (option_infos.output_file_name != NULL)
	{
		output = fopen(option_infos.output_file_name, "w");
		
		if (output == NULL)
		{
			print_error(argv[0], "fopen");
			
			exit(EXIT_FAILURE);
		}
	}
	
	if (positional_argument_count > 0)
	{
		for (int i = 0; i < positional_argument_count; i++)
		{
			FILE* input = fopen(argv[optind + i], "r");
			
			if (input != NULL)
			{
				if (!check_input(option_infos.s_occurred, option_infos.i_occurred, input, output))
				{
					print_error(argv[0], "check_input");
					
					if (fclose(input) != 0)
					{
						print_error(argv[0], "fclose");
						
						if (fclose(input) != 0)
						{
							print_error(argv[0], "fclose");
						}
						
						close_output(output, argv[0]);
						
						exit(EXIT_FAILURE);
					}
				}
			}
			else
			{
				print_error(argv[0], "fopen");
				
				close_output(output, argv[0]);
				
				exit(EXIT_FAILURE);
			}
			
			if (fclose(input) != 0)
			{
				print_error(argv[0], "fclose");
				
				close_output(output, argv[0]);
				
				exit(EXIT_FAILURE);
			}
		}
	}
	else
	{
		if (!check_input(option_infos.s_occurred, option_infos.i_occurred, stdin, output))
		{
			print_error(argv[0], "check_input");
			
			close_output(output, argv[0]);
			
			exit(EXIT_FAILURE);
		}
	}
	
	close_output(output, argv[0]);

	return EXIT_SUCCESS;
}
