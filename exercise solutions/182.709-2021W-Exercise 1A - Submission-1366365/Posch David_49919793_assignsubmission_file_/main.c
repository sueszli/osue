/**
 * @file main.c
 * @author David Posch e11817179@student.tuwien.ac.at
 * @date 28.10.2021
 *
 * @brief The whole mycompress module is based in this main.c file
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

char *myprog;

/**
 * Mandatory usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variable: myprog, stores the name of this program from argv[0]
 */
static void usage(void) 
{
    fprintf(stderr,"Usage: %s [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Compression function
 * @brief This function reads char by char from the input stream
 * and writes multiple sequential occurrences of a char in the form of
 * <prev_c><number of sequential occurrences> to the output stream.
 * While doing the compression, read_chars is incremented by one for each read char
 * and written_char is incremented by one for each written char
 * @param input FILE* input stream
 * @param out FILE* output stream
 * @param read_chars Int pointer, pointing to the read char counter
 * @param written_chars Int pointer, pointing to the written char counter
 * @return void
 */
static void compress(FILE *input, FILE *out, int *read_chars, int *written_chars);

/**
 * Program entry point
 * @brief This is the entrypoint of mycompress.
 * @details It handles the argument parsing, opening and closing of FILE* streams and calls compress
 * @param argc The argument counter
 * @param argv The argument vector
 * @return Successful execution returns EXIT_SUCCESS otherwise EXIT_FAILURE is returned
 */
int main (int argc, char **argv)
{
    myprog = argv[0]; // Set myprog to argument 0 (name of program)

    char *outfile_name = NULL; // argument o:

    // Read options
    int c;
    while( (c = getopt(argc, argv, "o:")) != -1)
    {
        switch(c)
        {
            case 'o':
                if(outfile_name != NULL)
                {
                    fprintf(stderr,"Multiple use of -o flag\n");
                    usage();
                }
                outfile_name = optarg;
                break;
            case '?':
                usage();
                break;
            default:
                assert(0);
        }
    }

    FILE *out;
    
    if(outfile_name == NULL)
        out = stdout; // Set the out stream to stdout if no output file is specified
    else
    {
        if ((out = fopen(outfile_name, "w")) == NULL) // Set the out stream to outfile
        {
            fprintf(stderr, "fopen failed for \"%s\": %s\n", outfile_name, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    int read_chars = 0;
    int written_chars = 0;

    if(optind >= argc)
    {
        printf("Reading from stdin, press ctrl+d to quit:\n");
        compress(stdin, out, &read_chars, &written_chars);
    }
    else
    {
        // Read from file
        // For each positional argument (filename)
        for(int i = optind; i < argc; i++) 
        {
            char *filename = argv[i];
            
            FILE *input = fopen(filename, "r");

            // Check if file exists
            if (input == NULL) 
            {
                fprintf(stderr, "fopen failed for \"%s\": %s\n", filename, strerror(errno));

                // If there are more files to read
                if((i + 1) < argc)
                {
                    fprintf(stderr, "Trying to open next file..\n");
                    continue;
                }
                exit(EXIT_FAILURE);
            }

            compress(input, out, &read_chars, &written_chars);
            
            fclose(input);
        }
    }

    fprintf(stdout, "\n");


    // Close outfile if used
    if(outfile_name != NULL)
        fclose(out);


    fprintf(stderr, "\nRead: \t\t%i characters\n", read_chars);
    fprintf(stderr, "Written: \t%i characters\n", written_chars);

    double ratio = ((double) written_chars/(double) read_chars) * 100.0;
    fprintf(stderr, "Compression ratio: \t%.1f%%\n", ratio);

    return EXIT_SUCCESS;
}


static void compress(FILE *input, FILE *out, int* read_chars, int* written_chars)
{
    // Read each char until EOF
    char next_c;
    next_c = fgetc(input);
    char prev_c = next_c;
    int counter = 0;

    while(1) 
    {
        if(feof(input) != 0)
        {
            // printf last char + counter
            if(counter > 0)
            {
                fprintf(out, "%c%i", prev_c, counter);
                *written_chars = *written_chars+2;
            }
            break;
        }

        *read_chars = *read_chars+1;

        if(next_c == prev_c)
            counter++;
        else
        {
            fprintf(out, "%c%i", prev_c, counter);
            *written_chars = *written_chars+2;
            counter = 1;
        }

        prev_c = next_c;
        next_c = fgetc(input);
    }
}