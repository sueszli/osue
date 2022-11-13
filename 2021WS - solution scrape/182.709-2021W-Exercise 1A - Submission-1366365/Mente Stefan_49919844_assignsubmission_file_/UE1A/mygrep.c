#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *myFile;

void stringToLower(char *const input) {
    for (int i = 0; input[i]; ++i) {
        input[i] = tolower(input[i]);
    }
}

int main(int argc, char **argv)
{
    myFile = argv[0];
    bool ignoreCase = false;
    char *output = NULL;
    int c;

    while ((c = getopt(argc, argv, "io: ")) != -1)
    {
        switch (c)
        {
        case 'o':
            if (output != NULL)
            {
                fprintf(stderr, "Using: %s [-i] [-o outputfile] keyword [file] \n", myFile);
                exit(EXIT_FAILURE);
            }
            output = optarg;
            break;
        case 'i':
            if (ignoreCase == true) 
            {
                fprintf(stderr, "Using: %s [-i] [-o outputfile] keyword [file] \n", myFile);
                exit(EXIT_FAILURE);
            }
            ignoreCase = true;
            break;
        case '?':
            fprintf(stderr, "Using: %s [-i] [-o outputfile] keyword [file] \n", myFile);
            exit(EXIT_FAILURE);
            break;
        default:
            assert(0);
            break;
        }
    }
    char *keyword = argv[optind];
    if (keyword == NULL) {
            fprintf(stderr, "Using: %s [-i] [-o outputfile] keyword [file] \n", myFile);
            exit(EXIT_FAILURE);
    } else {
        printf("keyword: key %d\n", optind);
    }
    char *inputFileName =argv[optind + 1];
    FILE *inputFile = stdin;
    char *buffer = NULL;
    size_t bufferLength = 0;
    ssize_t readLength;
    FILE *outputFile = stdout;
    if (output != NULL) {
        if ((outputFile = fopen(output, "w"))== NULL) {
            fprintf(stderr, "Using: %s [-i] [-o outputfile] keyword [file] \n", myFile);
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 1; ((inputFileName = argv[optind + i]) != NULL) || inputFile == stdin; ++i) {
        if (inputFileName != NULL) {
            if ((inputFile = fopen(inputFileName, "r")) == NULL) {
                fprintf(stderr, "Using: %s [-i] [-o outputfile] keyword [file] \n", myFile);
                exit(EXIT_FAILURE);
            }
        }
        while ((readLength = getline(&buffer, &bufferLength, inputFile)) != -1) {
            char bufferCopy[readLength];
            strcpy(bufferCopy, buffer);
            if (ignoreCase == true) {
                stringToLower(buffer);
                stringToLower(keyword);
            }
            if (strstr(buffer, keyword) != NULL) {
                strcpy(bufferCopy, buffer);
                if (fputs(bufferCopy, outputFile) == EOF) {
                    fprintf(stderr, "[%s]Error fputs failed: %s\n", myFile, strerror(errno));
                    free(buffer);
                    exit(EXIT_FAILURE);
                }
            } 
        }
        if (inputFile != stdin) {
            if (fclose(inputFile) == EOF) {
                fprintf(stderr, "Error fclose failed: %s\n", strerror(errno));
            }
        }
    }
        if (outputFile != stdout) {
            if (fclose(outputFile) == EOF) {
                fprintf(stderr, "[%s]Error fclose failed: %s\n", myFile, strerror(errno));
            }
        }
        if (buffer != NULL) {
            free(buffer);
        }
    exit(EXIT_SUCCESS);

}
