#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>

// a function for the usage
#define USAGE() \
    fprintf(stderr, "Usage: mycompress [-o outfile] [file...]\n");

// a function for errors
#define ERROR_MESSAGE(...) \
    fprintf(stderr, "mycompress error:   %s", __VA_ARGS__); \
    exit(EXIT_FAILURE);


int main(int argc, char *argv[]){ 
    
    FILE *output = stdout;
    int c;
    int total_read = 0;
    int total_written = 0;

    while ((c = getopt(argc, argv, "o:")) != -1){
        switch (c){
            case 'o':
                if(output == stdout){
                    output = fopen(optarg, "w");
                }else{
                    USAGE();
                    ERROR_MESSAGE("Output files can appear only once.");
                }
                break;
            case '?':
                USAGE();
                ERROR_MESSAGE("Invalid option");
                break;
            default: // Unreachable code
                assert(0); 
                break;
        }
    }

    if(output == NULL){
        ERROR_MESSAGE("Cannot open output file.");
        return -1;
    }


    // when there are no input files
    FILE *input;
    if(optind == argc){
        input = stdin;
        compress(input, output, &total_read, &total_written);
        fclose(input);
        return 0;
    }

    int i;
    for (i = optind; i < argc; i++){
        input = fopen(argv[i],"r");

        if(input == NULL){
            continue;
        }
        compress(input, output, &total_read, &total_written);
        fclose(input);
    }

    fclose(output);

    double ratio = ((double) total_written / (double) total_read) * 100.0;

    fprintf(stderr, "Read: %d\n", total_read);
    fprintf(stderr, "Written: %d\n", total_written);
    fprintf(stderr, "Ratio: %.1f%%\n", ratio);

    return 0;
}


// reads an input file character by character and counts the number of symbols for all sequences with similar characters
// total_read/total_written - globally read/written characters
void compress(FILE *input, FILE *output, int *total_read, int *total_written){
    if(input == NULL || output == NULL){
        ERROR_MESSAGE("No input or output in function \"compress\"");
    }else{

        int countChars = 0;
        int current;
        int previous;
        int local_read = 0; // Read symbols in a single file

        while (!feof(input)){

            current = fgetc(input);
            local_read++;
            *total_read += 1;

            if(current == previous){
                countChars++;
            }

            if(local_read > 1 && current != previous){
                fprintf(output, "%c%d", previous, countChars + 1);
                countChars = 0;
                *total_written += 2;
            }
            
            previous = current;
        }
        printf("\n");

    }
}