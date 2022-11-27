/*  myexpand.c
    @author Filip Rozana, 12024732
    @brief This program is used to convert tabstops into spaces
    @details The program takes files (or stdin if no file given) and changes all the tabstops into the defined
    amount of spaces. The output is written into a file (or stdout if no file given). 
    @date 11.14.2001
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

char *prog_name;

/*  @brief Describes the usage of the program
    @details Prints an error message containing the name of the program and the usage of it. Exits with an failure exit code
    @param void
    @return void
 */
void usage(void);
void usage(void) {
    fprintf(stderr, "Usage: %s [-t tabstop] [-o outfile] [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}

/*  @brief Takes two files and replaces the tabstops with spaces
    @param file_input this file contains the information 
    @param file_output this file gets modified
    @return void
 */
void myexpand_on_file(FILE *file_input, FILE *file_output, int spaces) {
    char c;
    int spaces_left;
    int pos_line = 0;
	while ((c = fgetc(file_input)) != EOF) {
        switch (c) {
            case '\t':
                spaces_left = (spaces * ((pos_line / spaces) + 1) - pos_line);
                for (int i = 0; i < spaces_left; i++)
                {
                    fprintf(file_output, " ");
                    pos_line ++;
                }
                break;
            case '\n':
                pos_line = 0;
                fprintf(file_output, "%c", c);
                break;
            default:
                pos_line ++;
                fprintf(file_output, "%c", c);
                break;
        }
    }
    fprintf(file_output, "\n");
}

int main(int argc, char *argv[])
{
    prog_name = argv[0];
    char *o_arg = NULL;
    int spaces = 8;
    int c;
    while ((c = getopt(argc, argv, ":t:o:")) != -1) {
        switch (c) {
            case 't':
                spaces = (int) strtol(optarg, (char **)NULL, 10);
                if (spaces <= 0) {
                    fprintf(stderr, "value of -t is invalid\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'o':
                o_arg = optarg;
                break;
            case ':':
                fprintf(stderr, "[%s] ERROR: option -%c is missing a required argument\n", prog_name, (char) optopt);
                usage();
                break;
            case '?':
                fprintf(stderr, "[%s] ERROR: Unknown option -%c.\n", prog_name, (char) optopt);
                usage();
                break;
            default:
                usage();
        }
    }

    // file to written into
    FILE *output_file;
    if (o_arg != NULL) {
        if ((output_file = fopen(o_arg, "w+")) == NULL) {
            fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        output_file = stdout;
    }

    // myexpand
    int files_to_read = argc - optind;
    FILE *input_file;
    if (optind == argc) {   // no file given, so stdin is used
        myexpand_on_file(stdin, output_file, spaces);
    } else {                // iterate trough files
        for (int i = 0; i < files_to_read; i++) {
            if ((input_file = fopen(argv[optind + i], "r")) == NULL) {
                fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", prog_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            myexpand_on_file(input_file, output_file, spaces);
            fclose(input_file);
        }
    }

    fclose(output_file);

    return 0;
}
