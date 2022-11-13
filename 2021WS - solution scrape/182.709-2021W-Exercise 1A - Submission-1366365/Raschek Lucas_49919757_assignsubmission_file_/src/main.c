/**
 * @file main.c
 * @author Lucas Raschek <e11939549@student.tuwien.ac.at>
 * @date 10.11.2021
 *
 * @brief Program to compress content
 *
 * This program compresses a given content with an simple algorithm. This content can be given in form of file(s) or as stdin.
 * The algorithm converts this string "aaaaabbb" to "a5b3". For each char it counts the number of times it occurs in succession.
 * The compressed output can be written to file with the option [-o file] or to stdout if this option is absent.
 *
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>


static char *prog_name; /**< Program name saved for error messages*/
static unsigned long long int read_chars = 0; /**< Track read chars for compression ratio*/
static unsigned long long int written_chars = 0; /**< Track written chars for compression ratio*/

/**
 * Prints usage information and exits with an error.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: prog_name
 */
static void usage(void) {
    (void) fprintf(stderr, "USAGE: %s [-o outfile] [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}


/**
 *
 * @brief Take the current_char + char_count and write it to output_stream.
 * @details Method should only be called, when there is no current_char following anymore.
 *
 * @param current_char variable that tracks the currently counted char
 * @param char_count variable that counts how often "current_char" has occurred
 * @param output_stream stream where the compressed char + count combo should be written
 * @details global variables: prog_name, written_chars
 */
static void print_compressed_char_to_stream(const char *current_char, const unsigned long long int *char_count,
                                            FILE *output_stream) {
    char *out_string;
    // size 22 = 1 + 20 + 1. 1 for the char, 20 for the max chars an unsigned long long int could have and 1 for \0 char to end the string
    out_string = malloc(sizeof(char) * (22));
    *out_string = *current_char;
    sprintf((out_string + 1), "%llu", *char_count);
    written_chars = written_chars + strnlen(out_string, 21);
    while (fputs(out_string, output_stream) == EOF) {
        if (errno != EINTR) {
            free(out_string);
            fprintf(stderr, "[%s] ERROR: Failed to write to output: %s\n", prog_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    free(out_string);
}

/**
 *
 * @brief compresses a sequence of chars and output it to output_stream
 *
 * @param current_char variable that tracks the currently counted char
 * @param char_count variable that counts how often "current_char" has occurred
 * @param sequence the current sequence, that is being compressed
 * @param output_stream stream where the compressed char + count combo should be written
 * @details global variables: read_chars
 */
static void
compress(char *current_char, unsigned long long int *char_count, const char *sequence, FILE *output_stream) {
    int offset = 0;
    char char_from_line;
    while ((char_from_line = *(sequence + offset++)) != '\0') {
        read_chars++;
        // When there isn't a char that is currently counted (e.g. first char of the file)
        if (*current_char == '\0') {
            *current_char = char_from_line;
            *char_count = 1;
            continue;
        }

        if (char_from_line != *current_char) {
            print_compressed_char_to_stream(current_char, char_count, output_stream);
            *current_char = char_from_line;
            *char_count = 1;
        } else {
            *char_count = *char_count + 1;
        }
    }
}


/**
 * Program entry point.
 * @brief Program compresses content. In this function the arguments are parsed, output file/stream is chosen/opened,
 *        input files/stream is chosen and in a loop the content is split into chunks and compressed piece by piece.
 *        At the end the streams are closed and statistic is printed to stderr.
 * @details global variables: prog_name
 * @param argc The argument counter
 * @param argv The argument vector
 * @return Returns EXIT_SUCCESS
 */
int main(int argc, char **argv) {
    prog_name = argv[0];
    char *output_file = NULL;
    int option_char;
    // Parse options
    while ((option_char = getopt(argc, argv, "o:")) != -1) {
        switch (option_char) {
            case 'o':
                output_file = optarg;
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
        }
    }
    // Get input files
    int currentInputFileIndex = 0;
    int inputFilesCount = argc - optind;
    char **inputFiles = &argv[optind];


    // init output stream
    FILE *output_stream;
    if (output_file == NULL) {
        output_stream = stdout;
    } else {
        while ((output_stream = fopen(output_file, "w")) == NULL) {
            if (errno != EINTR) {
                fprintf(stderr, "[%s] ERROR: Failed to open file %s: %s\n", prog_name, output_file, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }

    // compression loop
    FILE *currentInputStream;
    do {
        // init input stream
        if (inputFilesCount == 0) {
            currentInputStream = stdin;
        } else {
            while ((currentInputStream = fopen(inputFiles[currentInputFileIndex], "r")) == NULL) {
                if (errno != EINTR) {
                    fprintf(stderr, "[%s] ERROR: Failed to open file %s: %s\n", prog_name,
                            inputFiles[currentInputFileIndex], strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
        }
        char buffer[1024];
        char current_char = '\0';
        unsigned long long int char_count = 0;
        while (fgets(buffer, sizeof(buffer), currentInputStream) != NULL) {
            compress(&current_char, &char_count, (char *) buffer, output_stream);
        }
        // Error when reading
        if (ferror(currentInputStream)) {
            fprintf(stderr, "[%s] ERROR: There was an error when reading from the stream with the number %d: %s\n",
                    prog_name,
                    fileno(currentInputStream), strerror(errno));
            exit(EXIT_FAILURE);
        }
        // End of file was reached
        if (feof(currentInputStream)) {
            print_compressed_char_to_stream(&current_char, &char_count, output_stream);
        }
        // close input stream
        if (fclose(currentInputStream) == EOF) {
            fprintf(stderr, "[%s] ERROR: There was an error when closing the stream with the number %d: %s\n",
                    prog_name,
                    fileno(currentInputStream), strerror(errno));
        }
        currentInputFileIndex++;
        //Repeat process for each file (if multiple given)
    } while ((inputFilesCount != 0) && (currentInputFileIndex < inputFilesCount));
    if (fclose(output_stream) == EOF) {
        fprintf(stderr, "[%s] ERROR: There was an error when closing the output stream with the number %d: %s\n",
                prog_name,
                fileno(output_stream), strerror(errno));
    }
    fprintf(stderr, "\nRead:\t\t %llu characters\n", read_chars);
    fprintf(stderr, "Written:\t %llu characters\n", written_chars);
    long double compression_ratio;
    if (read_chars == 0) {
        compression_ratio = 0;
    } else {
        compression_ratio = (long double) (((long double) written_chars) / read_chars) * 100;
    }
    fprintf(stderr, "Compression ratio: %.1Lf%%\n", compression_ratio);
    return EXIT_SUCCESS;
}





