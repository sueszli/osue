/**
 * @file: myexpand.c
 * @author: Markovic Predrag (11810372)
 * @date: 2.11.2021
 * @brief: myexpand Source File
 * @details: source code for the expand program to replace tabs with spaces
**/

#include <errno.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

/*char* replaceTabsWithSpaces(char* input, int tabStop_distance) {
    char* buf = malloc(sizeof(char) * strlen(input));
    strcpy(buf, input);

    char* tab_offset = NULL;
    while((tab_offset = strchr(buf, '\t')) != NULL) {
        int tab_ind = (int)(tab_offset - buf);
        int spaceStringEnd_ind = (tabStop_distance * ((tab_ind / tabStop_distance) + 1));
        int nrOfSpacesToReplaceTab = spaceStringEnd_ind - tab_ind;
        int nrOfBytesToExtendBuf = nrOfSpacesToReplaceTab - 1;
        char textAfterTab[strlen((char*)(tab_offset + 1))];
        strcpy(textAfterTab, (char*)(tab_offset + 1));

        // Realloc buf
        char* newBufptr = realloc(buf, sizeof(buf) + nrOfBytesToExtendBuf);
        if(newBufptr == NULL) {
            fprintf(stderr, "error during replacing tabs with spaces: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        buf = newBufptr;
        for(int i = tab_ind; i < tab_ind + nrOfSpacesToReplaceTab; i++) {
            buf[i] = ' ';
        }
        int j = 0;
        for(int i = spaceStringEnd_ind; i < strlen(buf); i++) {
            buf[i] = textAfterTab[j];
            j++;
        }
    }

    return buf;
}*/


int main(int argc, char const *argv[])
{
    int t_arg_cnt = 0;
    int o_arg_cnt = 0;
    int tabStop_distance = 8;

    char* t_arg = NULL;
    char* o_arg = NULL;

    char c;

    while((c = getopt(argc, argv, "t:o:")) != -1) {
        if(c == 't') {
            t_arg_cnt++;
            t_arg = optarg;
        } else if (c == 'o') {
            o_arg_cnt++;
            o_arg = optarg;
        } else if (c == '?') {
            fprintf(stderr, "./myexpand: Correct synopsis: myexpand [-t tabstop] [-o outfile] [file ...]\n");
            exit(EXIT_FAILURE);
        } else {
            fprintf(stderr, "./myexpand: Correct synopsis: myexpand [-t tabstop] [-o outfile] [file ...]\n");
            exit(EXIT_FAILURE);
        }
    }

    if(t_arg_cnt > 1) {
        fprintf(stderr, "./myexpand: Correct synopsis: myexpand [-t tabstop] [-o outfile] [file ...]\n");
        exit(EXIT_FAILURE);
    } else if (o_arg_cnt > 1) {
        fprintf(stderr, "./myexpand: Correct synopsis: myexpand [-t tabstop] [-o outfile] [file ...]\n");
        exit(EXIT_FAILURE);
    }

    char* t_arg_str;

    if(t_arg_cnt == 1) {
        int t_arg_num = strtol(t_arg, &t_arg_str, 10);

        if(strlen(t_arg_str) != 0 || t_arg_num <= 0) {
            fprintf(stderr, "-t has to be provided a positive numerical value greater zero\n");
            fprintf(stderr, "./myexpand: Correct synopsis: myexpand [-t tabstop] [-o outfile] [file ...]\n");
            exit(EXIT_FAILURE);
        }

        tabStop_distance = t_arg_num;
    }

   /* if(argc - optind > 0) {
        // Read lines from input files
        // Note: optind implicitely sorts indices option-arguments and positional arguments
        // by putting option-args first and positional args last.
        // This in turn determines optind accordingly

        for(int i = 0; i < argc - optind; i++) {
            char* fileName = argv[optind + i];

            FILE* inputFile = fopen(fileName, "r");
            if(inputFile == NULL) {
                fprintf(stderr, "./myexpand: fopen failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            char* line = NULL;
            int line_buf_size = 0;
            
            while(getline(&line, &line_buf_size, inputFile) >= 0) {

                    if(o_arg_cnt == 1) {
                        char* outputFileName = o_arg;
                        FILE* outputFile = fopen(outputFileName, "w");
                        if(outputFile == NULL) {
                            fprintf(stderr, "./myexpand: fopen failed: %s\n", strerror(errno));
                            exit(EXIT_FAILURE);
                        }
                        //fputs(replaceTabsWithSpaces(line, tabStop_distance), outputFile);
                        fputs(line, outputFile);

                    } else {
                        fputs(line, stdout);
                    }
            }
            fclose(inputFile);
        }
    } else {


    }*/

    

    return 0;
}
