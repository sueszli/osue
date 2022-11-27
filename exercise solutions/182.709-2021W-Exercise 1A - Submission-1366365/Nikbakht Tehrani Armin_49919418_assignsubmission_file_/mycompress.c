/**
 * @file mycompress.c
 * @author Armin Nikbakht-Tehrani 11928288
 * @brief 
 * This programm implements a not lossy compression algorithm.
 * It takes input from stdin or from one or more inputs, if specified.
 * It then writes the solution either to stdout or to a file, if one is given.
 * @version 0.1
 * @date 2021-11-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>

char *myprog;
/**
 * @brief When the user enters a wrong input, this message is being display, informing him
 *          of the correct input-syntax.
 * 
 */
void usage(void){
    fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * Programm entry point.
 * @brief This function takes in certain options with one argument. Then it performs the compression
 * on the input and returns it. It also prints out the compression-ratio and corresponding data.
 * 
 * @param argc : This parameter stores the number of argumets that were given to this function
 * @param argv : This parameter saves the parameters that were given to this function, so they can be accessed
 * @return int : Return values shows whether the programm finishes successfully or not
 */

int main(int argc, char **argv){
    myprog = argv[0];

    int c = 0;
    int opt_o = 0;
    char *arg_o = NULL;
    long int total_character_counter = 0;
    long int compressed_character_counter = 0;

    while( (c = getopt(argc, argv, "o:")) != -1 ){ //handle arguments
        switch (c) {
            case 'o':
                opt_o++; arg_o = optarg;
                break;
            case '?': 
                usage();
                exit(EXIT_FAILURE);
        }
    }


    /**
     * @brief Calls the usage function and exits if the user calls the programm
     * with the option o but does not include an argument or if he enters multiple output files
     * 
     */
    if ( (opt_o > 1) || ( (opt_o == 1) && (arg_o == NULL) ) ) {
        usage();
    }

    
    char *input_text = malloc(sizeof(char));
    char current_char;

    /**
     * @brief If we have one or more input files, we read the data from the files
     * and then store it in the variable "input_text"
     * 
     */
    if( argc - optind > 0 ){
        FILE *inputfile;
        int index = optind;

        int i;
        for(i = 0; i < argc - optind; ++i){
            if( (inputfile = fopen(argv[index], "r")) == NULL ){
                    fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }
            
            while( 1 ){
                current_char = fgetc(inputfile);

                if( current_char == '\0' ) break;
                if( (current_char == EOF) ) break;

                total_character_counter++;
                input_text = realloc(input_text, total_character_counter * sizeof(input_text));
                char write[] = {current_char, '\0'};
                strncat(input_text, write, 1);
            }
        }
    } else{
        FILE *inputfile;
        if( (inputfile = stdin) == NULL ){
                    fprintf(stderr, "fopen failed: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                }

        while( 1 ){
                current_char = fgetc(inputfile);

                if( (current_char == '\0') || (current_char == '\n') ) break;

                total_character_counter++;
                input_text = realloc(input_text, total_character_counter * sizeof(input_text));
                char write[] = {current_char, '\0'};
                strncat(input_text, write, 1);
            }
    }

    char *compressed_string = malloc(2 * total_character_counter * sizeof(char));
    compressed_string[0] = '\0';
    char current, prev;
    int counter = 0;
    
    /**
     * @brief Here the input string is compresesd. The occurences of a character is being counted
     * with the variable "counter". If a different character occurs, we concatinate the
     * string and the number, which we converted into a string beforehand, and concatinate
     * it to the output_text string. 
     */
    int j; 
    for(j=0; ; ++j){
        current = input_text[j];

        if( j == 0 ){
            prev = current;
        }

        if( current != prev ){
            
            char str[counter];
            sprintf(str, "%i", counter);
            char write[] = {prev, '\0'};
            strncat(compressed_string, write, 2);
            strncat(compressed_string, str, 2);
            counter = 1;
            prev = current;
        } else{
            counter++;
        }
        if( current == '\0') break;
    }

    /**
     * @brief The length of the compressed string is being calculated
     * 
     */
    int k;
    for(k=0; ; k++){
        char kek = compressed_string[k];
        

        if(kek == '\0') break;
        compressed_character_counter++;
    }

    /**
     * @brief If we have an outputfile, the compressed string is being written into it,
     * otherwise its written to stdout.
     */
    if( (opt_o == 1) && (arg_o != NULL) ){
        FILE *outputfile;

        if( (outputfile = fopen(arg_o, "w")) == NULL ){
            fprintf(stderr, "fopen failed: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        char read_char;
        int compressed_counter = 0;

        int l;
        for(l=0; ;l++){
            read_char = compressed_string[l];

            fputc(read_char, outputfile);
            compressed_counter++;

            if(read_char == '\0') break;

        }
        double ratio = (100) * (double)(compressed_character_counter / ((double)total_character_counter));
        ratio *= 10;
        ratio = round(ratio);
        ratio = ratio/10;

        fprintf(stderr, "Read: \t %ld characters\nWritten: %ld characters\nCompression ration: %.1f\n",
            total_character_counter, compressed_character_counter,ratio); 
        exit(EXIT_SUCCESS);
    }

    printf("\n%s\n", compressed_string);

    double ratio = (100) * (double)(compressed_character_counter / ((double)total_character_counter));
    ratio *= 10;
    ratio = round(ratio);
    ratio = ratio/10;

    fprintf(stderr, "Read: \t %ld characters\nWritten: %ld characters\nCompression ration: %.1f\n",
            total_character_counter, compressed_character_counter,ratio); 


    return EXIT_SUCCESS;
}