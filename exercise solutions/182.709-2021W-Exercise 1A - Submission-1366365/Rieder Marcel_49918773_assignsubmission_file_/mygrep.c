/**
 *@file mygrep.c
 *@author Marcel Rieder <e11742405@student.tuwien.ac.at>
 *@date 27.10.2021
 *
 *@brief Main programm module
 * 
 * This program is an imitation of the grep function on unix. It has its most basic funtionalities for
 * searching textfiles for occurences of a keyword. It also supports text-insensitive search and writing
 * to an output file.
 * 
 *          
 **/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

char *myprog;


/**
 * Mandatory usage function.
 *@brief This function writes helpful usage information about the program to stderr.
 *@details global variables: myprog
      
 **/
static void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-o Outfile] keyword [file...]\n",
	    myprog);
    exit(EXIT_FAILURE);
}



/**
 * Auxiliary function 
 * @brief This function takes a null-terminated string of chars and turns them into their 
 * Uppercase-Variants
 * @details This function is primarily used to implement the case-insensitive search.
 **/
static void my_string_toupper(char *string)
{
    while ((*string) != '\0') {
	(*string) = toupper(*string);
	string++;
    }

}


/**
 * Most important function
 * @brief This function handles the reading and searching of the inputfiles.
 * @details It uses the fact that stdin and stdout behave like files to its advantage to save code
 * @param keyword this is the searchterm
 * @param fptr is either stdout or another outoutfile specified by the o-Option
 * @param offset marks the beginning of the inputfiles
 * @param case_insensitive either one or 0 depending on the specified case senitivity 
 * @param argc argument counter from main
 * @param argv argument vector from main
 * **/
void search_files(char *keyword, FILE * fptr, int offset,
		  int case_insensitive, int argc, char **argv)
{


    int read_from_stdin = 0;

    if ((offset - argc) == 0) {
	read_from_stdin = 1;
	offset -= 1;
    }

    for (int i = offset; i < argc; i++) {

	FILE *input_fp;
	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	if (read_from_stdin == 0) {
	    input_fp = fopen(argv[i], "r");

	    if (input_fp == NULL) {
		fprintf(stderr, "Opening an input file failed in %s\n",
			myprog);
		exit(EXIT_FAILURE);
	    }
	} else {
	    input_fp = stdin;
	}

	while ((read = getline(&line, &len, input_fp)) != -1) {
	    if (case_insensitive == 1) {

		char *copied_line = malloc(len);
		if (copied_line == NULL) {
		    free(line);
		    fprintf(stderr, "Calling malloc in %s failed.\n",
			    myprog);
		    exit(EXIT_FAILURE);
		}

		strncpy(copied_line, line, len);
		my_string_toupper(copied_line);
		my_string_toupper(keyword);

		if (strstr(copied_line, keyword) != NULL) {
		    fprintf(fptr, "%s", line);
		}
		free(copied_line);
	    } else {
		if (strstr(line, keyword) != NULL) {
		    fprintf(fptr, "%s", line);
		}

	    }

	}
	free(line);

    }
}










/**
 * Program entry point.
 * @brief The program starts here. The main function is responsible for the argument parsing.  
 * 
 * @details It uses getopt() for parsing. All searching functionality has been moved to the 
 *  function search_files
 * global variables: myprog
 * @param argc The argument counter.
 * @param argv The argument vector containing the options, flags, the keyword and the inputfiles.
 * @return Returns EXIT_SUCCESS.
 **/
int main(int argc, char **argv)
{

    myprog = argv[0];
    if (argc == 1) {
	usage();
    }
    char *keyword = NULL;
    int opt_i = 0;
    int opt_o = 0;
    char *arg_o = NULL;

    int c;

    while ((c = getopt(argc, argv, "io:")) != -1) {
	switch (c) {
	case 'i':
	    opt_i++;
	    break;
	case 'o':
	    opt_o++;
	    arg_o = optarg;
	    break;
	case '?':
	    usage();
	    break;
	default:
	    assert(0);
	}

    }

    if (opt_i > 1 || opt_o > 1) {
	usage();
    }


    keyword = argv[optind];

    if (keyword == NULL) {
	usage();
    }

    FILE *fptr;


    if (opt_o == 1) {
	fptr = fopen(arg_o, "w");

	if (fptr == NULL) {
	    fprintf(stderr, "Opening the outputfile failed in %s\n",
		    myprog);
	    exit(EXIT_FAILURE);

	}




    } else {
	fptr = stdout;
    }

    int offset = 2 + opt_i;

    if (opt_o == 1) {
	offset += 2;
    }



    search_files(keyword, fptr, offset, opt_i, argc, argv);



    if (opt_o == 1) {
	fclose(fptr);
    }


    return EXIT_SUCCESS;
}
