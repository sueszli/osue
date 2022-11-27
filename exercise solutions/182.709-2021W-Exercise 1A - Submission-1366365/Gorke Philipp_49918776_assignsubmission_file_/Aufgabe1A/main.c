/**
 * @file main.c
 * @author Philipp Gorke <e12022511@student.tuwien.ac.at>
 * @date 14.11.2021
 *
 * @brief mydiff main programm
 * 
 * This programm compares two textfiles and returns the incorrect lines
 *
 **/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "help.h"


/**
 * @brief compares textfiles and returns incorrect lines
 * @details First the flags are checked with getopt. Only i and o: are valid.
 * After that it's checked if there are exactly two files. Both of them are opened
 * and reading can begin. Each line is compared and the out String contains the incorrect 
 * lines and formats them in the perfect way.  
 */
int main(int argc, char *argv[])
{

    char *filename3 = NULL;

    char string1[1000];     //line from file one
    char string2[1000];     //line from file two
    int line = 1;           //line number used by fgets
    int temp;               //temporary value, helps main method
    int caseSensitive = 0; // 0 = false, 1 = true for i flag
    int outputFile = 0;     // 0 = false, 1 = true for o: flag

    int c;
    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        if (c == 'i')
        {
            caseSensitive = 1;
        }
        else if (c == 'o')
        {
            outputFile = 1;
            filename3 = optarg;
        }
    }

    if ((argc - optind) != 2)
    {
        fprintf(stderr, "ERROR: expected exactly 2 files in: %s \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *file1 = fopen(argv[optind], "r");
    FILE *file2 = fopen(argv[optind + 1], "r");

    if (file1 == NULL && file2 == NULL) {
        fprintf(stderr, "ERROR: problem with opening both files in: %s \n", argv[0]);
    }

    if (file1 == NULL)
    {
        fprintf(stderr, "ERROR: problem with opening file1 in: %s \n", argv[0]);
        if (fclose(file2) == EOF){
            fprintf(stderr, "ERROR: problem with closing file2");
        }
        exit(EXIT_FAILURE);
    }

    if (file2 == NULL) {
        fprintf(stderr, "ERROR: problem with opening file2 in: %s \n", argv[0]);
        if (fclose(file1) == EOF) {
            fprintf(stderr, "ERROR: problem with closing file1");
        }
        exit(EXIT_FAILURE);
    }

    //comparing both Strings, formating happens in getString
    char out[200] = "";
    while (fgets(string1, 256, file1) != NULL && fgets(string2, 256, file2) != NULL)
    {
        int maxLength = biggerString(string1, string2);
        if (caseSensitive == 1)
        {
            temp = strncasecmp(string1, string2, maxLength);
        }
        else
        {
            temp = strncmp(string1, string2, maxLength);
        }
        if (temp != 0)
        {
            getString(line, incorrectChars(string1, string2, maxLength), out);
        }
        line++;
    }
    // if o: flag is included 
    if (outputFile == 1)
    {
        FILE *output = fopen(filename3, "w");
        if (output == NULL)
        {
            fprintf(stderr, "ERROR: problem with output file in: %s \n", argv[0]);
            exit(EXIT_FAILURE);
        }
        if (fwrite(out, sizeof(char), sizeof(out), output) != sizeof(out)){
            fprintf(stderr, "ERROR: problem writing in the output file");
        }
        if (fclose(output) == EOF) {
            fprintf(stderr, "ERROR: problem closing output file");
        }
    }
    // if o: flag is excluded
    else
    {
        printf("%s", out);
    }


    if (fclose(file1) == EOF){
        fprintf(stderr, "ERROR: problem closing file1");
    };
    
    if (fclose(file2) == EOF) {
        fprintf(stderr, "ERROR: problem closing file2");
    }

    return EXIT_SUCCESS;
}
