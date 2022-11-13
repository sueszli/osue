/**
 * @file mycompress.c
 * @author Michael Blank 11909459 <e11909459@student.tuwien.ac.at>
 * @date 30.10.2021
 *
 * @brief Main program module.
 * 
 * @details This programm takes multiple files or stdin as input and either a file or stdout as output.
 * Then going thorugh the inputs one by one it will perform run-length-encoding over the content of the file.
 * The result of the encoding will be written to the ouput and a small message containing how many lines were compressed
 * and the compression ratio will be written to stderr.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details After writing the usage information the program will close with the EXIT_FAILURE value.
 * @param progName The name of the program.
 */
static void usage(char *progName) {
    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", progName);
    exit(EXIT_FAILURE);
}

/**
 * Compression over one file
 * @brief This function will perfrom the run-length encoding over one file.
 * @details Going through the file line by line this function will keep track of the run length of the current char
 * And write them to the output when necessary.
 * @param fileIn Pointer to the input file.
 * @param fileOut Pointer to the output file.
 * @param read Pointer to the char read accumulator.
 * @param written Pointer to the char written accumulator.
 */
static void compressFile(FILE *fileIn, FILE *fileOut, int* read, int* written) {
    char *line = NULL;
    size_t len = 0;
    int runLength = 0;
    char c;

    while (getline(&line, &len, fileIn) > 0) {
        *read += strlen(line);
        if (!c) {
            c = line[0];
        }

        for (int i = 0; i < strlen(line); i++)
        {
            if (c != line[i]) {
                fprintf(fileOut, "%c%i", c, runLength);
                *written += 1 + log10(runLength) + 1;
                runLength = 0;
                c = line[i];
            }
            runLength++;
        }
    }
    fprintf(fileOut, "%c%i", c, runLength);
    *written += 1 + log10(runLength) + 1;
    free(line);
}

/**
 * Program entry point.
 * @brief The program starts here. This function takes care about parameters, and files.
 * @details This function parses all the arguments, opens the needed files and loops through all the input files
 * and pass them on to the compressFile function. Then closes the files and writtes the read/written chars and compression
 * ratio to stderr.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS or EXIT_FAILURE.
 */
int main(int argc, char *argv[]) {
    char *progName = argv[0];

    char *fileNameOut = NULL;

    int opt;

    while ( (opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
        case 'o': 
            fileNameOut = optarg;
            break;
        default:
            usage(progName);
            break;
        }
    }

    FILE *fileOut;

    if (fileNameOut != NULL)
    {
        if((fileOut = fopen(fileNameOut, "w")) == NULL) {
            fprintf(stderr, "%s: Opening of %s failed: %s\n", progName, fileNameOut, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        fileOut = stdout;
    }

    int read = 0;
    int written = 0;

    if (optind == argc) {
        compressFile(stdin, fileOut, &read, &written);
    } else {
        for (int i = optind; i < argc; i++) {
            FILE *fileIn;
            if((fileIn = fopen(argv[i], "r")) == NULL) {
                fprintf(stderr, "%s: Opening of %s failed: %s\n", progName, argv[i], strerror(errno));
                exit(EXIT_FAILURE);
            }

            compressFile(fileIn, fileOut, &read, &written);
            fclose(fileIn);
        }
    }

    fclose(fileOut);

    fprintf(stderr, "\nRead:      %i characters\n", read);
    fprintf(stderr, "Written:   %i characters\n", written);
    double percentage = 100.0 * written/read;
    fprintf(stderr, "Compression Ratio: %.1f%%\n", percentage);
    exit(EXIT_SUCCESS);
}