/**
 * @author Agata Podgorska (12023151)
 * @date   11.11.2021
 *
 * @brief This program compares two files line by line
 *
 * @details This program compares two files line by line. If two lines differ, then the line number 
 * and the number of differing characters is printed.
 * Lines are only compared until one of them ends, which means 'abc\n' and 'abcdef\n' are considered to be equal by this program.
 * The program also accepts the lines of any length.
 * The program receives the paths to the two files to compare as arguments as well as the following optional arguments:
 * [-i] to specify a case-insensitive comparison -> the program does npt differentiate between lower and upper case letters
 * [-o path] to specify that the output should be written to the specified file (outfile) and not stdout.
 **/

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *name_program;

static void compare_line(FILE *input_a, FILE *input_b, FILE *output, int i);

/**
 * @brief the usage of the program
 * 
 * Prints the usage of the program in case the input is not correct
 */

static void usage(void)
{
    fprintf(stderr, "Usage: %s [-i] [-o outfile ] file1 file2 \n", name_program);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    name_program= argv[0];
    int c;
    char *o_arg = NULL;
    int i = 0;

    
     //reads from command line
     

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            i = 1;
            break;

        case 'o':
            o_arg = optarg; //where the output is saved
            break;

        case '?': //invalid option
            usage();
            break;

        default: //no case found
            usage();
            break;
        }
    }

    /**
     *  @brief for opening the file to write in
     */

    FILE *output;

    if (o_arg)
    {
        if ((output = fopen(o_arg, "w")) == NULL)
        {
            fprintf(stderr, "%s: Fail to open output file :(  %s \n", argv[0], strerror(errno));
            return EXIT_FAILURE;
        }
    }
    else
    {
        output = stdout;
    }

    /**
     *  @brief read files
     *  @details reads files and handles errors
     *  @param  FILE* a and FILE*b are the filepointers to the given file name
     */

    FILE *a;
    FILE *b;

    if ((a = fopen(argv[optind], "r")) == NULL)
    {
        fprintf(stderr, "%s: Fail to open input_a file  %s \n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    if ((b = fopen(argv[optind + 1], "r")) == NULL)
    {
        fprintf(stderr, "%s: Fail to open input_b file  %s \n", argv[0], strerror(errno));
        return EXIT_FAILURE;
    }

    compare_line(a, b, output, i);

    fclose(a);
    fclose(b);
    fclose(output);
    return 0;
}

/**
 * @brief Compares the given files 
 * @details This function compares two lines from each file. The comparison is being made based
 * if its case insensitive or not.
 * If two lines are different, the line number and the number of differences is printed out.
 * If the files are equal, "No differences found" is printed instead.
 * @param input_a first file 
 * @param input_b secon file
 * @param output file for priniting the result 
 * @param case_insensitive says if the comparison should be case insensitive
 * @param line_count shows in which line the file is different
 * @param count shows how many characters are wrong
 */

static void compare_line(FILE *input_a, FILE *input_b, FILE *output, int case_insensitive)
{
    char *line_left = NULL;
    char *line_right = NULL;
    size_t line_left_size = 0;
    size_t line_right_size = 0;

    //the actual size of the lines
    ssize_t real_size_left = 0;
    ssize_t real_size_right = 0;

    int line_count = 0;

    while ((real_size_left = getline(&line_left, &line_left_size, input_a)) != -1 && (real_size_right = getline(&line_right, &line_right_size, input_b)) != -1)
    {
        int cmp = 0;
        int count = 0;
        line_count++;

        size_t min_size = 0;
        if (real_size_left < real_size_right)
        {
            min_size = real_size_left;
        }
        else
        {
            min_size = real_size_right;
        }

        min_size = min_size - 1;

        if (case_insensitive)
        {
            cmp = strncasecmp(line_left, line_right, min_size);
        }
        else
        {
            cmp = strncmp(line_left, line_right, min_size);
        }

        if (cmp == 0)
        {
            printf("No differences found\n");
            continue;
        }
        else
        {
            for (int i = 0; i < min_size; i++)
            {
                if (line_left[i] != line_right[i])
                {
                    count++;
                }
            }
        }
        fprintf(output, "Line: %d, characters: %d \n", line_count, count);
        
    }
    free(line_left);
    free(line_right);
}
