/**
 * @file main.c
 * @author Maximilian Schenk 11908528 <e11908528@student.tuwien.ac.at>
 * @date 11.11.2021
 *
 * @brief Mycompress program module, implements the full functionallity of mycompress.
 * @details Implements the compression functionallity, by transforming sequences of the same character into one compressed format.
 **/

#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

/**
 * Usage Information function.
 * @brief Writes a usage description to stderr and exits the program.
 * @details Exits the programm with EXIT_FAILURE.
 **/
static void argumentError(void)
{
    fprintf(stderr, "Usage: mycompress [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}


/**
 * Digit Counter function.
 * @brief Counts digits of input.
 * @details The function counts the digits of the parsed in integer.
 * @param n Number to count the digits from.
 * @return Number of digits the input has.
 */
static int countDigits(int n)
{
    int digits = 0;

    while (n != 0)
    {
        n = n / 10;
        digits++;
    }
    return digits;
}


/**
 * Formating function.
 * @brief Formats the input.
 * @details The input (result of compression ratio), gets roundet to the next tenth and multiplied by 100,
 *  in order to get the correct percentage (0.506 -> 50.1).
 * @param n Number to format.
 * @return The formated input.
 */
static float formatRatio(double n)
{
    n = n * 1000;
    if (n < 0.0)
    {
        n = (int)(n - 0.5);
    }
    else
    {
        n = (int)(n + 0.5);
    }
    return n * 0.1f;
}


/**
 * Program entry point
 * @brief This function is the entrypoint of the mycompress program. This
 * function implements the arguent parsing, file opening and closing, compressing and 
 * generating the final statistics.
 * @param argc
 * @param argv
 * @return Upon success EXIT_SUCCESS is returned, otherwise EXIT_FAILURE.
 **/
int main(int argc, char **argv)
{
    bool outfile = false;
    int opt_o = 0;
    char *output_path;
    int c;

    // get options
    while ((c = getopt(argc, argv, "o:")) != -1)
    {
        switch (c)
        {
        case 'o':
            outfile = true;
            output_path = (char *)malloc(strlen(optarg) * sizeof(char));

            if (output_path)
            { // check if allocation was succesful
                strcpy(output_path, optarg);
                opt_o++;
            }
            else
            {
                exit(EXIT_FAILURE);
            }

            break;
        case '?':
            argumentError();
            break;
        default:
            assert(0);
        }
    }

    // If -o appears multiple times
    if (opt_o > 1)
    {
        argumentError();
    }

    // handle positional arguments
    int argument_counter = argc - optind;
    char **arguments;

    arguments = (char **)malloc(argument_counter * sizeof(char *));

    if (arguments == NULL)
    { // check if allocation was successful
        exit(EXIT_FAILURE);
    }

    
    int i, x;
    bool inFile;
    // copy parsed arguments into arguments array
    if (argc - argument_counter != 0 && argc != 1)
    {
        inFile = true;
        for (i = argc - argument_counter, x = 0; i < argc; i++, x++)
        {
            arguments[x] = (char *)malloc(strlen(argv[i]) * sizeof(char));

            if (arguments[x])
            { // check if allocation was succesful
                strcpy(arguments[x], argv[i]);
            }
            else
            {
                exit(EXIT_FAILURE);
            }
        }
    }
    else
    {
        inFile = false;
        argument_counter = 1;
    }

    char ch, temp;
    char *result = "";
    // if allocation of array fails
    if ((result = (char *)malloc(3)) == NULL)
    {
        exit(EXIT_FAILURE);
    }
    // make some variables to work with
    size_t len;
    int counter = 1;
    int digits, read = 0, written = 0;
    float compression;
    // check if input file was defined
    if (inFile == false)
    {
        argument_counter = 1;
    }
    // repeat for each input file
    for (i = 0; i < argument_counter; i++)
    {
        FILE *file;
        // if input file was specified, open it
        if (inFile)
        {
            file = fopen(arguments[i], "r");
        }
        else // if no input file is specified, use stdin
        {
            file = stdin;
        }
        // check if input file could be opended
        if (file == NULL)
        {
            perror("Error while opening file!\n");
            exit(EXIT_FAILURE);
        }
        // read first char
        temp = fgetc(file);
        bool last = false;

        // compression
        while (!feof(file) || !last)
        {   //read next char
            ch = fgetc(file);
            read++;
            // so the last char gets read too
            if (ch == EOF)
            {
                last = true;
            }
            // if still the same char increment counter
            if (ch == temp)
            {
                counter++;
            }
            else // if the chars are not the same anymore
            {
                len = strlen(result);
                digits = countDigits(counter);
                result = realloc(result, len + 2 + (sizeof(char) * digits)); // +1 Nullbyte +1 char + digits
                // if allocation of array fails
                if (result == NULL)
                {
                    exit(EXIT_FAILURE);
                }

                result[len] = temp;
                written++;

                int number, j;
                // logic for adding the counter to the compressed char
                for (j = digits; j > 1; j = j - 1)
                { 
                    number = counter % 10;
                    counter = counter / 10;
                    result[len + i] = number + '0';
                    written++;
                }
                result[len + 1] = counter + '0';
                written++;
                // add the nullbyte
                result[len + digits + 1] = '\0';

                counter = 1;
            }
            temp = ch;
        }

        fclose(file);
    }

    // handle output
    if (!outfile)
    { // if no outfile is specified write to stdout
        fprintf(stdout, "\n%s\n", result);
    }
    else
    { // else open outfile, write result, close outfile
        FILE *outfile = fopen(output_path, "w");
        if (outfile == NULL)
        {
            perror("Error while opening file!\n");
            exit(EXIT_FAILURE);
        }
        fprintf(outfile, "%s", result);
        fclose(outfile);
    }

    // calculate the compression ratio
    compression = ((float)written / (float)read);
    compression = formatRatio(compression);

    // write the statistics to stderr
    fprintf(stderr, "\nRead:\t\t %d characters\n", read);
    fprintf(stderr, "Written:\t %d characters\n", written);
    fprintf(stderr, "Compression ratio:\t %0.1f%%\n\n", compression);

    // free allocated Memory
    for (i = 0; i < argument_counter; i++)
    {
        free(arguments[i]);
    }
    free(arguments);
    free(result);

    if (outfile)
    {
        free(output_path);
    }

    exit(EXIT_SUCCESS);
}