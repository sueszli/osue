/**
 * @name mydiff
 * @author Christian Perzy [11809921]
 *
 * @brief Assignment A1 for OSUE21
 *
 * This program compares to files line by line and search for
 * differences char by char.
 **/

#include "mydiff.h"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

char *progname; /**< program name */

volatile sig_atomic_t quit = 0; /**< flag for program termination */

/**
 * Signal handler for program termination
 * @brief This Functions sets the quit flag to 1
 * @param Signal
 */
static void handle_signal(int signal) {
    quit = 1;
}

/**
 * Mandatory usage function.
 * @brief This function writes usage information to stderr.
 * @details global variables: progname
 */
static void usage(void) {
    fprintf(stderr,"Usage: %s [-i] [-o outfile] file1 file2\n",progname);
    exit(EXIT_FAILURE);
}

/**
 * This function compares two Strings char by char.
 * @brief Compares two Strings. If the char on position i is not
 * equal a counter will be incremented.
 * @details The comparison stops if '\n' or '\0' occurs in one
 * String.
 * @param String1
 * @param String2
 * @return Retuns the counter
 */
static int strcmp_cc(const char *s1, const char *s2) {
    int counter = 0;
    int i = 0;
    
    while (1){
        char c1 = s1[i];
        char c2 = s2[i];

        if (c1 == '\n' || c2 == '\n' ||
            c1 == '\0' || c2 == '\0') break;
        
        if (c1 != c2) counter++;
        i++;
    }

    return counter;
}

/**
 * Converts all uppercase letters in a string to lowercase
 * letters
 * @brief If a character is in the range between 'A' and 'Z'
 * the value 32 will be added to parse the char in his lowercase
 * version.
 * @details The proccess will end if a '\0' occurs.
 * @param String
 */
static void convert_to_lowercase(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = str[i] + 32;
        }
    }
}

/**
 * Main function of the Program
 * @brief This function parse the argument vector, open and
 * close all needed files and contains the main logic for the
 * program.
 * @details global variables: progname
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return EXIT_SUCCESS resp. EXIT_FAILURE if an error occurs.
 */
int main(int argc, char** argv) {
    // ############################ PREPERATION SECTION ############################
    // parse argument vector
    // open needed files
    // set up signal handler

    progname = argv[0];

    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    char c;
    int case_sensitive = 1;
    char *output_name = NULL;

    int i_counter = 0;
    int o_counter = 0;

    while ((c = getopt(argc,argv,"io:")) != -1) {
        switch (c) {
            case 'i':
            case_sensitive = 0;
            i_counter++;
            break;
            case 'o':
            output_name = optarg;
            o_counter++;
            break;
            case '?': usage();
            break;
            default: assert(0);
            break;
        }
    }

    if (i_counter > 1 || o_counter > 1) {
        fprintf(stderr,"Some options occur more than once!\n");
        usage();
    }

    FILE* file1 = fopen("difftest1.txt", "r");
    FILE* file2 = fopen("difftest2.txt", "r");

    if (file1 == NULL || file2 == NULL) {
        fprintf(stderr, "[%s] fopen failed: %s\n",progname,strerror(errno));
        exit(EXIT_FAILURE);
    }

    // set output to stdout if no outfile is given
    // set output to outfile if one is specified
    FILE *output = stdout;
    if (output_name != NULL) {
        FILE* out = fopen(output_name, "w");
        if (out == NULL) {
            fprintf(stderr, "[%s] output File not available: %s\n",progname,strerror(errno));
            exit(EXIT_FAILURE);
        }
        output = out;
    }

    // ############################ MAIN SECTION ############################
    // contains program logic

    char *line1 = NULL;
    char *line2 = NULL;
    size_t len1 = 0;
    size_t len2 = 0;
    int line_num = 1;

    // get lines until no one is left
    while(getline(&line1, &len1, file1) != -1 &&
          getline(&line2, &len2, file2) != -1 &&
          quit == 0) {
        // if the program should not operate case sensitive
        // parse both strings to lowercase
        if (case_sensitive == 0) {
            convert_to_lowercase(line1);
            convert_to_lowercase(line2);
        }

        int diff = strcmp_cc(line1,line2);

        if (diff > 0) fprintf(output, "Line: %d, characters: %d\n", line_num, diff);
        line_num++;
    }

    // ############################ CLEANUP SECTION ############################
    // free used memory
    // close used files

    free(line1);
    free(line2);

    if (fclose(file1) != 0 || fclose(file2)) {
        fprintf(stderr,"[%s] close Files failed: %s\n",progname,strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (output != stdout) {
        if (fclose(output) != 0) {
            fprintf(stderr,"[%s] close Outputfile failed: %s\n",progname,strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}