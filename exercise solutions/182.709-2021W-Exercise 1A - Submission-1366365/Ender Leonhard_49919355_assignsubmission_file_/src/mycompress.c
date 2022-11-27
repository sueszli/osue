/**
 * @file mycompress.c
 * @author Leonhard Ender (12027408)
 * @date 13.11.2021
 *
 * @brief A basic algorithm for compressing text.
 * 
 * The program takes text as input (either from a file or
 * stdin), compresses it by substituting subsequent identical
 * characters by only one ocurrence of the character followed
 * by the number of characters (e.g. replace aaa with a3).
 * Usage: mycompress [-o outfile] [file]
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/** A global variable holding the name of the program. */
char *myprog;

/**
 * @brief Print the usage to stderr and exit.
 * @details global variables: myprog
 */
static void usage(void) {
    fprintf(stderr,"Usage: %s [-o outfile] [file]\n", myprog);
    exit(EXIT_FAILURE);
}

/**
 * @brief Print the error to stderr and exit.
 * @details Print out the program name, followed by
 * the err passed to the function and the string
 * representation of the errorcode in errno.
 * 
 * global variables: myprog
 */
static void print_error(char *err) {
    fprintf(stderr, "%s: %s: %s\n", myprog,err, strerror(errno));
}

/**
 * @brief Return the contents of a file as a string.
 * @details Read the file character by character and
 * write into a buffer that is expanded when needed.
 * @param f The input file (stream).
 */
static char *read_file(FILE *f) {
    // buffer size is initially 100
    int c, bufsize = 100, n = 0;
    char *buf = malloc(bufsize);
    if (buf==NULL) {
        print_error("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    // read from input until encountering end of file
    while ((c = fgetc(f)) != EOF) {
        // if buffer is almost full, expand it
        if (n >= bufsize -10) {
            bufsize+= 100;
            buf = realloc(buf, bufsize);
            if (buf==NULL) {
                print_error("realloc failed");
                exit(EXIT_FAILURE);
            }
        }
        buf[n] = c;
        n++;
    }
    buf[n]='\0'; // append string termination character
    
    return buf;
}

/**
 * @brief Compress the input string.
 * @details Replace subsequent identical characteres
 * by one occurence of the character followed by the
 * number of sequential appearences (e.g. replace aaa
 * with a3). The input string is expected to include 
 * the string termination character (\0), otherwise
 * behavior is undefined. 
 * @param input The input string
 * @return The compressed version of the input string.
*/
static char *compress(char *input) {
    /* the result can be at most twice as long as the input,
       therefor a buffer of that size is required. */    
    char *buf = malloc(strlen(input)*2+1);
    if (buf==NULL) {
        print_error("malloc failed");
        exit(EXIT_FAILURE);
    }
    
    int char_count = 0; // counter for occurences of a character
    char current_char = *input; 
    *buf = '\0'; // "empty" the buffer

    /* a temporary string for printing the number
       can hold string representations of numbers
       with up to 11 digits, enough for 32bit int.
    */
    char n[12]; 
    
    // scan input until encountering string termination
    while (*input != '\0') {
        // character is identical to the previous one
        if (*input == current_char) {
            char_count++;
        }
        // character is different from the previous one
        else {
            // print string representation of number of occurences to n
            sprintf(n, "%c%d",current_char, char_count); 
            // append character + occurences to output string
            strcat(buf, n);
            char_count = 1;
            current_char = *input;
        }
        
        input++;
    }
    
    // process last character
    sprintf(n, "%c%d",current_char, char_count);
    strcat(buf, n);
    char_count = 1;
    current_char = *input;
    
    return buf;
}


/**
 * @brief Program entry point.
 * @details Parse input parameters, read input file,
 * compress it, write it to ouput file.
 * Valid parameters are [-o outfile] (specifying the output
 * file) and [file] (specifying the input file). Both are optional,
 * if not given, stdout and stdin are used, respectively.
 *
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Return EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
    myprog = argv[0]; // program name

    // input and ouput files
    char *outfile_path = NULL;
    char *infile_path = NULL;
    FILE *outfile = NULL;
    FILE *infile = NULL;
        
    // parse input parameters
    int c;
    while ( (c = getopt(argc, argv, "o:")) != -1 ){
        switch ( c ) {
            case 'o':
                outfile_path=optarg;
                break;
            default:
                usage();
                break;
        }
    }
    
    // no input file was given - use stdin
    if (argc == optind) {
        infile = stdin;
    }
    // input file was given - try to open it
    else if (argc == optind+1) {
        infile_path = argv[optind];
        infile = fopen(infile_path,"r");
        if (infile==NULL) {
            print_error("fopen failed");
            exit(EXIT_FAILURE);
        }
    }
    // too many parameters
    else {
        usage();
    }
    

    // output file was specified: open file
    if (outfile_path != NULL) {
        outfile = fopen(outfile_path,"w");
        if (outfile==NULL) {
        print_error("fopen failed");
            exit(EXIT_FAILURE);
        }
    }
    // output file was not specified - use stdout
    else{
        outfile=stdout;
    }

    // read input file and compress it
    char *input_string = read_file(infile);
    char *output_string = compress(input_string);

    // close infile (if existant)
    if (infile != stdin) {
        fclose(infile);
    }
    
    // write result to output file
    fprintf(outfile, "%s", output_string);
    
    // print number of character read/written an compression rate to stderr
    fprintf(stderr, "Read: \t %ld characters\n", strlen(input_string));    
    fprintf(stderr, "Written: \t %ld characters\n", strlen(output_string));
    fprintf(stderr, "Compression ratio: %f %%\n", (float)strlen(output_string)/strlen(input_string)*100);

    // free allocated memory
    free(input_string);
    free(output_string);
    
    // close outfile (if existant)
    if (outfile != stdout) {
        fclose(outfile);
    }
    
    return EXIT_SUCCESS;
}

