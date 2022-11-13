/**
 * myexpand.c
 * 
 * @author Luis Henrique Paoletti 11929823
 * @brief Source file for myexpand.exe executable
 * @date 12.11.2021
 */

#include "myexpand.h"

/**
 * @brief main function of the source file
 * @param argc Number of arguments
 * @param argv Array of arguments
 * @return int (0 or 1)
 */
int main(int argc, char *argv[]) {

    char *myprogram = argv[0];
    char *outfile = NULL;
    int tabstop = 8;
    
    int c;
    int t_count = 0;
    int o_count = 0;
    char *t_arg = NULL;
    char *o_arg = NULL;

    while ( (c = getopt(argc, argv, "t:o:")) != -1 ) {
        switch (c) {
            case 't':
                t_arg = optarg;
                t_count++;
                break;
            case 'o':
                o_arg = optarg;
                o_count++;
                break;
            case '?': usage("\n%s [-t tabstop (positive int)] [-o outfile (\"str\".\"txt\")] [files...] \n", myprogram);
                break;
        }
    }


    if (t_count > 1) {
        usage("Flag -t in %s should be set maximum once \n", myprogram);
    } else if (t_count == 1) {
        if (t_arg == NULL) {
            usage("Flag -t in %s requires an integer argument \n", myprogram);
        } else {
            int i;
            if ((i = (int) strtol(t_arg, NULL, 10)) > 0) {
                tabstop = i;
            } else {
                usage("\n%s [-t tabstop (positive int)] [-o outfile (\"str\".\"txt\")] [files...] \n", myprogram);
            }

            if (errno == ERANGE) {
                error("\n[%s] ERROR: tabstop out of range: %s\n", myprogram);
            }
        }
    }

    if (o_count > 1) {
        usage("Flag -o in %s should be set maximum once \n", myprogram);
    } else if (o_count == 1) {
        if (o_arg == NULL) {
            usage("Flag -o in %s requires a string argument \n", myprogram);
        } else {
            char *substring = NULL;
            if ((substring = strstr(o_arg, ".txt")) != NULL) {
                outfile = o_arg;
            } else {
                usage("\n%s [-t tabstop (positive int)] [-o outfile (\"str\".\"txt\")] [files...] \n", myprogram);
            }
        }
    }
    

    if (argc - optind == 0) {

        char *string = NULL;
        size_t length = 0;
        
        printf("\nPlease type in your input: ");

        getline(&string, &length, stdin);
        if (errno == EINVAL || errno == ENOMEM) {
            error("\n[%s] ERROR: getline failed: %s\n", myprogram);
        }

        char *new_string = tabulate(tabstop, string);

        if (outfile == NULL) {
            printf("\n%s", new_string);
        } else {
            
            FILE *file = fopen(outfile, "w");
            if (file == NULL) {
                error("\n[%s] ERROR: fopen failed: %s\n", myprogram);
            }

            if (fwrite(new_string, sizeof(char), strlen(new_string), file) != strlen(new_string)) {
                error("\n[%s] ERROR: fwrite failed: %s\n", myprogram);
            }
            
            fclose(file);
        }
        
        free(string);   // getstring() allocates memory for string

    } else {            // argc - optind > 0

        for (int i = optind; i < argc; i++) {

            char *substring = NULL;
            if ((substring = strstr(argv[i], ".txt")) != NULL) {
                
                printf("\nThe opening of the file throws the following error, which I don't know how to solve (input via stdin still works though):");
                // Somehow this next line throws the error "Segmentation fault"
                FILE *file = fopen(argv[i], "r");
                if (file == NULL) {
                    error("\n[%s] ERROR: fopen failed: %s\n", myprogram);
                }
                
                if (outfile == NULL) {
                    tabulateFiles(tabstop, myprogram, file, NULL);
                } else {
                    FILE *file_out = fopen(outfile, "w");
                    if (file_out == NULL) {
                        error("\n[%s] ERROR: fopen failed: %s\n", myprogram);
                    }

                    tabulateFiles(tabstop, myprogram, file, file_out);

                    fclose(file_out);
                }

                fclose(file);
            } else {
                usage("\n%s [-t tabstop (positive int)] [-o outfile (\"str\".\"txt\")] [files...] \n", myprogram);
            }
        }
    }
    

    printf("\n");
    return EXIT_SUCCESS;
}


void usage(const char *const formated_message, const char *const program) {
    fprintf(stderr, formated_message, program);
    exit(EXIT_FAILURE);
}

void error(const char *const formated_message, const char *const program) {
    fprintf(stderr, formated_message, program, strerror(errno));
    exit(EXIT_FAILURE);
}

char *tabulate(int tab_size, char *const string) {
    int tab_count = 0;
    int next_pos = 0;
    int offset = 0;

    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '\t') {
            tab_count++;
        } else if (string[i] == '\n') {
            string[i] = '\0';
        }
    }

    char *new_string = malloc((strlen(string) + tab_count * tab_size) * sizeof(char));

    for (int i = 0; i <= strlen(string); i++) {     // operator <= is used because we want to copy the \0 character as well
        if (string[i] == '\t') {
            next_pos = tab_size * (((i + offset) / tab_size) + 1);
            for (int j = i + offset; j < next_pos; j++) {
                new_string[j] = ' ';
            }
            offset = ((next_pos - 1) - i);
        } else {
            new_string[i + offset] = string[i];
        }
    }

    return new_string;
}

void tabulateFiles(int tab_size, const char *program, FILE *file_in, FILE *file_out) {
    char *string = NULL;
    while (fgets(string, __INT16_MAX__, file_in) != NULL) {

        char *new_string = tabulate(tab_size, string);
        
        if (file_out == NULL) {
            printf("\n%s", new_string);
        } else {
            new_string = strcat(new_string, "\n");

            if (fwrite(new_string, sizeof(char), strlen(new_string), file_out) != strlen(new_string)) {
                error("\n[%s] ERROR: fwrite failed: %s\n", program);
            }
        }
    }
}