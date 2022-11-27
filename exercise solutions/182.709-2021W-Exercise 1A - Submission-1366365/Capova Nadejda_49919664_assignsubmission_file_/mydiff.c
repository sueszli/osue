/**
 * @file   mydiff.c
 * @author Nadejda Capova (11923550)
 * @date   24.10.2021
 *
 * @brief C-program, which reads in two files and compares them.
 *
 * @details The program compares two files. If two lines differ, then the line number
 * and the number of differing characters is printed. If two lines have different
 * length, then the comparison shall stop upon reaching the end of the shorter line.
 * If the option -o is given, the output is written to the specified file (outfile). Otherwise, the output is
 * written to stdout.
 * If the option -i is given, the program shall not differentiate between lower and upper case letters, i.e.
 * the comparison of the two lines shall be case insensitive.
 * The program must accept lines of any length.
 *
 **/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>

/*defines which of the both numbers is smaller*/
#define my_min(x,y) (((x)<=(y))?(x):(y))

/*name of the program*/
static char *myprog;

/**
 * @brief Prints in stderr the synopsis and terminates the program with EXIT_FAILURE
 */
void usage(void) {
    fprintf(stderr, "%s [-i] [-o outfile] file1 file2\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief The entry point of the program.
 * @details This is the only function that executes the whole program.
 * The programs opens the both files given from the positional operands and depending on the arguments the
 * the files are compared. Aftherwards, the line and the number of different charecters is given either in the console,
 * or in a seperate file.
 * global variables used: myprog which stores the program name
 *
 * @param argc  - argument counter
 * @param argv - argument values stored in an array
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[]) {
    int ifnd = 0;
    int opt;
    int ofnd = 0;
    char *outfile = NULL;
    myprog = argv[0];

    while ((opt = getopt(argc, argv, "io:")) != -1) {
        switch (opt) {
            case 'i': /*case insensitive*/
                ifnd++;
                break;
            case 'o': /*output file*/
                ofnd++;
                outfile = optarg;
                break;
            default: /* '?' */
                usage();
        }
    }

    if ((argc - optind) != 2) {
        fprintf(stderr, "number of positional arguments is not 2\n");
        exit(EXIT_FAILURE);
    }

    if (ifnd > 1) {
        fprintf(stderr, "Option ’i’ occurs more than once\n");
        exit(EXIT_FAILURE);
    }

    if (ofnd > 1) {
        fprintf(stderr, " Option ’o’ occurs more than once\n");
        exit(EXIT_FAILURE);
    }

    char *doc1 = argv[optind];
    char *doc2 = argv[optind+1];



    /** Open output file or open stdout * */
    FILE *output;
    if (ofnd == 1) {
        if ((output = fopen(outfile, "w")) == NULL) {
            fprintf(stderr, "[%s] ERROR: fopen failed: %s\n", myprog, strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        if ((output = fdopen(1, "w")) == NULL) {
            fprintf(stderr, "[%s] ERROR: fdopen failed: %s\n", myprog, strerror(errno));
            exit(EXIT_FAILURE);
        }

    }

    /*open both text files and start reading from them
     * inspect each line in the documents and print (in stdout or in a document)
     * the result from the comparisson*/
    FILE *file1;
    if ((file1 = fopen(doc1, "r")) == NULL) {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    FILE *file2;
    if ((file2 = fopen(doc2, "r")) == NULL) {
        fprintf(stderr, "fopen failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int char_diff=0;
    int line = 1;

    char *line1 = NULL; //here is stored the read line
    char *line2 = NULL;
    size_t len1=0; //initial size
    size_t len2=0;
    ssize_t nread1; //length of a line
    ssize_t nread2;

//compare each line and print in output file the result of the comparisson
    while (((nread1 = getline(&line1, &len1, file1)) != -1)
            && ((nread2 = getline(&line2, &len2, file2))!=-1)) {

               char_diff=0;

               int size = my_min(nread1, nread2);

        if(ifnd==1){  //case insensitive
            if(strncasecmp(line1,line2,size-1)!=0){
                for (int i = 0; i <size-1 ; i++) {
                    if(tolower(line1[i])!=tolower(line2[i])){
                        char_diff++;
                    }
                }
            }
        }else{  //case sensitive
            if(strncmp(line1,line2,size-1)!=0){
                for (int i = 0; i <size-1 ; i++) {
                    if(line1[i]!=line2[i]){
                        char_diff++;
                    }
                }
            }
        }if(char_diff!=0){
            fprintf(output, "Line: %d, charecters: %d\n",line,char_diff);
        }
        line++;

           }

            //free resources
           free(line1);
           free(line2);

           //close all streams
            if(ferror(file1)){
              fprintf(stderr, "fgets failed for the first file: %s\n",strerror(errno));
              exit(EXIT_FAILURE);
            }
            if(ferror(file2)){
              fprintf(stderr, "fgets failed for the second file: %s\n",strerror(errno));
              exit(EXIT_FAILURE);
            }

            if (fclose(file1) != 0) {
                fprintf(stderr, "fclose failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (fclose(file2) != 0) {
                fprintf(stderr, "fclose failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (fclose(output) != 0) {
                fprintf(stderr, "fclose failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

 exit(EXIT_SUCCESS);
    }
