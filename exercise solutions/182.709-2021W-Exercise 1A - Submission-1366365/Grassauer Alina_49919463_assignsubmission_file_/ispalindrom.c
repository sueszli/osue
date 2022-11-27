
/**
 * 
 * 
 * @file ispalindrom.c
 * @author Alina Grassauer 11905176
 * @date 14.11.2021
 * @brief BSUE Exercise 1a ispalindrom
 * @details 
 * 
 * 
 */

#define  _POSIX_C_SOURCE 200809L

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <string.h> // for errno


// is needed when we close the program 
volatile sig_atomic_t quit = 0;

/**
 * @brief in case we close the program with SIGINT or SIGTERM
 * 
 * @param signal 
 */

 static void handle_signal(int signal) { 
     quit = 1; }


static void checks_all( FILE * read_from, int s, int i, FILE * write_to);


static int is_palindrom(char * input, int s, int i);


/**
 * @brief the program name is saved in a variable because we need it for error meassages
 */

static char * program_name;



/**
 * @brief program gets argument which can be option on how to behave
 * it checks if a input, which can be files or from stdin, is a palindrom depending
 * on if th s,i options are set
 * 
 * @param argc number of elemenst in argv
 * @param argv array of command line arguments
 * @return int it indicates if the program ran sucessfully or if an error occurred
 */
int main(int argc, char **argv){

    program_name = argv[0];

    int opt_s = 0;
    int opt_i = 0;
    int opt_o = 0;
    FILE *o_value = stdout;
    int c;

    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        // no whitespaces
        case 's': 
            opt_s++;
            break;
        // all lowerCase
        case 'i': 
            opt_i++;
            break;
        //outputfile
        case 'o': 
            o_value = fopen(optarg, "w");
            if(o_value == NULL){
                fprintf(stderr, "%s: problem with outputfile %s", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }
            opt_o++;
            break;
        default:
            fprintf(stderr, "Usage: %s [-s] [-i] [-o outfile] [file...] \n", program_name);
            exit(EXIT_FAILURE);
            break;
        }
    }

    //in case of a SIGINT or SIFTERM call the program still writes in the outputfile
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    // no file to read from, means we take the input from the commandline
    if (optind == argc)
    {
        checks_all(stdin, opt_s, opt_i, o_value);
    }
    

    // goes through every non-option argument
    while (argv[optind] != NULL)
    {
        // gets the files we want to read from 

        FILE *input_file = fopen(argv[optind], "r");

        if (input_file == NULL)
        {
           fprintf(stderr, "%s : opening file failed: %s \n", program_name, strerror(errno));
           exit(EXIT_FAILURE);
        }
        
        checks_all(input_file, opt_s, opt_i, o_value);

        optind++;
    }


    exit(EXIT_SUCCESS);
}


/**
 * @brief reads line by line and for each line the program checks 
 * if it is a palindrom or not at writes it to t
 * getline reads the next line and writes it in the buffer, we do not have to fix a size because
 * getline automatially adjusts the size
 * if we have a newlien character "\n" we want to "cut it off" because we only want to have one line
 * after that we make a copy in case the original input gets altered, which we have to free after we are done
 * 
 * 
 * @param read_from reads the input from either a file or stdin
 * @param s s option means no whitespaces, can be 0 or 1
 * @param i i option means no case sensitivity, can be 0 or 1
 * @param write_to writes either to stdout or to a file, depending on the o option
 */

 static void checks_all( FILE * read_from, int s, int i, FILE * write_to)
    {
        char *buff = NULL;
        size_t buffsize = 0;

        while ((getline(&buff,&buffsize,read_from)!= -1) && !quit)
        {

            if(buff[ strlen(buff) - 1] == '\n'){	
			    buff[ strlen(buff) - 1] = '\0';
		    }
        
            char * c_input = strdup(buff);

            if(c_input == NULL){
                fprintf(stderr, "%s: problem with outputfile %s", program_name, strerror(errno));
                exit(EXIT_FAILURE);
            }

            if(is_palindrom(c_input, s, i) == 1){
                fprintf(write_to,"%s is palindrom \n", buff);

            }else if (is_palindrom(c_input, s, i) == 0){
                fprintf(write_to, "%s is no palindrom \n", buff);
            }

            free(c_input);
        }

        free(buff);
        fclose(write_to);
        
    }
    




/**
 * @brief puts the input to lowercase
 * 
 * @param input pointer to the original input
 */

static void to_lower_Case(char* input){

    for (size_t i = 0; input[i] != '\0'; i++)
    {
        input[i] = tolower(input[i]);
    }
    
}

/**
 * @brief deletes all spaces
 * if current char is not a space, we set the char of to current input to the index of j
 * therefor we skip the spaces when i is incremented
 * 
 * @param input pointer to the original input
 */


static void no_spaces(char* input){

    int i = 0, j = 0;

    while (input[i])
    {
        if (input[i] != ' ')
        {
            input[j++] = input[i];
        }
        i++;
    }
    input[j] = '\0';
    
}


/**
 * @brief checks if the input is a palindrom or not
 * 
 * @param input the input we want to check
 * @param s if s is not 0, we delete all spaces
 * @param i if i is not 0, we igonore the cases
 * @return int if the return value is 1, it means the input is a palindrom, 
 * if the return value is 0, it means it is not a palindrom
 */

static int is_palindrom(char * input, int s, int i){


    if (s > 0 )
    {
        no_spaces(input);
    }
    if (i > 0)
    {
        to_lower_Case(input);
    }
    
    int length = 0;

    while (input[length] != '\0'){
        length++;
    }

    int count = length/2;

    for (size_t i = 0; i < count; i++)
    {
        if (input[i] != input[(length-1) - i])
        {
            return 0;
        }
    }
     return 1;
     
}