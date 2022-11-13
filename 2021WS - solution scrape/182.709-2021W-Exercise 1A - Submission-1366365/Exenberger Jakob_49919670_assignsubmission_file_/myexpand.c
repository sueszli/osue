#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* 
* @author: Jakob Exenberger - 11707692
* @name: myexpand
* @brief: reads files or from stdin, and replaces all tabs by user specified number of spaces
* @details: parses the arguments, opens stdin or in a loop calls per file given the function "replaceTabs". This function replaces the tabs
    with a specified number of spaces given by the user, if non given the default is 8
* @date: 07.11.2021  
*/

/* 
* @brief: Prints the synopsis end exits the program
* @details: Prints the synopsis of the program to the stdout. After that, the exit function with the code "EXIT_FAILURE" 
    is called to end the program 
* @params: void
* @return: void
*/
static void usage(void) {
    printf("Usage: myexpand [-t tabstop] [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}

/* 
* @brief: Reads the input file replaces the tabstops and writes the result to the outputfile
* @details: read all lines from the input File and replace all the tabs with the given number of spaces stored 
    in tabstop write the result in the output File  
* @params: 
*   input: a FILE which contains the inputfile, from which the function reads
*   output: a FILE which the function will write the resulting chars
*   tabstop: a number which specifics with how many spaces a tab should be replaced 
* @return: void
*/
static void replaceTabs(FILE * input, FILE * output, int tabstop) {
    char *line = NULL;
    char *out = NULL;
    size_t len = 0;
    ssize_t nread;
    while ((nread = getline(&line, &len, input)) != -1) {
            char * copy;
            out = malloc((sizeof(char)) * ((sizeof(line) / sizeof(char)) * tabstop));
            int i = 0;
            for (copy = line; *copy != '\0'; copy++) {                
                // check if current char is tab
                if (*copy == '\t') {
                    int p = tabstop * ((i / tabstop) +1);
                    for (; i < p; i++)
                    {
                        // push space until next character position is reached 
                        out[i] = ' ';
                    }
                } else {
                    // push normal char
                    out[i] = *copy;
                    i++;
                }
            }
            fwrite(out, i, 1, output);
            free(out);
           }
}

/* 
* @brief: Main function which parses the arguments by the user and depending on the arguments reading the files given or reading from stdin
* @details: the main function parses the arguments comming from the user, if everthing was parsed succesfully, we either call the 
    function replaceTabs one time with the stdin or loop over all given files and call the function for every file individually
* @params: 
*   argc: number of arguments given
*   argv: array with the arugments
* @return: integer with 0 on success and 1 if errors occured
*/
int main(int argc, char *argv[])
{
int tabstop = 8;
FILE * outfile = stdout;

int opt;
int oFlag = 0; 
int tFlag = 0;
char *ptr;
while ((opt = getopt(argc, argv, "t:o:")) != -1) {
    switch (opt)
    {
    // parse argument into int, replace default value
    case 't':    
        tabstop = strtol(optarg, &ptr, 10);
        if (tabstop <= 0) {
            usage();
        }
        tFlag++;
        break;
    // close stdout, open new file with the given argument
    case 'o':
        if (outfile != stdout) {
            fclose(outfile);
        }
        outfile = fopen(optarg, "w");
        oFlag++;
        break;
    default:
        usage();
        break;
    }
}

// either stdout or the given outfile could not be open correctly
if (outfile == NULL) {
    exit(EXIT_FAILURE);
}
// o flag ocurred more then once
if (oFlag > 1) {
    usage();
}
// f flag ocurred more then once
if (tFlag > 1) {
    usage();
}

int inputIndex = optind;

// only one input, read from stdin
if (inputIndex == argc) {
    replaceTabs(stdin, outfile, tabstop);
} 
// multiple input files, process them in a loop
else {
    while (inputIndex < argc) {
        FILE * input = fopen(argv[inputIndex], "r");
        replaceTabs(input, outfile, tabstop);
        fclose(input);
        inputIndex++;
    }   
}


return EXIT_SUCCESS;
}
