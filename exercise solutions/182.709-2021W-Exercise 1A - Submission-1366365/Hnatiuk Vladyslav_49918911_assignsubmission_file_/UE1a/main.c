/*
 * @author Vladyslav Hnatiuk(01613669)
 * @brief  Data Compressor
 * @details Compresses the data either from input files or stdin and writes it to output file or stdout
 * @date  13-11-2021
 * */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

/*
 * @brief compress data from input file and write them to output file
 * @param input_file input file
 * @param output_file output file
 * @param read_chars count how many characters are read
 * @param written_chars count how many characters are written
 * */
int compress(FILE *input_file, FILE *output_file, unsigned long *read_chars, unsigned  long *written_chars) {
    int buffer_length = 1024;
    char buffer[buffer_length];
    memset(buffer, 0, buffer_length);

    char *file_line = NULL;
    int end_of_file = 0;
    char last_char = '\0';
    int last_char_count = 1;
    char value[12]; // unsigned long is up to 10 chars long, 1 for last_char and 1 for string ending
    while(end_of_file == 0) {
        file_line = fgets(buffer, buffer_length, input_file);

        if (file_line == NULL) {
            if (last_char != '\0') {
                // write last value
                memset(value, 0, 12);
                if (snprintf(value, 12, "%c%d", last_char, last_char_count) < 0) {
                    printf("snprintf error\n");
                    return EXIT_FAILURE;
                }
                *written_chars += strlen(value);
                if (fputs(value, output_file) == EOF) {
                    printf("fputs error\n");
                    return EXIT_FAILURE;
                }
            }
            end_of_file = 1;
            continue;
        }

        *read_chars += strlen(file_line);

        for (int i=0; i < strlen(file_line); i++) {
            if (file_line[i] == last_char) {
                last_char_count++;
            } else {
                // handle first iteration separately
                if (last_char_count == 0) {
                    last_char = file_line[i];
                    last_char_count = 1;
                } else {
                    memset(value, 0, 12);
                    if (snprintf(value, 12, "%c%d", last_char, last_char_count) < 0) {
                        printf("snprintf error\n");
                        return EXIT_FAILURE;
                    }
                    *written_chars += strlen(value);
                    if (fputs(value, output_file) == EOF) {
                        printf("fputs error\n");
                        return EXIT_FAILURE;
                    }
                    last_char = file_line[i];
                    last_char_count = 1;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

int main(int argc, char **argv) {
    int return_value = EXIT_SUCCESS;
    char *output_file = NULL;
    char **input_files = NULL;
    int input_files_count = 0;

    int opt = 0;
    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                output_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [-o ouput_file] [file1] [file2] [..] \n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (argc > optind) {
        input_files_count = argc - optind;
        input_files = malloc(sizeof(char) * input_files_count);
        if (input_files == NULL) {
            fprintf(stderr, "Failed to allocate memory for input file list");
            exit(EXIT_FAILURE);
        }
        for (int i=0; i < input_files_count; i++) {
            input_files[i] = argv[optind + i];
        }
    }

    int output_to_file = 0;
    if (output_file != NULL) {
        output_to_file = 1;
    }

    FILE *input_file_pointer = NULL;
    FILE *output_file_pointer = NULL;
    unsigned long read_chars = 0;
    unsigned long written_chars = 0;

    if (output_to_file == 1) {
        output_file_pointer = fopen(output_file, "w");
        if (output_file_pointer == NULL) {
            printf("Cannot open file %s", output_file);

            if (input_files_count > 0) {
                free(input_files);
            }
            exit(EXIT_FAILURE);
        }
    } else {
        output_file_pointer = stdout;
    }

    if (input_files_count > 0) {
        for (int i = 0; i < input_files_count; i++) {
            input_file_pointer = fopen(input_files[i], "r");
            if (input_file_pointer == NULL) {
                printf("Cannot open file %s", input_files[i]);
                if (output_to_file == 1) {
                    free(output_file_pointer);
                }
                goto end;
            }
            if (compress(input_file_pointer, output_file_pointer, &read_chars, &written_chars) != EXIT_SUCCESS) {
                fclose(input_file_pointer);
                return_value = EXIT_FAILURE;
                goto end;
            }
            fclose(input_file_pointer);
        }
    } else {
        if (compress(stdin, output_file_pointer, &read_chars, &written_chars) != EXIT_SUCCESS) {
            return_value = EXIT_FAILURE;
            goto end;
        }
    }

    double compression_ratio = round(((double)written_chars / (double)read_chars) * 1000) / 10.0;
    fprintf( stderr, "\nRead:      %lu characters\n", read_chars);
    fprintf( stderr, "Written:   %lu characters\n", written_chars);
    fprintf( stderr, "Compression ratio: %.1f%%\n", compression_ratio);

end:
    if (input_files != NULL) {
        free(input_files);
    }
    if (output_to_file == 1 && output_file_pointer != NULL) {
        fclose(output_file_pointer);
    }

    return return_value;
}
