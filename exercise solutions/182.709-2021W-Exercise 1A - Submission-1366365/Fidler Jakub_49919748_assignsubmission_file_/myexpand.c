
/**
 * @file myexpand.c
 * @author Jakub Fidler 12022512
 * @date 22 Oct 2021
 * @brief myexpand reads strings, replaces tabs with spaces and prints them to an output
 * 
 * @details myexpand takes the option -o outputfile where outputfile is a file. If no output file is given it will print to stdout. 
 * It also takes an -t tabstop option where tabstop is a number. 
 * Furthermore, the programm takes an arbitrary number of input files as arguments. If no input files are given, myexpand reads from stdin.
 * For each input file given, myexpand reads it line by line, replaces tabs with spaces and writes the resulting lines to the given output file (or stdout).
 * The amount of space characters is chosen so that the characters after the added spaces start at the smallest possible multiple of tabstop.
 */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

char *program_name;

/**
 * @brief reads from an input file, replaces tabs with spaces and writes the results to an output file.
 * @details expand_and_print reads from an input file, for each line it replaces the tab characters with
 * space characters. The amount of space characters is chosen so that the characters after the added spaces start at
 * the smallest possible multiple of tabstop
 * @param in is read and processed. It gets closed after processing.
 * @param out is where the resulting lines get written to. Out is NOT closed after processing. 
 */
void expand_and_print(FILE* in, FILE* out, long tabstop);

/**
 * @brief prints the usage message to stderr of the program and exits.
 * @details prints the usage message in format "USAGE: PROGRAM_NAME options arguments"  on stderr and exits the program.
 */
void usage(void);

/**
 * @brief prints an error message to stderr and exits the program.
 * @details prints an given error message in format  "[PROGRAM_NAME] ERROR: reasons" to stderr and exits the program afterwards.
 * @param error_message custom message to be printed.
 */
static void error(char *error_message);


int main(int argc, char *argv[]) {
    program_name = argv[0];

    long tabstop;
    int opt_t = 0;
    
    FILE *out;
    int opt_o = 0;

    int c;
    while ( (c=getopt(argc, argv, "t:o:") ) != -1 ) {
        switch (c) {
            case 't':
                opt_t++;
                char *endptr;
                tabstop = strtol(optarg, &endptr, 10);
                if(*optarg == '\0' || *endptr != '\0') error("tabstop argument has to be a number");
                break;
            case 'o':
                opt_o++;
                if((out = fopen(optarg, "w")) == NULL){
                    error("Failed to open outpfile");
                }
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
        }
    }

    if ((opt_o > 1 || opt_t > 1)) usage();
    if(opt_o == 0) out = stdout;
    if(opt_t == 0) tabstop = 8;

    if (argv[optind] == NULL) {
        // input from stdin
        FILE *in = stdin;
        expand_and_print(in, out, tabstop);
        if(fclose(out) != 0) {
            error("fclose failed");
        }
    } else {
        // input from files from command line 
        int arg_idx = optind;
        while (argv[arg_idx] != NULL) {
            FILE *in;
            if ((in = fopen(argv[arg_idx], "r")) == NULL) {
                error("Could not open file: fopen failed");
            }        
            expand_and_print(in, out, tabstop);
            arg_idx++;
        }

        if(fclose(out) != 0) {
            error("flcose failed");
        }
    }
}

void expand_and_print(FILE* in, FILE* out, long tabstop){
    
    char *line = NULL;
    size_t n = 0;
    while(getline(&line, &n , in) != -1) {
        char *cur_start = line;
        char *cur_end;
        int cur_result_line_length = 0;
        while(1){
            cur_end = strchr(line, '\t');
            if(cur_end == NULL) cur_end = line + strlen(line);

            // prints chars from cur_start to cur_end (not inclusive)
            fprintf(out, "%.*s", (int) (cur_end - cur_start), cur_start);
            
            if(*cur_end == '\0') break;
            
            cur_result_line_length += cur_end - cur_start;
            int num_spaces = tabstop - ((cur_result_line_length) % tabstop);
            for(int i=0; i<num_spaces; i++){
                fputs(" ", out);
            }
            cur_result_line_length += num_spaces;

            *cur_end = '.'; // replace the \t so that strchr searches for the next one
            cur_start = cur_end + 1;
        }
    }
    free(line);
}


void usage(void) {
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", program_name);
    exit(EXIT_FAILURE);
}

static void error(char *error_message) {
    fprintf(stderr, "[%s] ERROR: %s: %s.\n", program_name, error_message, strcmp(strerror(errno), "Success") == 0 ? "Failure" : strerror(errno));
    exit(EXIT_FAILURE);
}