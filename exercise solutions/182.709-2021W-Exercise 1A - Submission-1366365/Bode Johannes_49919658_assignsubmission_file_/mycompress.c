/**
 * @file mycompress.c
 * @author Johannes Bode <e12025979@student.tuwien.ac.at
 * @date 07.11.2021
 * @brief Main programm
 * @details If option -o has ben written, the output of this program will be written to an outputfile. If there is
 * another file named like the outputfile it will exit. If non such file exist, the file will be created. If there is
 * no option -o * the output will be written in stdout. If there are inputfiles are given the program will read all
 * files, compress them one by one. When there are none inputfiles the program reads from stdin. Then the conclusion
 * is output how many characters were read and how many were written including the calculated compression rate.
**/


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define MAXIMUM_SIZE 	100

/**
 * global Variables
 * @details myprog (my program name)
 * outputfile_flag: outputfile successfully created
 * rd_counter, wr_counter: to calculate the compression
**/
char *myprog;
FILE *outfile;
bool outfile_flag;
int rd_counter = 0;
int wr_counter = 0;

/**
 * @brief This function will close the outfile, if the outfile has been opened/created.
 * @details gloabl variable: myprog (my program name)
 * function: fclose(x) returns 0 by success
 * @return Error Messsage in stderr if fclose() failed.
**/
static void rm_resources(void) {
    if(outfile != NULL) {
        if(fclose(outfile) != 0) {
            fprintf(stderr, "%s: fclose() failed - %s\n", myprog, strerror(errno));
        }
    }
}

/**
 * @brief This function provides helpful arguments for using the program in stderr.
 * @details global variable: myprog (my program name)
 * @return Exit programm with EXIT_FAILURE (unsuccessful termination).
 **/
static void invalid_usage(void) {
    rm_resources();
    fprintf(stderr, "Usage: %s [-o outfile] [infile] [infile1] [infile...] \n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Error message function.
 * @brief This function writes usefull error messages in stderr.
 * @details global variable: myprog (my program name)
 * @param error: Message which error occurred.
 * @return Exit programm with EXIT_FAILURE (unsuccessful termination).
 **/
static void error_msg(char *error) {
    rm_resources();
    fprintf(stderr, "%s: %s - %s\n", myprog, error, strerror(errno));
    exit(EXIT_FAILURE);
}


/**
 * Compressing the input from a stream.
 * @brief This function compress the file. If the function reads a character more than once in a row,
 * it will be compressed in the specific character and the number of times the character has ben read.
 * When the output_flag has been set, the function will write in the Outputfile. Otherwise it will write in stdout.
 * And count the characters of the input and characters of the output.
 * @param stream is the input which should be compressed
 **/
void compressing(FILE *stream) {
    char buffer[MAXIMUM_SIZE];
    char allChars[MAXIMUM_SIZE];
    int i = 0;
    while(fgets(buffer, sizeof(buffer), stream) != NULL) {	/* move all chars into one array */			
        int j = 0;
        while (j < strlen(buffer)){
            allChars[i] = buffer[j];
            i++;
            j++;
        }
    }
	rd_counter = i;											/* already read all chars from the inputfile */			
    char prevChar = '\0';
    int count = 0;
    for (int k = 0; k <= i ; k++) {
        char thisChar = allChars[k];
        if (k == 0){
            prevChar = thisChar;
            count = 1;
        }else{
            if (prevChar == thisChar){
                count++;
            }else{
                if(outfile_flag) {							/* if the outputfile is set write in the file otherwise to stdout */
                    fprintf(outfile, "%c%d", prevChar, count);
                    wr_counter+=2;
                } else {
                    fprintf(stdout, "%c%d", prevChar, count);
                    wr_counter+=2;
                }
                prevChar = thisChar;
                count = 1;
            }
        }
    }
    if(ferror(stream)) {									/* ferror checks if the End-Of-File_Indikator has been set*/
        error_msg("fgets failed");
    }
}
/**
 * The main program.
 * @details global variable: myprog, wr_counter, rd_counter, outfile, outfile_flag
 * @param argc: argument counter
 * argv: argument vector
 * @return Exit programm on success with EXIT_SUCCESS.
 **/
int main(int argc, char *argv[]) {
    if(argc > 0) {
        myprog = argv[0];
    }

    char *file_name;
    outfile_flag = false;
    int oflag = 0;
    int option;

    /* getopt() evaluates the command line */
    while((option = getopt(argc, argv, "o:")) != -1) {
        switch (option) {
            case 'o':
                oflag++;
                if (oflag > 1) {                             /* only one option is allowed */
                    invalid_usage();
                    break;

                }
                file_name = optarg;
                outfile_flag = true;
                break;

            /* if getoppt() get an option character that is not contained in optstring it returns an question-mark */
            case '?':
                invalid_usage();
                break;

            default:

                break;
        }
    }

    if(outfile_flag) {                                      /* check if a outfile name has been set */
        FILE *temp = fopen(file_name, "r");          		/* check if there is already an file named like file_name */
        if(temp == NULL) {                                  /* if the file doesn't exist it can be created */
            outfile = fopen(file_name, "w");
        } else {
            fprintf(stderr, "mycompress: file %s already exists\n", file_name);
            if(fclose(temp) != 0) {                         /* close the existing file */
                error_msg("fclose failed");
            }
            exit(EXIT_FAILURE);
        }
    }

    if(argv[optind] != NULL) {                              /* check if any inputfiles has been decleared */
        for(int i = optind; i < argc; i++) {
            FILE *infile = fopen(argv[i], "r");
            if(infile == NULL) {                            /* check if the inputfile exist */
                fprintf(stderr, "mycompress:  the inputfile %s doesn't exists\n", argv[i]);
                exit(EXIT_FAILURE);
            } else {
                compressing(infile);
            }
            if(fclose(infile) != 0) {
                error_msg("fclose failed");
            }
        }
    } else  {
        compressing(stdin);
    }

    /* calc the compression rate */
    float rate = 0.0;
    if(rd_counter != 0) {                                   /* (avoid division by zero) */
        if(wr_counter != 0) {
            if(outfile_flag) {
                fprintf(outfile, "\n");
            } else {
                fprintf(stdout, "\n");
            }

            float compress_rate = ((float) wr_counter) / ((float) rd_counter);
            int temp = (compress_rate * 10000);
            rate = temp / 100.0;
        }
    }

    /* print compression rate to stderr */
    fprintf(stderr, "Gelesen:\t\t%d Zeichen\nGeschrieben:\t\t%d Zeichen\nKomprimierungsrate:\t%.1f %%\n", rd_counter, wr_counter, rate);
    rm_resources();
    return EXIT_SUCCESS;
}

