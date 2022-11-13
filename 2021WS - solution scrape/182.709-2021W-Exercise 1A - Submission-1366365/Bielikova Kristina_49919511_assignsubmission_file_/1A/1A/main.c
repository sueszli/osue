/**
* @file main.c
* @author Kristina Bielikova <e12024707@student.tuwien.ac.at>
* @date 2021-11-14
*
* @brief This program reads reads the files and says if the line is a palindrom or not
*/

#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
* @details INITIAL_BUFFER_SZ initializes the buffer size
*/

#define INITIAL_BUFFER_SZ 4096 //2^12

/**
* @brief static char* get_line(FILE* file) loads the line 
* @details int buf_size initializes the buffer size
* @details int read_so_far initializes read symbols
* @details char* buf allocates the space on memory
* @details bool complete declares a variable
*/

static char* get_line(FILE* file)
{
	int buf_size = INITIAL_BUFFER_SZ;

	int read_so_far = 0;

	char* buf = malloc(buf_size);

	bool complete = false;

	while (!complete)
	{
		if (fgets(&buf[read_so_far], buf_size, file) == NULL) //we load a line from the file to string long buf_size, if it is NULL 
		{
			if (feof(file) == 0) //if we aren't at the end of the file or the file is empty
			{
				free(buf); //we free our memory

				return NULL;
			}
			else if (read_so_far == 0) //if we read nothing, we free our memory
			{
				free(buf);

				return NULL;
			}
			else 
			{
				break;
			}
		}

		read_so_far = strlen(buf); //length of the string

		if (buf[read_so_far - 1] != '\n') //if we don't read the whole line (the allocated memory is too small)
		{
			char* temp = realloc(buf, buf_size * 2); //if the line is longer than INITIAL_BUFFER_SZ, we duplicate the memory

			if (temp == NULL) 
			{
				free(buf);

				return NULL;
			}

			buf = temp;

			buf_size *= 2;
		}
		else
		{
			complete = true; //the line was read
		}
	}

	buf[read_so_far - 1] = '\0'; //the last symbol will be '\0'

	return buf;
}

/**
* @brief static bool check_file_for_palindroms(FILE* file, FILE* output, bool ignore_whitespace, bool ignore_case) checks if the line is a palindrom
* @details char* line defines the line
*/


static bool check_file_for_palindroms(FILE* file, FILE* output, bool ignore_whitespace, bool ignore_case)
{
	char* line; //defines the line

	while ((line = get_line(file)) != NULL) //if the loaded line isn't NULL
	{

		int length = strlen(line); //the length of the line

    	bool palindrom = true;
		
    	fprintf(output, "%s ", line); //we print the line

		if(ignore_case){

			for(int i = 0; i < length; i++)
    		{
    		    if(line[i] >= 'A' && line[i] <= 'Z') //if the symbol is a capital
    		    {
    		        line[i] = line[i] + 'a' - 'A'; //we change the letters to non-capitals (ASCII)
    		    }

    		}
		}

		if(ignore_whitespace)
		{

			for(int i = 0; i < length; i++)
    		{
    		    if(line[i] == ' ') //if there is a space on the place i
    		    {

					for(int j = i; j < length; j++) //all characters after place i
    		        {
					
    		            line[j] = line[j + 1]; //will shift left
    		        }

    		        length--; //without a space the length is smaller

    		    }
    		}
		}

		for(int i = 0; i < length/2; i++) //we iterate just the half of the line
		{
			if(line[i] != line[length - i - 1]) //if the first and the last symbol aren't the same
			{
				palindrom = false; //false
				break;
			}
    	}

    	if(palindrom == true)
		{
			fprintf(output, "is a palindrom\n");
		}

		else
		{
			fprintf(output, "is not a palindrom\n");
		}

		free(line); //we free the memory of the line
	}

	if (feof(file) == 0) //if we aren't at the of the file
		return false;
	else
		return true;
}

/**
* @brief int main(int argc, char** argv) is our main function checking for the options and failures
* @details char option defines our option
* @details bool ignore_whitespace initialises the boolean with false
* @details bool ignore_case initialises the boolean with false
* @details FILE* outfile defines FILE* to Standard Output as default
*/

int main(int argc, char** argv) //char** is array from Character-Pointers
{
	char option;

	bool ignore_whitespace = false;

	bool ignore_case = false;

	FILE* outfile = stdout; // FILE* to Standard Output as default

	while ((option = getopt(argc, argv, "sio:")) != -1) //if we read the option
	{
		switch (option)
		{
		case 's':

			ignore_whitespace = true;

			break;
			
		case 'i':

			ignore_case = true;

			break;

		case 'o':

			if ((outfile = fopen(optarg, "w")) == NULL) //w = write; we open the file for writing, if can't open it
			{
				fprintf(stderr, "[%s] The file %s could not opened.\n", argv[0], optarg); //stderr = standard error 

				return EXIT_FAILURE;
			}

			break;
		}
	}

	if (optind == argc) 
	{  
		if(!check_file_for_palindroms(stdin, outfile, ignore_whitespace, ignore_case))
		{
			fprintf(stderr, "[%s] An error occurred while processing stdin.\n", argv[0]);
			return EXIT_FAILURE;
		}
	}
	
	for (int i = optind; i < argc; ++i) //alle Elemente liegen in argc nach optind
	{
		FILE* current_file = fopen(argv[i], "r");
		if (current_file == NULL)
		{
			fprintf(stderr, "[%s] The file %s could not opened.\n", argv[0], argv[i]);
			return EXIT_FAILURE;
		}
		if (!check_file_for_palindroms(current_file, outfile, ignore_whitespace, ignore_case))
		{
			fprintf(stderr, "[%s] An error occurred while processing the file %s.\n", argv[0], argv[i]);
			fclose(current_file);
			return EXIT_FAILURE;
		}
		fclose(current_file);
	}

	return EXIT_SUCCESS;
}
