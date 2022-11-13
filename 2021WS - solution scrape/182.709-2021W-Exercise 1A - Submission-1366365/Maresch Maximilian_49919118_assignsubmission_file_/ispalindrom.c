/**
 * @author Maximilian Maresch
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "palindrom_algorithm.h"

/**
 * @brief Prints the usage message for this program
 * @detail Prints the usage message to stderr which shows how to correctly use this program
 * @param argv - argv from main
 */ 
void usage(char *argv[]) {
    fprintf(stderr,"[%s] Usage: ispalindrom [-s] [-i] [-o outfile] [file...]\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    char *o_arg = NULL;
    int opt_s = 0;
    int opt_i = 0;
    int c;
    while ((c = getopt(argc, argv, "sio:")) != -1) {
        switch (c) {
            case 's': opt_s++;
                break;
            case 'i': opt_i++;
                break;
            case 'o': o_arg = optarg;
                break;
            case '?':
                usage(argv);
                break;
            default:
                assert(0);
                break;
        }
    }

    while (true) {
        char *input_orig = NULL;

        ssize_t lineSize = 0;
        if (o_arg == NULL && (argc - optind) == 0) {
            size_t len = 0;
            lineSize = getline(&input_orig, &len, stdin);
        }

        if ( (argc - optind) > 0 ) {
            char* files[argc - optind];

            for (int i = 0; i < (argc - optind); i++) {
                files[i] = argv[optind + i];
            }

            FILE *out;

            if (o_arg != NULL) {
                if ((out = fopen(o_arg, "w")) == NULL) {
                    fprintf(stderr, "[%s] fopen failed: %s\n", argv[0], strerror(errno));
                    break;
                }
            }
            
            for (int i = 0; i < (argc - optind); i++) {
                char buffer[1024];
                FILE *in;

                if ((in = fopen(files[i], "r")) == NULL) {
                    fprintf(stderr, "[%s] fopen failed: %s\n", argv[0], strerror(errno));
                    break;
                }
                
                while (fgets(buffer, sizeof(buffer), in) != NULL) { 

                    if (buffer[strlen(buffer) - 1] == '\n') {
                        buffer[strlen(buffer) - 1] = '\0';
                    }

                    char* result = ispalindrom(buffer, strlen(buffer) + 1, opt_i, opt_s);

                    if (o_arg == NULL) {
                        printf("%s%s", buffer, result);
                    } else {
                        char *line = malloc(strlen(buffer) + strlen(result) + 1);
                
                        if (line == NULL) {
                            fprintf(stderr, "[%s] malloc failed: %s\n", argv[0], strerror(errno));
                            break;
                        }

                        strcpy(line, buffer);
                        strcat(line, result);

                        if (fputs(line, out) == EOF) {
                            fprintf(stderr, "[%s] fputs failed: %s\n", argv[0], strerror(errno));
                            break;
                        }

                        free(line);
                    }
                }
                
                if (ferror(in)) {
                    fprintf(stderr, "[%s] fgets failed: %s\n", argv[0], strerror(errno));
                    break;
                }

                fclose(in);
            }

            if (o_arg != NULL) {
                fclose(out);
            }

            return 0;
        }

        if (input_orig[lineSize - 1] == '\n') {
            input_orig[lineSize - 1] = '\0';
        }

        char *input = NULL;
        input = strdup(input_orig);

        char* result = ispalindrom(input, lineSize, opt_i, opt_s);

        FILE *out;

        if (o_arg != NULL) {
            if ((out = fopen(o_arg, "w")) == NULL) {
                fprintf(stderr, "[%s] fopen failed: %s\n", argv[0], strerror(errno));
                break;
            }

            char *line = malloc(strlen(input_orig) + strlen(result) + 1);

            if (line == NULL) {
                fprintf(stderr, "[%s] malloc failed: %s\n", argv[0], strerror(errno));
                break;
            }

            strcpy(line, input_orig);
            strcat(line, result);

            if (fputs(line, out) == EOF) {
                fprintf(stderr, "[%s] fputs failed: %s\n", argv[0], strerror(errno));
                break;
            }

            free(line);
            fclose(out);
        } else {
            printf("%s%s", input_orig, result);
        }

        free(input);
    }

    return 0;
}