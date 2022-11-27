/**
 * @file myexpand.c
 * @author Gerald Schindlegger
 * @date 09.11.2021
 *
 * @brief MyExpand program.
 * 
 * This program is the myexpand.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>


/* ----- constants ----- */

#define INPUT_READ_SIZE 255	//max input size for reading once from the terminal or inputfiles


/* ----- prototypes ----- */

/**
 * @brief Generates the output. All tabs found will be replaced with space charecters
 * @param buffer Buffer from input (terminal or inputfiles)
 * @param tabstop The tabstop to calculate the end of the tab when detected
 * @param output The output which is written on stdout or to the output_file
**/
static void generate_output(char buffer[], int tabstop, char *output);

/**
 * @brief Writes the given output to stdout or to the output_file
 * @param output The otput string
 * @param opt_o The indicater if opt_o is given
 * @param output_file The output_file if given
 * @return 0 if success, -1 if failed
**/
static int write_output(char *output, int opt_o, FILE* output_file);

/**
 * @brief define usage function
**/
static void usage(void);


/**
 * @brief Program entry point
 * @param argc The argument counter
 * @param argv The argument vector
 * @return EXIT_SUCCESS on success, EXIT_FAILURE if failures occur
**/
int main(int argc, char *argv[])
{
	
	int opt, opt_t = -1, opt_o = -1;
	int tabstop = 8;
    char *output_file_name;	
	FILE *output_file;															
	char buff[INPUT_READ_SIZE] = "";

	/* check options 
       -t tabstop -o outfile
	   HINT: the function getopt sorts the argv[]! e.g. ./myexpand t1.txt -t 6 --> ./myexpand -t 6 t1.txt 
	   		 this is important if inputfiles are given (start of the for loop) */
    while ((opt = getopt(argc, argv, "t:o:")) != -1) 
	{
		switch (opt) 
		{
			case 't':
				// checkes if the option -t is only given onces 
				if(opt_t != -1)	
				{
					(void)fprintf(stderr, "opt_t multiple times\n");
                	usage();
				}
				opt_t = 1;
				
				// copies the argument from the option t into the tabstop and checks if the input was an integer
				char *temp = "";	
				tabstop = strtol(optarg, &temp, 10);

				if (tabstop <= 0 || temp == optarg || *temp != '\0' || ((tabstop == INT_MIN || tabstop == INT_MAX) && errno == ERANGE))
				{
					(void)fprintf(stderr, "Tabstop must be an integer > 0!\n");
					usage();
				}
				break;

            case 'o':
				// checkes if the option -o is only given onces
				if(opt_o != -1)	
				{
					(void)fprintf(stderr, "opt_o multiple times\n");
                	usage();
				}
				opt_o = 1;
				
				output_file_name = optarg;
				break;

			case '?':
				usage();

			default:
				return EXIT_FAILURE;  
		}
	}    

	// calculate at which position in the argument the input files could starts
	int start_files_arg = 1;
	if (opt_t == 1)
		start_files_arg +=2;
	if (opt_o == 1)
		start_files_arg +=2;

	if (opt_o == 1)
		output_file = fopen(output_file_name, "w");

	if(argc == start_files_arg)
	{
        // no inputfile. read from terminal.
		while(fgets(buff, INPUT_READ_SIZE, stdin))
		{
			char *output = (char *)malloc(sizeof(char));
			generate_output(buff, tabstop, output);
			if (write_output(output, opt_o, output_file) == -1) {
				(void)fprintf(stderr, "Error occurred while writing output\n");
				return EXIT_FAILURE;
			}
			free(output);
		}
	} 
    else
	{
        // inputfiles given. read from the inputfiles.
		for(int i = start_files_arg; i < argc; i++)
		{
			FILE *input = fopen(argv[i], "r");
			// checks if the inputfile can be opened 
			if(input == NULL)	
			{
				(void)fprintf(stderr, "Can't open the file %s!\n", argv[i]);
				usage();
			}
			while(fgets(buff, INPUT_READ_SIZE, input) != NULL)
			{	
    			char *output = (char *)malloc(sizeof(char));
				generate_output(buff, tabstop, output);
				if (write_output(output, opt_o, output_file) == -1) {
					(void)fprintf(stderr, "Error occurred while writing output\n");
					return EXIT_FAILURE;
				}
				free(output);
			}	
			fclose(input);
		}
	}

	if (opt_o == 1)
		fclose(output_file);
	
	return EXIT_SUCCESS;
}


/* ----- implementations ----- */

static void generate_output(char buffer[], int tabstop, char *output)
{
	int output_size = 0;
	int p = 0;

	// goes through the entire buffer
	for(int x = 0; x < strlen(buffer); x++)
	{

		// looks for a tab in the buffer
		if(buffer[x] == '\t')	
		{
			// if a tab is found it calculates the Position p for the next character and fills the space until this position with empty spaces
			p = tabstop * ((x / tabstop) + 1);
			for(int i = x; i < p; i++)
			{
                output_size = output_size + 1;
                output = (char *)realloc(output, output_size * sizeof(char));
				output[output_size - 1] = ' ';
			}
		}

		// if no \t is found, you copy the character at the position x to the output string
		else
		{
            output_size = output_size + 1;
            output = (char *)realloc(output, output_size * sizeof(char));
			output[output_size - 1] = buffer[x];
		}		
	}
}

static int write_output(char *output, int opt_o, FILE* output_file)
{
	if(opt_o == 1)
	{
		if (fputs(output, output_file) == EOF)
			return -1;
		fflush(output_file);
	}
	else 
		(void)fprintf(stdout, "%s", output);	

	memset(output, '\0', strlen(output));
	return 0;
}

static void usage(void)	
{
	(void)fprintf(stderr, "SYNOPSIS: myexpand [-t tabstop] [-o outfile] [file...]\n");
	exit(EXIT_FAILURE);
}
