/**
 * @author Julia Liepold, 11807886
 * @name ispalindrom 
 * @date 30.10.2021
 * @brief ispalindrom [-s] [-i] [-o outfile] [file...]
        -o specifies the outputfile and file the input file, if either one not given: use stdout or/ and stdin
        -s ignores whitespaces -i is for case insensitivity 
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>



/**
*@brief print usage & exit with error
*/
void usage(void)
{
    fprintf(stderr, "ispalindrom: SYNOPSIS: ispalindrom [-s] [-i] [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}

/**
*@brief determine if input is palindrome
*@param input where to read from
*@param output where to write to
*@param iflag indicator flag for case insensitivity
*@param sflag indicator flag to ignore whitespace
*/
static int ispalindrome(FILE *input, FILE *output, int iflag, int sflag)
{
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    while( (nread = getline(&line, &len, input)) != -1) {

        // need original input for fprintf
        char *copy = NULL;
        copy = malloc(sizeof(char) * strlen(line) + 1);
        if (copy == NULL) {
            fprintf(stderr, "ispalindrom: malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        snprintf(copy, strlen(line), "%s", line);
        
        // all chars to lower bc case insensitive
        if (iflag == 1) {
            int i;
            for(i = 0; copy[i]; i++) {
            copy[i] = tolower(copy[i]);
            }
        }
    
        // remove ' ', '\t', '\r', '\n', '\v', '\f' bc ignore whitespace characters
        if (sflag == 1) {
            char temp[strlen(copy) + 1];
            int j = 0;
            int i;
            for(i = 0; copy[i]; i++) {
                if(!isspace(copy[i])) temp[j++] = copy[i];
            }
            temp[j] = '\0'; 
            snprintf(copy, strlen(copy) + 1, "%s", temp);
        }

        // reverse copy into cmp
        char *cmp = NULL;
        cmp = malloc(sizeof(char) * strlen(copy) + 1);
        if (cmp == NULL) {
            fprintf(stderr, "ispalindrom: malloc() failed\n");
            exit(EXIT_FAILURE);
        }
        
        snprintf(cmp, strlen(copy) + 1, "%s", copy);

        int l = strlen(cmp); 
        int temp = 0;

        int i;
        for (i = 0; i < (l / 2); i++) {  
            temp = cmp[i];  
            cmp[i] = cmp[l - i - 1];  
            cmp[l - i - 1] = temp;  
        }

        line[strlen(line) - 1] = '\0';

        if (strcmp(copy, cmp) == 0) {
            fprintf(output, "%s is a palindrome\n", line);
        } else {
            fprintf(output, "%s is not a palindrome\n", line);
        }
        free(cmp);
        free(copy);
    }
    free(line);
    fflush(output);  

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  char c;
  int oflag = 0;
  int sflag = 0;
  int iflag = 0;
  char *path_out = NULL;

  while ((c = getopt(argc, argv, "iso:")) != -1) {
        switch (c) {
            case 'o':
                oflag += 1;
                path_out = optarg;
                break;
            case 'i':
                iflag += 1;
                break;
            case 's':
                sflag += 1;
                break;      
            default:
                usage();
                break;
        }
    }

    if (oflag > 1 || iflag > 1 || sflag > 1) usage();
        
    FILE *output = stdout;

    // if outputfile is specified
    if (oflag == 1) {
        output = fopen(path_out, "w");
        if (output == NULL) {
            fprintf(stderr, "\n%s: fopen() failed\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc) {
        // no input file given -> read from stdin
        ispalindrome(stdin, output, iflag, sflag);
    } else {
        // loop through all given input files
        int i;
        for (i = optind; i < argc; i++) {
            FILE *input = fopen(argv[i], "r");
            if (input == NULL) {
                fprintf(stderr, "%s: here: fopen() failed\n", argv[0]);
                exit(EXIT_FAILURE);
            }
            ispalindrome(input, output, iflag, sflag);
            fclose(input);
        }
    }
    fclose(output);
    return EXIT_SUCCESS;
} 