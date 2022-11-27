#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <stdlib.h>

/**
<Takes 2 files and returns # of diff. characters in every line.>

<Longer description>
<Reads in two files und compares them. If two lines differ,
then the line number and the number of differing characters is printed.
 The program shall read each file line by line and compare the characters.
 If two lines have different length, 
then the comparison shall stop upon reaching the end of the shorter line.
 Therefore, the lines abc\n und abcdef\n shall be treated as being identical.>

@return 0 if the program terminated successfully,
@return 1 if an error occured.
*/

int main(int argc, char **argv)
{
    // temp variable is needed for switch cases only.
    int t;
    FILE *outputFile = stdout;
    int isCaseSensitiv = 1;
    while ((t = getopt(argc, argv, "o:i")) != -1)
    {
        switch (t)
        {
        case 'o':
            outputFile = fopen(optarg, "w");
            if (outputFile == NULL){
                fprintf(stderr, "Could not write into file %s\n", optarg);
                return (EXIT_FAILURE);
            }
            break;
        case 'i':
            isCaseSensitiv = 0;
            break;
        case '?':
            printf("Usage: %s file 1 file2\n", argv[0]);
            return (EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }
    if ((argc - optind) != 2)
    {
        fprintf(stderr, "Usage: %s [-i] [-o outfile] file 1 file2\n", argv[0]);
        return (EXIT_SUCCESS);
    }
    char *input1 = argv[optind];
    char *input2 = argv[optind + 1];

    FILE *file1;
    if ((file1 = fopen(input1, "r")) == NULL)
    {
        fprintf(stderr, "Could not open the first file\n");
        return (EXIT_FAILURE);
    }

    FILE *file2;
    if ((file2 = fopen(input2, "r")) == NULL)
    {
        fprintf(stderr, "Could not open the second file\n");
        return (EXIT_FAILURE);
    }

    char c1 = 0;
    char c2 = 0;
    int currLine = 1;
    int numOfDiffChars = 0;

    // single or instead of double so that both characters are updated
    while (((c1 = fgetc(file1)) != EOF) | ((c2 = fgetc(file2)) != EOF))
    {
        int check1 = c1 == '\n' || c1 == EOF;
        int check2 = c2 == '\n' || c2 == EOF;

        // If a line by file 1 is ended, but by file 2 is not -
        // iterate file 2 till the line ended as well.
        if (check1 && !check2)
        {
            while (!check2)
            {
                c2 = fgetc(file2);
                check2 = c2 == '\n' || c2 == EOF;
            }
        }
        // vice versa
        else if (!check1 && check2)
        {
            while (!check1)
            {
                c1 = fgetc(file1);
                check1 = c1 == '\n' || c1 == EOF;
            }
        }

        if (((!isCaseSensitiv) ? tolower(c1) != tolower(c2) : c1 != c2) && (!check1 && !check2))
        {
            numOfDiffChars++;
        }
        // both characters are either '\n' or EOF
        else if (check1 && check2)
        {
            if (numOfDiffChars > 0)
            {
                fprintf(outputFile, "Line: %d, characters: %d\n", currLine, numOfDiffChars);
                numOfDiffChars = 0;
            }
            currLine++;
        }
    }

    fclose(file1);
    fclose(file2);
    fclose(outputFile);

    return EXIT_SUCCESS;
}
