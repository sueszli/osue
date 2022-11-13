/**
 * @file mydiff.c
 * @author Alexander Knapp <e11808240@student.tuwien.ac.at>
 * @date 10.11.2021
 * 
 * @brief mydiff program module
 * 
 * This Program compares two files line by line, outputting the
 * number of differing characters per line. 
 * 
 **/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<getopt.h>
#include<ctype.h>

//HELPER FUNCTIONS
/**
 * Helper function that compares to integers, returning the smaller one.
 * (Im using this to increase readability.)
 * 
 * @brief This function is used to get the smaller of two integers.
 * @param a is an integer
 * @param b is an integer
 * @return the smaller integer
 * 
 **/
static int myMin(int a, int b){
    if(a < b){
        return a;
    } else {
        return b;
    }
}

/**
 * Helper function that prints a usage message.
 * 
 * @brief This function is called when mydiff is called incorrectly (for example incorrect flags or an incorrect number of arguments). 
 */
static void usage(void){
    (void) fprintf(stderr, "USAGE: mydiff [-i] [-o outfile] file1 file2");
}

//MAIN
/**
 * Program entry point.
 * @brief The program starts here. Arguments are parsed and based on them the program is executed.
 * 
 * @param argc The argument counter
 * @param argv The argument vector
 * @return returns 0 inducationg successful execution.
 **/ 


int main(int argc, char *argv[]){
    FILE * fp1;
    FILE * fp2;
    char* filename1 = argv[optind];
    char* filename2 = argv[optind+1];
    fp1 = fopen(filename1, "r");
    fp2 = fopen(filename2, "r");
    size_t len1 = 0;
    size_t len2 = 0;
    size_t read1 = 0;
    size_t read2 = 0;
    char* outputfilename = "";
    FILE * output;

    int option;
    int optI = 0;
    int optO = 0;
    while((option = getopt(argc, argv, "io:")) != -1){
        switch (option) {
            case 'i':
                optI = 1;
                break;
            case 'o':
                optO = 1;
                outputfilename = optarg;
                break;
            default:
                usage();
                return -1;
        }
    }


    if(optO == 1){
        output = fopen(outputfilename, "w");
        if(output == NULL){
            fprintf(stderr, "Error opening output file");
            return -1;
        }
    }

    if (fp1 == NULL)
    {
        fprintf(stderr, "Error opening first input file\n");
        return -1;
    }else if (fp2 == NULL) {
        fprintf(stderr, "Error opening second input file\n");
        return -1;
    } else {
        char * line1 = "";
        char * line2 = "";
        int lineCount = 1;
        int done = 0;
        while(done == 0){
        if (((read1 = getline(&line1, &len1, fp1)) != -1) && ((read2 = getline(&line2, &len2, fp2)) != -1)) {
            if(optI == 1){
                char * safe_line1 = line1;
                char * safe_line2 = line2;
                char c1,c2 = '\0';
                int j = 0;
                while(line1[j] != '\0' && line2[j] != '\0'){
                    c1 = tolower(line1[j]);
                    c2 = tolower(line2[j]);
                    line1[j] = c1;
                    line2[j] = c2;
                    j++;
                }
                line1 = safe_line1;
                line2 = safe_line2;
            }

        int i = 0;
        int count = 0;
        for (i = 0; i < myMin(read1, read2) - 1; i++)
        {
            if(line1[i] != line2[i]){
                count++;
            }
        }
        //printf("debug: %s, %s", line1, line2);
        if(count > 0){
            if(optO == 0){
                printf("Line: %d, characters: %d\n", lineCount, count);
            } else {
                fprintf(output, "Line: %d, characters: %d\n", lineCount, count);
            }
        }
        lineCount = lineCount + 1;

        } else {
            done = 1;
        }

        }

        free(line1);
        free(line2);
    }
    fclose(fp1);
    fclose(fp2);
    return 0;
}