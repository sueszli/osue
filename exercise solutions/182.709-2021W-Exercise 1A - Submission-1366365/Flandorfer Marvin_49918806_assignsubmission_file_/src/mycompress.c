/**
 * Main module
 * @file mycompress.c
 * @author Marvin Flandorfer, 52004069
 * @date 25.10.2021
 * 
 * @brief Main program module.
 * This module is the entry point of the program.
 * 
 * @details This module covers the structure of the whole process (read input -> compress input -> output compressed input) and the compress procedure itself.
 * It is also responsible for the exit of the program in every case (whether due to failure or success).
 * */

#include <getopt.h>
#include "filein.h"
#include "fileout.h"
#include "misc.h"

char *program_name; /**< The program name*/

/**
 * Compress function
 * @brief This function compresses the input and returns the result as a pointer.
 * @details The compression algorithm used substitutes subsequent identical characters with only one occurence of the character followed by the number of characters.
 * On success it returns a char pointer with previously allocated memory (using malloc) that needs to be freed after usage.
 * On failure a null-pointer will be returned, an error message will be written (using error_message from misc) and all (internally) allocated memory will be freed.
 * 
 * @param str_to_compress Char pointer pointing to input string that needs to be compressed.
 * @return On success char pointer with allocated memory will be returned. On failure null-pointer will be returned.
 * */
static char *compress(char *str_to_compress){
    char c1 = str_to_compress[0];                       /**< Character for comparison*/
    char c2;                                            /**< Character for comparison*/
    char *tmp = NULL;
    int counter = 0;                                    /**< Counter of subsequent identical characters*/
    int str_length = strlen(str_to_compress);           /**< Length of str_to_compress*/
    char *compressed_str = malloc(str_length*2+1);      /**< Compressed string*/
    if(!compressed_str){
        error_message("malloc");
        return NULL;
    }
    compressed_str[0] = '\0';
    for(int i = 0; i <= str_length; i++){
        c2 = str_to_compress[i];
        if(c1 == c2){
            counter++;
        }
        else if(c1 != c2 || i == str_length){
            char num[12];
            char character[2] = "\0";
            character[0] = c1;
            int a = sprintf(num,"%d",counter);
            if(a < 0){
                error_message("sprintf");
                free(compressed_str);
                return NULL;
            }
            (void) strcat(compressed_str,character);
            (void) strcat(compressed_str,num);
            if(i != str_length){
                c1 = c2;
                counter = 1;
            }
        }
    }
    tmp = realloc(compressed_str, strlen(compressed_str)+1);
    if(tmp == NULL){
        error_message("realloc");
        free(compressed_str);
        return NULL;
    }
    else{
        compressed_str = tmp;
    }
    return compressed_str;
}

/**
 * Function for the summarization of the output
 * @brief Summarizes the output and writes read character, written characters and the compression ratio to stderr.
 * @details On failure error message will be written (via error_message).
 * 
 * @param read_chars Number of characters read from input.
 * @param written_chars Number of characters written to output.
 * @return On success returns 0. On failure returns -1.
 * */
static int summarize_output(int read_chars, int written_chars){
    int a = fprintf(stderr, "Read:\t\t%i characters\n",read_chars);
    if(a < 0){
        error_message("fprintf");
        return -1;
    }
    a = fprintf(stderr, "Written:\t%i characters\n",written_chars);
    if(a < 0){
        error_message("fprintf");
        return -1;
    }
    a = fprintf(stderr, "Compression ratio:\t%.1f %%\n", (double) written_chars/read_chars*100);
    if(a < 0){
        error_message("fprintf");
        return -1;
    }
    return 0;
}

/**
 * Entry point of the program.
 * @brief Main function that structures the whole procedure and processes the call options and arguments.
 * @details This function ist the only funtion in the program that is allowed to exit the program (except the usage function).
 * It also covers the majority of memory management (especially freeing memory). 
 * The ouput to stdout is also covered here.
 * @details global variables: program_name
 * 
 * @param argc Argument counter
 * @param argv Argument vector
 * @return Returns EXIT_SUCCESS
 * */
int main(int argc, char *argv[]){
    int c;                              
    int read_chars;                     /**< Number of read characters*/
    int written_chars;                  /**< Number of written characters*/
    int opt_o = 0;                      /**< Option counter for option -o*/
    int pos_arg = 0;                    /**< Counter for positional arguments*/
    char *outfile_name = NULL;          /**< Name/Path of the outfile*/
    char *infile_name = NULL;           /**< Name/Path of the current infile*/
    char *to_compress = NULL;           /**< String that needs to be compressed*/
    char *compressed = NULL;            /**< Compressed string*/
    program_name = argv[0];
    while((c = getopt(argc, argv, "o:")) != -1){
        switch(c){
            case 'o':
            outfile_name = optarg;
            opt_o++;
            break;
            default: /* '?' */
            usage();
        }
    }
    pos_arg = argc - optind;
    if(opt_o > 1){
        usage();
    }
    if(pos_arg > 0){
        for(int i = 0; i < pos_arg; i++){
            infile_name = argv[optind+i];
            char *s = read_content_from_file(infile_name);
            if(!s){
                free(to_compress);
                exit(EXIT_FAILURE);
            }
            char *t = concat(to_compress,s);
            free(s);
            free(to_compress);
            if(!t){
                exit(EXIT_FAILURE);
            }
            to_compress = malloc(strlen(t)+1);
            if(!to_compress){
                error_message("malloc");
                free(t);
                exit(EXIT_FAILURE);
            }
            (void) strcpy(to_compress,t);
            free(t);
        }
    }
    else{
        to_compress = read_content_from_stdin();
        if(!to_compress){
            exit(EXIT_FAILURE);
        }
    }
    read_chars = strlen(to_compress);
    compressed = compress(to_compress);
    free(to_compress);
    if(!compressed){
        exit(EXIT_FAILURE);
    }
    written_chars = strlen(compressed);
    if(opt_o == 0){
        int a = fprintf(stdout, "%s\n", compressed);
        if(a < 0){
            error_message("fprintf");
            free(compressed);
            exit(EXIT_FAILURE);
        }
    }
    else if(opt_o == 1){
        int a = write_into_file(outfile_name, compressed);
        if(a < 0){
            free(compressed);
            exit(EXIT_FAILURE);
        }
    }
    free(compressed);
    int a = summarize_output(read_chars, written_chars);
    if(a < 0){
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}