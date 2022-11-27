#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <assert.h>

/**
 * @file myexpand.c
 * @author Fodor Francesca-Diana, 11808223
 * @date 5.11.2021
 * @brief The program myexpand recognizes tabulator spaces and replaces them with normal spaces.
 * @details The program myexpand recognizes tabulator spaces and replaces them with normal spaces.
 * The tab space can be changed accordingly, by default it is 8. The input can be either from the command
 * line stdin or from one or more input files given. The output is either written to the command line stdout
 * or to a file.
 **/


int main(int argc, char *argv[]) {
    FILE *in;
    FILE *out;
    char *outputPath = NULL;

    int tabstop = 8;

    int opt_t = 0;
    int opt_o = 0;
    int c;

    while ((c = getopt(argc, argv, "t:o:")) != -1) {
        switch (c) {
            case 't':
                opt_t++;
                if (opt_t > 1) {
                    fprintf(stderr, "%s: option -t allowed at most once\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                tabstop = strtol(optarg, NULL, 10);
                if (tabstop <= 0) {
                    fprintf(stderr, "%s: tabstop must be an arbitrary positive number greater than 0\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                opt_o++;
                if (opt_o > 1) {
                    fprintf(stderr, "%s: option -o allowed at most once\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                outputPath = malloc(strlen(optarg));
                strcpy(outputPath, optarg);
                break;
            case '?':
                fprintf(stderr, "%s: invalid option\n", argv[0]);
                exit(EXIT_FAILURE);
                break;
            default:
                assert(0);
                break;
        }
    }

    //check whether an output path is given or print to stdout
    if (opt_o == 1) {
        out = fopen(outputPath, "w");
        if (out == NULL) {
            fprintf(stderr, "%s: fopen() failed\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    } else {
        out = stdout;
    }

    //input files given
    if (argc - optind + 1 > 0) {
        for (int i = 0; i < argc - optind; i++) {
            in = fopen(argv[optind + i], "r");
            if (in == NULL) {
                fprintf(stderr, "%s:  fopen() failed\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            char c;
            char lastChar = fgetc(in);

            //parse input file given character by character
            while (lastChar != EOF) {
                int lastIndex = -1;
                c = lastChar;
                while (c != '\n') {
                    if (c == '\t') {
                        lastIndex++;
                        int p = tabstop * ((lastIndex / tabstop) + 1);
                        for (int i = 0; i < p - lastIndex; i++) {
                            fputc(' ', out);
                        }
                        lastIndex = p - 1;
                    } else {
                        lastIndex++;
                        fputc(c, out);
                    }
                    c = fgetc(in);
                }
                fputc('\n', out);
                lastChar = fgetc(in);
            }
            fclose(in);
        }
    }

    //no input files given - read from stdin until \n given
    if (argc == optind) {
        in = stdin;
        char c;
        char lastChar = fgetc(in);

        while (lastChar != '\n') {
            int lastIndex = -1;
            c = lastChar;
            while (c != '\n') {
                if (c == '\t') {
                    lastIndex++;
                    int p = tabstop * ((lastIndex / tabstop) + 1);
                    for (int i = 0; i < p - lastIndex; i++) {
                        fputc(' ', out);
                    }
                    lastIndex = p - 1;
                } else {
                    lastIndex++;
                    fputc(c, out);
                }
                c = fgetc(in);
            }
            fputc('\n', out);
            lastChar = fgetc(in);
        }
        fclose(in);
    }

    //free resources
    free(outputPath);
    fclose(out);
    return 0;
}



