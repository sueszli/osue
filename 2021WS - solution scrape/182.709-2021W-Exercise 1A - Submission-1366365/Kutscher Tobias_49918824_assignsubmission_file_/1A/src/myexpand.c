/**
 * @file myexpand.c
 * @author Tobias Kutscher <e1634063.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief Myexpand program.
 *
 * This program takes a string input fia file or terminal and expands all tab characters to a given amount of spaces
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "myexpand.h"

int t_arg = 8; /**< Length of a tabstop, may be overwritten by an input */

/**
 * function to create a string with tabstop amount of spaces
 * @brief This function returns a string with tabstop spaces.
 * @details This function returns a string with tabstop spaces.
 * @param amount of spaces to put in the string
 * @return the string of spaces
 */
static char * createSpaces(int amount) {
    char *spaces = malloc(amount * sizeof(char));
    for(int i = 0; i < amount; i++) {
        strcat(spaces, " ");
    }
    return spaces;
}

/**
 * reads a single line with buffer
 * @brief This function returns single line, from a file or the terminal input
 * @details reads part of the line until the whole line is read. uses BUFSIZE which is the best buffer size for the given system
 * @param file to read from
 * @return string with the complete line
 */
char *readline(FILE * file)
{
    size_t size = 0;
    size_t len  = 0;
    size_t last = 0;
    char *buf = NULL;

    do {
        size += BUFSIZ;
        buf = realloc(buf, size);

        if (buf == NULL) return NULL;
        fgets(buf + last, size, file);
        len = strlen(buf);
        last = len - 1;
    } while (!feof(file) && buf[last] != '\n');
    return buf;
}
/**
 * Char replacement in a string
 * @brief Replaces all specific chars with a part of a given string.
 * @details chars are replaced according to the formular: p = tabstop * ((x / tabstop) + 1)
 * @param search char to be replaces
 * @param replace string to replace the char
 * @param string string where chars are replaced
 */
static char * stringReplace(char search, char *replace, char *string) {
	char *tempString, *searchStart;
	int len=0;

	searchStart = strchr(string, search);
	if(searchStart == NULL) {
		return string;
	}
    while(searchStart != NULL) {
        tempString = (char*) malloc(strlen(string) * sizeof(char));
        strcpy(tempString, string);
            
        len = searchStart - string;
        string[len] = '\0';
        int amountOfSpaces = t_arg - (len % t_arg);

        strncat(string, replace, amountOfSpaces);

        len++;
        strcat(string, (char *)tempString + len);

        searchStart = strchr(string, search);

    }

    free(tempString);
	
	return string;
}

/**
 * File Reader an Replacement caller
 * @brief This function reads files and writes the replaced string in an output
 * @details global_variables: t_arg
 */
void readFile(FILE *file, FILE *o) {
    char *read;
    char * spaces = createSpaces(t_arg);
    while (((read = readline(file))[0] != '\0') && (read != NULL) && (!feof(file))) {
        //leave if exit
        if (strcmp(read, "exit\n") == 0)
        {
            break;
        }
        char *def = stringReplace('\t', spaces, read);
        fputs(def, o); 
        fflush(o);   
    }
    free(spaces);
}

/**
 * Program entry point.
 * @brief Starting point of the Program, parses arguments, reads files, delegates to functions
 * @details main reads the arguments and checks if files need to be read. start ReadFiles and sets the output.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]) {

    int c;
    char *o_arg = NULL;


    while ( (c = getopt(argc, argv, "o:t:h")) != -1) {
        switch ( c ) {
            case 'h' :
                fprintf(stderr, "usage: myexpand [-t tabstop] [-o outfile] [file...]\n");
                return 0;
            case 't' : sscanf(optarg, "%d", &t_arg);
                break;
            case 'o': o_arg = optarg;
                break;
            case '?':
                if (optopt == 'o')
                    fprintf (stderr, "Option -%o requires an argument.\n", optopt);
                else if (optopt == 't')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                return EXIT_FAILURE;
            default:
                abort ();
        }
    }

    //set fileOut (Terminal or File)
    FILE *fileOut = stdout;
    if (o_arg != NULL) {
        fileOut = fopen(o_arg, "a");
    }

    // read from Terminal
    if ( (argc - optind) < 1) {
        readFile(stdin, fileOut);
        if (fclose(fileOut) != 0)
        {
            fprintf(stderr, "error while closing file, %s\n", strerror(errno));
        }
        return EXIT_SUCCESS;
    }

    int i = 0;
    while (argc > i  + optind) {
        char *fileName = argv[i+optind];
        i++;
        FILE *file;
        file = fopen(fileName, "r");
        if (file == NULL)
        {
            fprintf(stderr, "error while opening file, %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
        readFile(file, fileOut);
        if (fclose(file) != 0)
        {
            fprintf(stderr, "error while closing file, %s\n", strerror(errno));
        }

    }
    if (fclose(fileOut) != 0)
    {
        fprintf(stderr, "error while closing file, %s\n", strerror(errno));
    }
}