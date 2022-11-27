/**
 * @file main.c
 * @author Jakob Guttmann 11810289 <e11810289@student.tuwien.ac.at>
 * @brief Main program module for mycompress, no other modules needed
 * @date 8.11.2021
 * 
 * This is my solution for "Assignment A - mycompress"
 * Compresses text files with algorithm in form of a character and the number of subsequent occurences
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

/**
 * Program entry point
 * @brief main method of mycompress, no other functions needed
 * first part of the main method performs the option handling and opens an output file if the option -o is activated
 * then opens every input file which is passed to the program
 * compresses the input line after line and "on the fly"
 * if no arguments are passed to the program the input stream is stdin and output stream is stdout
 * in this case if the program gets one line from stdin, this line gets compressed and printed to stdout (I hope this behaviour is desired)
 * 
 * @param argc argument counter
 * @param argv argument vector
 * @return int returns EXIT_SUCCESS if everything worked correctly, returns EXIT_FAILURE if any file operations like opening/reading fail
 */
int main(int argc, char *argv[])
{

    char *o_arg = NULL;
    int opt_o = 0;
    int c;
    int counter = 1;

    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            opt_o++;
            o_arg = optarg;
            counter += 2;
            break;
        default:
            fprintf(stderr, "[%s] ERROR: invalid option/wrong arguments\ncorrect calling syntax: mycompress [-o outfile] [file...]\n", argv[0]);
            assert(0);
            break;
        }
    }

    FILE *in = stdin;
    FILE *out = stdout;

    if (opt_o != 0)
    {
        if (o_arg != NULL)
        {
            out = fopen(o_arg, "w");
            if (out == NULL)
            {
                fprintf(stderr, "[%s] ERROR: cannot open file %s\nReason: %s\nExit with error code\n", argv[0], o_arg, strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "error in %s: output file name is null\nExit with error status\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    char *buffer = NULL;
    size_t len = 0;
    ssize_t nread;

    int numRead = 0;
    int numWrite = 0;

    for (int i = counter; (i < argc) || (argc == counter); i++)
    {
        if (argc != counter)
        {
            in = fopen(argv[i], "r");
            if (in == NULL)
            {
                fclose(out);
                fprintf(stderr, "[%s] ERROR: cannot open file %s\nReason: %s\nExit with error code\n", argv[0], argv[i], strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            counter++;
        }

        char c;
        if ((c = fgetc(in)) == EOF)
        {
            fclose(in);
            continue;
        }
        fprintf(out, "%c", c);
        numWrite += 1;
        numRead += 1;
        int sameChars = 1;

        while ((nread = getline(&buffer, &len, in)) != -1)
        {
            numRead += nread;
            for (int i = 0; i < nread; i++)
            {
                if (buffer[i] == c)
                {
                    sameChars++;
                }
                else
                {
                    c = buffer[i];
                    if (fprintf(out, "%d%c", sameChars, c) < 0)
                    {
                        fclose(in);
                        fclose(out);
                        free(buffer);
                        fprintf(stderr, "[%s]: writitng to stdout/output file has failed.\nReason: %s\nExit with Error code\n", argv[0], strerror(errno));
                        exit(EXIT_FAILURE);
                    }
                    sameChars = 1;
                    numWrite += 2;
                }
            }
        }
        if (fprintf(out, "%d", sameChars) < 0)
        {
            fclose(out);
            fclose(in);
            free(buffer);
            fprintf(stderr, "[%s]: writitng to stdout/output file has failed.\nReason: %s\nExit with Error code\n", argv[0], strerror(errno));
            exit(EXIT_FAILURE);
        }
        numWrite += 1;

        fclose(in);
    }
    free(buffer);
    fclose(out);

    double ratio = (((double)numWrite) / ((double)numRead)) * 100;

    fprintf(stderr, "\nRead:\t\t%d characters\nWritten:\t%d characters\nCompression ratio: %.1f\n", numRead, numWrite, ratio);

    exit(EXIT_SUCCESS);
}