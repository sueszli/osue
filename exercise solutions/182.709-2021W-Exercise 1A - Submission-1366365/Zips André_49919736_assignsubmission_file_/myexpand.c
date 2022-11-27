/**
 * @file myexpand.c
 * @author Andre Zips 11811363 
 * @brief Raplace all tabs with space in file
 * @date 2021-11-07
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

char *app;

/**
 * @brief Prints the usage documentatio if and invalid input was recognised
 * 
 */
void usage(void){
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", app);
    exit(EXIT_FAILURE);
}

/**
 * @brief replaces all tabs with space in the given File and outputs it to out.
 * 
 * @param input the input file to replace the tabs
 * @param output the output file or stdout with the replaced tabs
 * @param tabstop the length of a tab defined
 * @return int exit code
 */
static int replace(FILE *input, FILE *output, int tabstop){
    char buffer[1024];
    //fprintf(stdout, "tabstop: %i\n", tabstop);
    //while go through all rows until ther is non --> NULL
    while (fgets(buffer, sizeof(buffer), input) != NULL){
        int tabsreplaced = 0;
        // i is the position in row
        for (int i = 0; i < strlen(buffer); i++){
            if(buffer[i] == '\t') {
                int p = tabstop * (((i+tabsreplaced) / tabstop) + 1);
                //fprintf(stdout, "p: %d, i = %i\n", p, i);
                for (int spacecount = 0; spacecount < p; spacecount++){
                    fputc(' ', output);
                }
                tabsreplaced += p; 
            }else{
                fputc(buffer[i], output);
            } 
        }
    }
    return EXIT_SUCCESS;
}

/**
 * @brief starting point for the myexpand application
 * 
 * @param argc count of the input arguments
 * @param argv array of input arguments with length of argc
 * @return Exit code 
 */
int main(int argc, char * const argv[])
{
    app = argv[0];
    FILE* output = stdout;
    char *_;
    int tabstop = 8, opt;
    while((opt = getopt (argc, argv, "t:o:")) != -1){
        switch (opt){
            case 't':
                tabstop = strtol(optarg, &_, 10);
                if(!strcmp(optarg, _) || tabstop <= 0) {
                        usage();
                }
                break;
            case 'o': 
                output = fopen(optarg, "w");
                if(output == NULL){
                    usage();
                }
                break;
            default:
                break;
        }
    }
    //fprintf(stdout, "Optind: %i, argc: %i\n", optind, argc);
    if(optind == argc){
        replace(stdin, output, tabstop);
    }
    while(optind < argc) {
        FILE* input = fopen(argv[optind], "r");
        if(input == NULL){
            fclose(output);
            usage();
        }
        if(replace(input, output, tabstop) == EXIT_FAILURE){
            fclose(input);
            fclose(output);
            return EXIT_FAILURE;
        }
        optind++;
    }
    fclose(output);
    return EXIT_SUCCESS;
}




