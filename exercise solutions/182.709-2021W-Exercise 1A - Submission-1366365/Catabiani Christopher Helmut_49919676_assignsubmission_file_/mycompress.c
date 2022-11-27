/**
 * @file mycompress.c
 * @author Christopher Catabiani (e11776169@student.tuwien.ac.at)
 * @date 2021-11-14
 * 
 * @brief Main program module.
 *
 * This program counts the same consecutive chars of the input and compresses them to <char|amount>, i.e "a5"
 */


#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

/**
 * definition of the structure STATISTICS and its variables.
 */
typedef struct 
{
    int read;
    int written;
}STATISTICS;

STATISTICS stats = {0,0};


/**
 * Mandatory usage function.
 *
 * @brief This function writes helpful usage information about the program.
 */
void usage(){
    fprintf(stderr, "mycompress [-o outfile] [file...]");
    exit(EXIT_FAILURE);
}

/**
 * @brief read the input from the terminal/file and count same consecutive characters and compress them,
 *        also counts read/written characters for the statistics
 * @param in the input source, either stdin or file/s
 * @param out the output destination, either stdout or a file
 * @return bool sensitive The boolean that determines if the comparison is case sensitive or not.
 */
void countConsecutive(FILE* in, FILE* out){
    int charCounter = 0;
    char currentChar;

    while(1){ //while char isn't EOF or error occurs, do
        char c = fgetc(in);

        if(c == EOF){
            break;
        }

        stats.read++;

        if(charCounter == 0){ // first char ever read
            charCounter++;
            currentChar = c;
        } else if(c == currentChar){ // matching char 
            charCounter++;
        } else { // different chars
            int write = fprintf(out, "%c%d", currentChar, charCounter);
            if(write == -1){
                fclose(in);
                fclose(out);
                exit(EXIT_FAILURE);
            }
            stats.written += write;
            currentChar = c;
            charCounter = 1;
        }

        
    }
    int write = fprintf(out, "%c%d",currentChar, charCounter);
    if(write != -1){
        stats.written += write;
    }
}

/**
 * Program entry point.
 * @brief The program starts here. This function checks the options given in the command line,
 * initializes the input-source/s and output-destination and
 * calls the countConsecutive function and compresses the input.
 * 
 * @param argc The argument counter
 * @param argv The argument vector
 * @return Returns EXIT_SUCCESS on successful execution, else EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    
    int option;
    int opt_o = 0;

    FILE* out = NULL; 
    // Read options and check if output file valid
    while ((option = getopt(argc, argv, "o:")) != -1){
        switch (option) {
            case 'o':
                if(opt_o == 1){
                    fclose(out);
                    fprintf(stderr, "Max. 1 Output File!");
                    usage();
                    break;
                }
                
                out = fopen(optarg, "w");
                opt_o++;
               
                if(out == NULL){
                    fprintf(stderr, "Output File could not be opened!");
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                usage();
                break;
        }
    }
    if(opt_o == 0){
        out = stdout;
    }
    
    //check if input from terminal or file/s
    FILE* in;
    if(optind == argc){ // take input from terminal
        in = stdin;
        countConsecutive(in, out);     
    } else { // take input from file/s
        while (optind < argc){
            in = fopen(argv[optind], "r");
            if(in == NULL){ //if file can't be opened
                fprintf(stderr,"Could not open file %s", argv[optind]);
                fclose(out);
                exit(EXIT_FAILURE);
            } else { // if input-file is valid
                countConsecutive(in, out);
            }
            optind++;
        }
        fclose(in);
        fclose(out);
        float ratio = ((double) stats.written / stats.read) * 100.0;
        
        fprintf(stderr, "Read: %d characters\nWritten: %d characters\nCompression ratio: %.1f%%\n", stats.read, stats.written,ratio);
        exit(EXIT_SUCCESS);
    }
}



