/**
* @file myexpand.c
* @author Thomas Kipper Pires <52004233>
* @date 08.11.2021
*
* @brief Main program module.
*
* @details This program reads in several files and replaces tabs with spaces.
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include "myexpand.h"

static char *myprog = "myexpand"; // name of this program

/**
* @brief Prints out correct usage of program and exits.
* @details Writes the correct usage of the program and 
* its arguments to the console and exits as a failure ;(.
**/
static void usage(void) {
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]", myprog);
    exit(EXIT_FAILURE);
}

int main(int argc, char *const argv[]) {

    // initialize tabstop with default value of 8 and 
    // go through loop to get arguments for tabstop and outfile
    int t = 8;
    int t_opt = 0;
    char *o_arg = NULL;
    int o_opt = 0;
    int c;
    while ((c = getopt(argc, argv, "t:o:")) != -1) {
        switch (c) {
            case 't':
                t_opt++;
                sscanf(optarg, "%d", &t);
                if (t < 1) {
                    fprintf(stderr, "\nTabstop needs to be a positive integer!");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                o_opt++;
                o_arg = optarg;
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
                break;
        }
    }

    // call usage function if [-t] or [-o] were used more than once
    if ( t_opt > 1) {
        usage();
    }
    if( o_opt > 1) {
        usage();
    }

    FILE *outfile;
    // if outfile is specified it is opened here
    if (o_arg != NULL) {
        if ((outfile = fopen(o_arg, "w")) == NULL) {
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    char buffer[1024];
    // fill buffer with data so no jargin is left
    for (int a = 0; a < 1024; a++){
        buffer[a] = 0;
    }

    char tmp[sizeof(buffer)/sizeof(char)];
    for (int a = 0; a < 1024; a++){
        tmp[a] = 0;
    }

    char a = ' ';
    int y = 0;
    int x = 1; // current position in line

    // if no file to read from is specified
    if ((argc == 5 && o_opt == 1 && t_opt == 1) || (argc == 1 && o_opt == 0 && t_opt == 0) || 
        (argc == 3 && o_opt == 1 && t_opt == 0) || (argc == 3 && o_opt == 0 && t_opt == 1)) {
        
        // get user input
        while ((a = fgetc(stdin)) != EOF) {
            buffer[y] = a;
            y++;
        }

        int i = 0;

        // same as logic below
        for (int b = 0; b < 1024; b++) {
            a = buffer[b];

            if (a == '\n') {
                x = 0;
            }

            if (a == '\t') {
                int pos = t * ((x / t) + 1);
                int j = 0;
                int offset = x;
                if (x % t == 0) {
                    pos = x;
                }
                if (o_arg != NULL) {
                    for (j = offset; j <= pos; j++) {
                        if (fputc(' ', outfile) == EOF) {
                            fprintf(stderr, "fputc failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        tmp[i+(j-offset)] = ' ';
                        x++;
                    }
                }
                else {
                    for (j = offset; j <= pos; j++) {
                        tmp[i+(j-offset)] = ' ';
                        x++;
                    }
                }
                i += (j-offset);
                
            }
            else if (o_arg != NULL) {
                if (fputc(a, outfile) == EOF) {
                    fprintf(stderr, "fputc failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                tmp[i] = a;
                i++;
                x++;
            }
            else {
                tmp[i] = a;
                i++;
                x++;
            }

        }
        if (o_arg == NULL) {
        printf("\n%s", tmp);
        }
    }
    
    // go through the files specified if any present
    for (int i = optind; i < argc; i++) {

        FILE *file;
        if ((file = fopen(argv[i], "r")) == NULL) {
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        char c = ' ';
        int i = 0;
        int x = 1; // current position in line

        // loop goes through characters of each file specified 
        // and checks if it is a tab and replaces them with spaces
        while ((c = fgetc(file)) != EOF) {

            // reset line position if new line
            if (c == '\n') {
                x = 0;
            }
            
            if (c == '\t') {
                int pos = t * ((x / t) + 1); // next tab position according to formula
                int j = 0;
                int offset = x;
                if (x % t == 0) {
                    pos = x;
                }
                if (o_arg != NULL) {
                    for (j = offset; j <= pos; j++) {
                        if (fputc(' ', outfile) == EOF) {
                            fprintf(stderr, "fputc failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        buffer[i+(j-offset)] = ' ';
                        x++;
                    }
                }
                else {
                    for (j = offset; j <= pos; j++) {
                        buffer[i+(j-offset)] = ' ';
                        x++;
                    }
                }
                i += (j-offset);
            }
            else if (o_arg != NULL) {
                if (fputc(c, outfile) == EOF) {
                    fprintf(stderr, "fputc failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
                buffer[i] = c;
                i++;
                x++;
            }
            else {
                buffer[i] = c;
                i++;
                x++;
            }

        }

        fclose(file);

        // print to console if no outfile specified
        if (o_arg == NULL) {
        printf("\n%s", buffer);
        }
    }

    if (o_arg != NULL) {
        fclose(outfile);
    }

    return 0;
}