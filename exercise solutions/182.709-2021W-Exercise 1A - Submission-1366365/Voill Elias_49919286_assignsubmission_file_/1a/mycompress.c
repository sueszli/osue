// @file mycompress.c
// @author Elias Voill e12023149@student.tuwien.ac.at
// @date 21.10.2020
// @brief mycompress BSUE excercise 1a


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

// @brief main() compresses a file
// @details main() first reads and saves arguments
// (flages, input and output file names)
// then 
int main(int argc, char *argv[])
{
    // Read arguments and open output. Will be used later
    FILE *output = stdout;
    const char *outputName = NULL;
    const char *const programName = argv[0];
    int c;

    while ((c = getopt(argc, argv, "o:")) != -1) { //handle flags

        switch (c) {

        case 'o':
            outputName = optarg;
            break;
        default:
            break;
        }
    }

    // Save input into array
    int temp = argc - optind;
    char const *inArr[temp];
    for (int i = 0; i < temp; i++) {
        inArr[i] = argv[optind + i];
    }

    if (outputName != NULL)
    {
        output = open(outputName, "w");
        if (output == NULL)
        {
            fprintf(stderr, "(mycompress) opening file failed");

            exit(-1);
        }
    }

    uint64_t writeChars = 0;
    uint64_t readChars = 0;

    if (temp == 0) { // if stdin should be used

        if (compressFile(stdin, output, &readChars, &writeChars) == -1) {

            fprintf(stderr, "(mycompress) compressing file failed");
            fclose(output);

            exit(-1);
        }
    }

    // Compress the files
    for (int i = 0; i < temp; i++) {

        FILE *input = fopen(inArr[i], "r");

        if (input == NULL) {
            fprintf(stderr,"(mycompress) input file opening failed");

            close(output);

            exit(-1);
        } 


        if (compressFile(input, output, &readChars, &writeChars) == -1) {
            fprintf(stderr, "(mycompress) compressing file failed");

            close(input);
            close(output);

            exit(-1);
        }  

        close(input);
    }


    fprintf(stderr, "Read: \t\t %lu characters \nWritten: \t %lu chararcters \nCompression ratio: %f%%\n", readChars, writeChars, ((double)(writeChars)/readChars) * 100.0);

    if (output != stdout) {
        close(output);
    }

    return 0;
}

// @brief compresses a file and writes it in the output
// @details this function will compress incoming chars as long as the
// file contains chars and write it into the output file
// @param two files in and out, and pointer to variable which track the amount of written and read chars

int compressFile(FILE *in, FILE *out, uint64_t *readChars, uint64_t *writeChars) {

    int count = 0;
    int last;

    while (1) { // compresses as 

        int c = fgetc(in); // gets the current char 

        if (feof(in)) { // if file is empty
            break;
        }

        *readChars =*readChars + 1;

        if (count == 0) {

            count = 1;
            last = c;
            continue;
        }

        if (c == last) {

            count++;
            continue;
        }

        int x = fprintf(out, "%c%d", last, count);

        if (x == -1) { // if nothing was printed
            return -1;
        }

        count = 1;
        last = c;
        *writeChars = *writeChars + x;
    }

    // Write the last character to the file
    int x = fprintf(out, "%c%d", last, count);
    if (x == -1) {
        return -1;
    }
    *writeChars += x;
    return 0;
}