/**
 * @file mydiff.c
 * @author Jakub Poprawski 11812310
 * @date 05.11.2021
 *
 * @brief Programme comparing two lines and returning the number of differing characters
 *  
 **/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>



/**
 * Programm entry point
 * @brief The function which reads in several files and replaces tabs with spaces.
 * @details global variables: 
 */

void change(char *in1, char *in2, int line_number, FILE *f_out, bool case_insensitive, bool to_file)
{
    int differences = 0;

    while(*in1 != '\n' && *in2 != '\n')
    {
        if(case_insensitive)
        { 
            if(isupper(*in1))
            {
                *in1 = tolower(*in1);
            }
            if(isupper(*in2))
            {
                *in2 = tolower(*in2);
            }
        }
       
        if (*in1 != *in2)
        {
            differences++;
        }

        in1++;
        in2++;
    }

    if(differences > 0){
        if(to_file)
        {
           fprintf(f_out, "Line: %d, characters: %d\n", line_number, differences);
        }
        else
        {
            printf("Line: %d, characters: %d\n", line_number, differences);
        }
    }
}

int main (int argc, char *argv[])
{
    int c = 0;
    FILE *f_in1;
    FILE *f_in2;
    FILE *f_out;
    bool to_file = false;
    bool case_insensitive = false;


    if(argc != 0)
    {
        while((c = getopt(argc, argv, "io:")) != -1)
         switch (c)
          {
            case 'i': /* if the case insensitivity optional parameter is passed, it is saved in a boolean variable in order not to be forgotten*/
            {
                case_insensitive = true;
            }
            break;
            case 'o': /* if the output file optional parameter is passed, output will be saved there*/
            {
                f_out =  fopen(optarg, "w");
                to_file = true;
            }
            case '?': /* invalid option */
                break;
            default:
                break;
        }

    }
    f_in1 = fopen(argv[argc - 2], "r");
    f_in2 = fopen(argv[argc - 1], "r");
    if(f_in1 == NULL || f_in2 == NULL)
    {
        printf(stderr, "fopen failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }
    char *in1 = NULL, *in2 = NULL;
    int cur_line = 1;
    size_t len1 = 0, len2 = 0;

    while (getline(&in1, &len1, f_in1) != -1 && getline(&in2, &len2, f_in2) != -1)  /* if there is a next line in the input file 1*/
    {
        change(in1, in2, cur_line, f_out, case_insensitive, to_file);
        cur_line++;
    }
    
    return EXIT_SUCCESS;
}

