#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

// Global to be used in terminate
static char *prg_name_g;

/**
 * @brief Tests if the given character is EOF or a newline character
 *
 * @param character Character to be tested
 * @return bool: true if character is EOF or NL
 */
static inline bool isEOForNewline(int character) {
    return (character == '\n' || character == '\r' || character == EOF);
}

/**
 * @brief Terminates the program and prints the custom error-message and
 * the stderror-message to the console
 *
 * @param character Character to be tested
 */
static void terminate(char *message) {
    fprintf(stderr,"%s - %s: %s", prg_name_g, message, strerror(errno));
    exit(EXIT_FAILURE);
}


/**
 * @brief main
 * 
 * @param argc Argument counter 
 * @param argv Arguments
 * @return int exit code
 */
int main(int argc, char *argv[]) {

    // set program name for output
    prg_name_g = argv[0];

    // parse argument values
    bool case_insensitive = false;
    char *input_path1 = NULL;
    char *input_path2 = NULL;
    char *output_path = NULL;
    
    int c;
    while((c = getopt(argc, argv, "io:")) != -1) {
        switch(c) {
            case 'o': output_path = optarg; break;
            case 'i': case_insensitive = true; break;

            case '?': terminate("Invalid option(s)");
            default:  terminate("Unknown option");
        }
    }

    if(argc - optind != 2) {
        terminate("Invalid number of arguments");
    }
    input_path1 = argv[optind];
    input_path2 = argv[optind+1];

    // open input files
    FILE *input_file1 = fopen(input_path1, "r");
    FILE *input_file2 = fopen(input_path2, "r");

    if(input_file1 == NULL) { terminate("Reading input file 1 failed"); }
    if(input_file2 == NULL) { terminate("Reading input file 2 failed"); }

    // open or create output file, or write to stdout
    FILE *output;
    if(output_path != NULL) {
        output = fopen(output_path, "w");
        if(output == NULL) { terminate("Opening or creating output file failed"); }
    } else {
        output = stdout;
    }

    // compare files
    int (*comparer)(const char*, const char*) = (case_insensitive ? &strcasecmp : &strcmp);

    int differences_line = 0;
    int differences = 0;
    
    int cur_char_file1 = fgetc(input_file1);
    int cur_char_file2 = fgetc(input_file2);

    for(int cur_line = 1; (cur_char_file1 != EOF && cur_char_file2 != EOF); cur_line++) {
        for(int cur_pos = 0; !isEOForNewline(cur_char_file1) && !isEOForNewline(cur_char_file2); cur_pos++) {
            // compare the two characters
            if((*comparer)((char *) &cur_char_file1, (char *) &cur_char_file2) != 0) {
                differences_line++;
                differences++;
            }
            cur_char_file1 = fgetc(input_file1);
            cur_char_file2 = fgetc(input_file2);
        }
        // print differences to output
        if(differences_line > 0) {
            fprintf(output, "Line: %i, characters: %i\n", cur_line, differences_line);
        }

        // go to the beginning of the next line
        while(!isEOForNewline(cur_char_file1)) { cur_char_file1 = fgetc(input_file1); }
        if(cur_char_file1 != EOF) { cur_char_file1 = fgetc(input_file1); }
        else { break; }

        while(!isEOForNewline(cur_char_file2)) { cur_char_file2 = fgetc(input_file2); }
        if(cur_char_file2 != EOF) { cur_char_file2 = fgetc(input_file2); }
        else { break; }

        differences_line = 0;
    }
    if(differences == 0) {
        fprintf(output, "No differnces found\n");
    }

    fclose(input_file1);
    fclose(input_file2);
    fclose(output);            
    return EXIT_SUCCESS;
}