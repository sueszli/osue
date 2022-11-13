/**
 * @file main.c
 * @author Lukas Fink 11911069
 * @date 04.11.2021
 *
 * @brief Main program module.
 * 
 * @details This program compresses input(s) and writes it to an ouput.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "comp.h"

/**
 * Program entry point.
 * @brief The program starts here. It should be called like 'mycompress [-o outfile] [inputFile...]'.
 * @details It can handle an optional outfile as well as several inputFiles.
 * If the option -o together with an outfile is not used, it writes to stdout.
 * If it is called with no input files, it reads from stdin.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 **/
int main (int argc, char *argv[]) {
    char *o_arg = NULL;
    int opt_o = 0;
    int c;
    while ( (c = getopt(argc, argv, "o:")) != -1){
        switch (c) {
            case 'o': 
                opt_o++;
                o_arg = optarg;
                break;
            case '?':
                exit(EXIT_FAILURE);
                break;
            default: 
                assert(0);
                break;
        }
    }
    // option o did occur more than once
    if (opt_o > 1) {
        exit(EXIT_FAILURE);
    }

    char* programName = argv[0];

/** read from the input.
 * In both cases (file(s) or stdin) the same algorithm is used.
 * Algorithm:   reads chars until EOF is reached. (repeats this for every input).
 *              The inputBuffer is dynamically allocated and increased if it is full.
 **/
    int size = 256;
    char *inputBuf = malloc(sizeof(char) * size);

    if (inputBuf == NULL) {
        fprintf(stderr, "%s: out of memory!\n", programName);
        exit(EXIT_FAILURE);
    }

    FILE *input;
    int nextIndex = optind;
    int current;
    int position = 0;
    // there is an input argument => read the input file(s)
    if (nextIndex < argc) {
        while (nextIndex < argc) {
            char* inputFileName = argv[nextIndex];
            input = fopen(inputFileName, "r");
            if (input == NULL) {
                fprintf(stderr, "%s: File cannot be found: %s\n", programName, inputFileName);
                free(inputBuf);
                exit(EXIT_FAILURE);
            }
            while ((current = fgetc(input)) != EOF) {
                if (ferror(input) != 0) {
                    fprintf(stderr, "%s: fgetc failed: %s\n", programName, strerror(errno));
                    fclose(input);
                    free(inputBuf);
                    exit(EXIT_FAILURE);
                }

                if (position >= size-2) {
                    size+=256;
                    inputBuf = realloc(inputBuf, size * sizeof(char));
                    if (inputBuf == NULL) {
                        fprintf(stderr, "%s: out of memory! m\n", programName);
                        free(inputBuf);
                        fclose(input);
                        exit(EXIT_FAILURE);
                    }
                }
                inputBuf[position] = current;
                position++;
            }
            nextIndex++;
        }
    // there is no input argument => read from stdin
    } else {
        input = stdin;
        while ((current = fgetc(input)) != EOF) {
            if (ferror(input) != 0) {
                fprintf(stderr, "%s: fgetc failed: %s\n", programName, strerror(errno));
                free(inputBuf);
                fclose(input);
                exit(EXIT_FAILURE);
            }

            if (position >= size-1) {
                size+=256;
                inputBuf = realloc(inputBuf, size * sizeof(char));
                if (inputBuf == NULL) {
                    fprintf(stderr, "%s: out of memory! m\n", programName);
                    free(inputBuf);
                    fclose(input);
                    exit(EXIT_FAILURE);
                }
            }
            inputBuf[position] = current;
            position++;
        }
    }
    if (input == NULL){
        fprintf(stderr, "%s: fopen failed: %s\n", programName, strerror(errno));
        free(inputBuf);
        exit(EXIT_FAILURE);
    }
    // the Algorithm has finished. The last char to be written is the null byte.
    inputBuf[position] = '\0';

// compress the data
    char *compressed = compress(inputBuf, position, programName);
    if (compressed == NULL) {
        fprintf(stderr, "%s: compression failed!\n", programName);
        free(inputBuf);
        fclose(input);
        exit(EXIT_FAILURE);
    }

// write to the output
    FILE *output;
    if (opt_o == 1) {
        // option o did  occur => write to the output file
        char* outputFileName = o_arg;
        output = fopen(outputFileName, "w");
        if (output == NULL) {
            fprintf(stderr, "%s: fopen failed, outputfile: %s does not exist!\n", programName, outputFileName);
            free(inputBuf);
            free(compressed);
            fclose(input);
            exit(EXIT_FAILURE);
        }
    } else {
        // there is no output argument => write to stdout
        output = stdout;
    }
    int inputLength = sizeOfCharArray(inputBuf)/sizeof(char);
    int outputLength = sizeOfCharArray(compressed)/sizeof(char);
    float compressionRatio = 100 * ((float) outputLength / (float) inputLength);

    size_t fw = fwrite(compressed, sizeof(char), outputLength, output);
    if (fw != outputLength) {
        fprintf(stderr, "%s: fwrite failed!\n", programName);
        free(inputBuf);
        free(compressed);
        fclose(input);
        fclose(output);
        exit(EXIT_FAILURE);
    }

// free memory and close streams
    int ic = fclose(input);
    if (ic != 0) {
        fprintf(stderr, "%s: fclose failed on input-stream\n", programName);
        free(inputBuf);
        free(compressed);
        fclose(output);
        exit(EXIT_FAILURE);
    }
    int oc = fclose(output);
    if (oc != 0) {
        fprintf(stderr, "%s: fclose failed on output-stream\n", programName);
        free(inputBuf);
        free(compressed);
        exit(EXIT_FAILURE);
    }
    free(inputBuf);
    free(compressed);

    int fRatio = fprintf(stderr, "Read:\t\t%d characters\nWritten:\t%d characters\nCompression ratio: %.1f%%\n", inputLength, outputLength, compressionRatio);
    if (fRatio < 0) {
        fprintf(stderr, "%s: fprintf failed!\n", programName);
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}