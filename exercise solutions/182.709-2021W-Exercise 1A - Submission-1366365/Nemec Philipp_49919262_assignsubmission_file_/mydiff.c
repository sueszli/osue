/**
 * @file mydiff.c
 * @author Philipp Nemec 11907551
 * @date 08.11.2021
 *
 * @brief A program that reads two files and compares them.
 *
 * @details If two lines differ, then the line number and the number of differing characters is printed.
 * If two lines have different length, then the comparison stop upon reaching the end of the shorter line.
 * Therefore, the lines abc\n und abcdef\n are treated as being identical. The program accepts lines of any length.
 * If the option [-o outfile] is given, the output is written to the specified file. Otherwise, the output is written to stdout.
 * If the option [-i] is given, the program does not differentiate between lower and upper case letters.
 **/

#include <ctype.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief A function which compares two files line by line and prints the difference result for every line in some output channel/file.
 * 
 * @details The function runs a loop till one or both of the two input files get an end of file symbol.
 * Inside the loop each character of both input files lines are compared with each other,
 * if a difference occurs it is printed to the specified output channel/file. When a newline character is detected
 * on one of the two input files lines the single character reader is fast forwarded on both files to the next line.
 * 
 * @param file1 The first input file of the program.
 * @param file2 The second input file of the program.
 * @param outfile The output channel/file of the program.
 * @param caseSensitive A flag to determine if case sensitivity is important.
 */
static void compareFiles(FILE *file1, FILE *file2, FILE *outfile, bool caseSensitive)
{
    int line = 1;
    int diffCounter = 0;
    bool diff;
    int c1 = fgetc(file1);
    int c2 = fgetc(file2);
    do
    {
        c1 = fgetc(file1);
        c2 = fgetc(file2);
        if (c1 == '\r' || c2 == '\r' || c1 == EOF || c2 == EOF)
        {
            if (diffCounter != 0)
            {
                fprintf(outfile, "Line: %d, characters: %d\n", line, diffCounter);
            }
            while (c1 != '\r' && c1 != EOF)
            {
                c1 = fgetc(file1);
            }
            if (c1 == '\n') //For Windows
            {
                c1 = fgetc(file1);
            }
            while (c2 != '\r' && c2 != EOF)
            {
                c2 = fgetc(file2);
            }
            if (c2 == '\n') //For Windows
            {
                c2 = fgetc(file2);
            }
            diffCounter = 0;
            line++;
        }
        else
        {
            if (!caseSensitive)
            {
                c1 = tolower(c1);
                c2 = tolower(c2);
            }
            if (c1 != c2)
            {
                diffCounter++;
                diff = true;
            }
        }
    } while (c1 != EOF && c2 != EOF);
    if (!diff)
    {
        fprintf(outfile, "The input files are equal.\n");
    }
}

/**
 * @brief The start of the program.
 * 
 * @details First the parameters of the program get checked, if they are used wrong in any way
 * a error is printed out on stderr and the program closes with the exit code EXIT_FAILURE.
 * Second the two input files, the output channel/file and a case sensitivity flag are handed over
 * to a function which compares the files line by line. Last but not least all files will be closed and
 * on success the program will end with the exit code EXIT_SUCCESS.
 * 
 * @param argc The argument counter of the program.
 * @param argv The argument values of the program.
 * @return The exit code of the program.
 */
int main(int argc, char *argv[])
{
    FILE *outfile = stdout;
    bool caseSensitive = true;
    int option;
    while ((option = getopt(argc, argv, "o:i")) != -1) //Check possible options
    {
        switch (option)
        {
        case 'o':
            if ((outfile = fopen(optarg, "w")) == NULL)
            {
                fprintf(stderr, "%s: Output file cannot be opened!\n", argv[0]);
                return EXIT_FAILURE;
            }
            break;
        case 'i':
            caseSensitive = false;
            break;
        default:
            fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (argc != optind + 2) //Check parameters count
    {
        fprintf(stderr, "Usage: %s [-i] [-o outfile] file1 file2\n", argv[0]);
        return EXIT_FAILURE;
    }
    FILE *file1, *file2;
    file1 = fopen(argv[optind], "r");
    file2 = fopen(argv[optind + 1], "r");
    if (file1 == NULL || file2 == NULL)
    {
        fprintf(stderr, "%s: Input files cannot be opened!\n", argv[0]);
        return EXIT_FAILURE;
    }
    compareFiles(file1, file2, outfile, caseSensitive);
    fclose(file1);
    fclose(file2);
    if (fclose(outfile) == EOF)
    {
        fprintf(stderr, "%s: Output file could not be saved!\n", argv[0]);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
