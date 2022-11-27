/**
 * @file mycompress.c
 * @author Istvan Kiss (istvan.kiss@student.tuwien.ac.at)
 * @brief Implementation of the mycompress program.
 * @version 0.1
 * @date 2021-11-08
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/**
 * @brief default buffer size macro
 * 
 */
#define BUFFER_SIZE 512
/**
 * @brief program name macro
 * 
 */
#define PROG_NAME "mycompress"

/**
 * @brief global variable to store the command the program was executed with
 * 
 */
static char *command;


/**
 * @brief Prints the usage message to stderr
 * @details after printing the program terminates with failiure
 * 
 */
static void usage(void){
    fprintf(stderr, "Usage: %s [-o outfile]  [file...]\n", PROG_NAME);
    exit(EXIT_FAILURE) ;
}

/**
 * @brief Handles program errors with messages
 * @details Prints error message to stderr. If fatal the program terminates after printing message.
 * 
 * global variable: command
 * @param fatal if 1 then program terminates
 * @param message failure message
 */
static void handle_error(int fatal, char* message) {
    if (fatal == 1) {
        fprintf(stderr, "[%s] fatal error: %s: %s \n", command, message, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "[%s] %s: %s \n", command, message, strerror(errno));

}

/**
 * @brief Handles program arguments.
 * @details It reads the program arguments and quits if needed.
 * If there is an outpute file param then it changes stdout to this file.
 * 
 * global variable: command
 * @param argc argument counter
 * @param argv argument variables
 * @param inputFiles input file pointers
 * @param inputCount input file counter
 */
static void handle_args(int argc, char** argv, char*** inputFiles, int* inputCount) {
    int c;
    char* o_arg = NULL;
    command = argv[0];

    while((c = getopt(argc, argv, "o:")) != -1) {
        switch(c) {
            case 'o':
                o_arg = optarg;
                break;
            default:
                usage();
        }
    }

    if (o_arg != NULL) {
        FILE *file;
        if ( (file = freopen(o_arg, "w", stdout)) == NULL) {
            handle_error(1, "freopen on stdout failed");
        }
    }

    if (optind < argc) {
        *inputFiles = &argv[optind];
        *inputCount = argc - optind;
    }
}

/**
 * @brief Compresses a whole file
 * @details Reads every char of src and compresses then the compressed line is copied to dst.
 * If needed it allocates buffer automatically.
 * 
 * @param dst pointer to the destination the compressed text is copied to
 * @param src source of text
 * @param len length of text
 * @return int length of compressed text
 */
static int compress_text(char** dst, char* src, size_t len) {
    int buffer = BUFFER_SIZE;
    if (*dst == NULL) {
        *dst = malloc(buffer * sizeof(char));
        if (*dst == NULL) {
            handle_error(1, "malloc failed");
        }
    }
    if (len == 0) {
        return 0;
    }else if (len == 1) {
        char* c = "1";
        strncpy(*dst, src, 1);
        strncpy(*dst + sizeof(char), c, 1);
        return 2;
    }

    int char_counter = 1;
    int counter = 0;
    //max length of the max integer
    int max_length = (int)((ceil(log10(__INT_MAX__))+1)*sizeof(char));
    for (size_t i = 0; i < len; i++){
        if (counter + max_length + 1 >= buffer) {
            buffer += BUFFER_SIZE;
            *dst = realloc(*dst, (buffer) * sizeof(char));
            if (*dst == NULL) {
                handle_error(1, "realloc failed");
            }
        }

        if (i == 0) {
            strncpy(*dst + counter * sizeof(char), src + i * sizeof(char), 1);
        } else {
            if (strncmp(*dst + counter * sizeof(char), src + i * sizeof(char), 1) == 0) {
                char_counter++;
            } else {
                counter++;
                counter += sprintf(*dst + counter * sizeof(char), "%d", char_counter);
                strncpy(*dst + (counter * sizeof(char)), src + (i * sizeof(char)), 1);
                char_counter = 1;
            }
        }
    }
    strncpy(*dst + (counter * sizeof(char)), src + (len - 1 * sizeof(char)), 1);
    counter++;
    counter += sprintf(*dst + counter * sizeof(char), "%d", char_counter);
    return counter;
}

/**
 * @brief Compresses input
 * @details Reads all chars of the input, after compression it gets printed to stdout.
 * 
 * @param read pointer to number of read chars
 * @param written pointer to number of written chars
 */
static void compress_input(long* read, long* written) {
    int buffer = BUFFER_SIZE;
    int compressed_len = 0;
    char* compressedText = NULL;
    ssize_t textlen = 0;
    int c = 0;
    char* text = malloc(buffer * sizeof(char));

    if (text == NULL) {
        handle_error(1, "malloc failed");
    }

    while((c = fgetc(stdin)) != EOF) {
        if (textlen >= buffer) {
            buffer += BUFFER_SIZE;
            text = realloc(text, buffer * sizeof(char));
            if (text == NULL) {
                handle_error(1, "realloc failed");
            }
        }
        text[textlen] = c;
        textlen++;
    }

    compressed_len = compress_text(&compressedText, text, textlen);
    if (fputs(compressedText, stdout) < 0) {
        handle_error(1, "fprintf failed");
    }
    *read += textlen;
    *written += compressed_len;
    free(compressedText);
    free(text);
}

/**
 * @brief Prints compress ratio.
 * @details It prints to stderr.
 * 
 * @param read number of read characters
 * @param written number of written characters
 */
static void write_ratio(long read, long written) {
    double ratio;
    if (written == 0) {
        ratio = 0;
    } else {
        ratio = ((double)written / (double)read) * 100;
    }
    fprintf(stderr, "Read: %ld characters\n", read);
    fprintf(stderr, "Written: %ld characters\n", written);
    fprintf(stderr, "Compression ratio: %.1f%%\n", ratio);
}

/**
 * @brief function for the main logic
 * @details If there is an input file specified, then it reopens stdin with this file.
 * Otherwise it compresses standard input.
 * 
 * @param argc argcument counter
 * @param argv argument variables
 * @return int 0 on success
 */
int main(int argc, char** argv) {
    int inputCount;
    long read = 0;
    long written = 0;
    char** inputFiles = NULL;
    handle_args(argc, argv, &inputFiles, &inputCount);

    if (inputFiles != NULL) {
        FILE *file;
        for (size_t i = 0; i < inputCount; i++) {
            if ( (file = freopen(inputFiles[i], "r", stdin)) != NULL) {
                compress_input(&read, &written);
                fclose(file);
            } else {
                handle_error(1, inputFiles[i]);
            }
        }
    } else {
        compress_input(&read, &written);
    }
    write_ratio(read, written);
    return EXIT_SUCCESS;
}
