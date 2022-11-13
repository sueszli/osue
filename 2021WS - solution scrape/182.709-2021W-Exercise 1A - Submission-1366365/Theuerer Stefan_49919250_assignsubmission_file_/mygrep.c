/**
 * @file mygrep.c
 * @author Stefan Theuerer <e01228985@student.tuwien.ac.at>
 * @date 12.11.2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

char *myprog; /**< The program name */

/**
 * @brief prints the correct usage of the programm (mygrep)
 * @details global variables: myprog
 */
void usage(void) {
    (void) fprintf(stderr,"Usage: %s [-i] [-o outfile] keyword [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief converts any given string to a lower case string
 * @param any given string
 * @return a lower case string
 */
static char *toLowerCase(char *string) {

    for(int i = 0; string[i] != '\0'; i++){
        string[i] = tolower(string[i]);
    }
    return string;
}

/**
 * @brief checks if a given keyword is within a string
 * @param any given string
 * @return 1 if the keyword is a substring
 *         -1 if not a substring
 */
static int contains(char *string, char *keyword) {

    if(strstr(string, keyword) != NULL) {
        return 1;
    }
    return -1;
}

/**
 * @brief Reduced variton of the unix-command 'grep'
 * @details enter any stdin or files, check if a 
 *          key is within a string. Print solution 
 *          in stdout or a specific file
 * @param argc number of parameters
 * @param argv array of parameters
 * @param [-i] case insensitve
 * @param [-o outfile] output in specific file
 * @param keyword to look for
 * @param [files...] multiple input files
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char **argv) {
    myprog = argv[0];
    FILE *input = stdin;
    FILE *output = stdout;
    char *arg_o = NULL;
    char *keyword = NULL;
    int ch, opt_i,opt_o;
    int number_of_input_files;

    while ((ch = getopt(argc, argv,"io:")) != -1) {
        switch (ch)
        {
        case 'i':
            opt_i = 1;
            break;
        case 'o':
            opt_o = 1;
            arg_o = optarg;
            break;
        case '?':
            usage();
        default:
            assert(0);
        }
    }
    keyword = argv[optind];
    if(keyword == NULL) {
        usage();
    }
    number_of_input_files = (argc - (optind+1));

    if(opt_o == 1) {
        if((output = fopen(arg_o, "w")) == NULL) {
            (void) fprintf(stderr, "Unable to open %s\n", arg_o);
            fclose(output);
            fclose(input);
            exit(EXIT_FAILURE);
        }
    }

    char *line = NULL;
    size_t len = 0;
    size_t line_size;

    if(number_of_input_files > 0) {
        for(int i = 1; i <= number_of_input_files; i++) {
            input = fopen(argv[optind + i], "r");
            if(input == NULL) {
                (void) fprintf(stderr, "Unable to open %s\n", argv[optind+i]);
                continue;
            }

            while((line_size = getline(&line, &len, input)) != -1) {
                if((line_size-1) != 0) {
                    // save original line input and overwrite "\n"
                    int size = strlen(line);
                    char *original = malloc(size);
                    strcpy(original,line);
                    original[size-1] = '\0';

                    if(opt_i == 1) {
                        keyword = toLowerCase(keyword);
                        line = toLowerCase(line);
                    }
                    if(contains(line, keyword) == 1) {
                        (void) fprintf(output,"%s\n",original);
                    }
                    free(original);
                }   
            }
            fclose(input);
        }
    } else {
        while((line_size = getline(&line, &len, input)) != -1) {
            if((line_size-1) != 0) {
                // save original line input and overwrite "\n"
                int size = strlen(line);
                char *original = malloc(size);
                strcpy(original,line);
                original[size-1] = '\0';

                if(opt_i == 1) {
                    keyword = toLowerCase(keyword);
                    line = toLowerCase(line);
                }
                if(contains(line, keyword) == 1) {
                    (void) fprintf(output,"%s\n",original);
                }

                free(original);
            }   
        }
        fclose(input);
    }
    return EXIT_SUCCESS;
}
