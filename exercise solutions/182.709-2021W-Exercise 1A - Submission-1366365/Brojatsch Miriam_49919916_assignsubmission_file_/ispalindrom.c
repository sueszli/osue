/**
* @file ispalindrom.c
* @author Miriam Brojatsch 11913096
* @date 12.11.2021
*
* @brief Program reads from specified input and for each line prints whether it is a palindrome or not to specified output.
* @details If no input file is specified, program reads from stdin, if no output file is specified (with -o) it prints to stdout.
* Options -i and -s indicate whether the line is handled either as case insensitive or without taking spaces into account.
**/



#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

/**
* @brief Prints message and writes to stderr.
* @param msg: message to be printed
**/
static int exit_on_error(char * msg) {
    fprintf (stderr, "Problem in ispalindrom: %s %s", msg, strerror (errno));
    exit(EXIT_FAILURE);
}

/**
* @brief Makes a copy of the word and only writes characters back to word if they aren't spaces. (deletes spaces)
* @param word: pointer to first char of word
**/
static void make_spaceless(char * word) {

    for (char * copy = word; *copy != '\0'; ++copy) {
        if (*copy != ' ') {
            *word = *copy;
            ++word;
        }
    }
    *word = '\0';
}

/**
* @brief Converts entire word to lowercase.
* @param word: pointer to first char of word
**/
static void make_caseinsensitive(char * word) {
    while (*word != '\0') {
        *word = tolower(*word);
        ++word;
    }
}

/**
* @brief Checks whether word is a palindrome by comparing the first and last char and incrementing/decrementing until they are the same char.
* Returns 0 if first and last don't match in at least one iteration.
* @param word: pointer to first char of word
**/
static int pure_palindrome(char * word) {
    char * last = word + strlen(word) -1;

    while (last >= word) {
        if (*last != *word) {
            return 0;
        }
        --last;
        ++word;
        
    }
    return 1;
}

/**
* @brief Iterates through a word and deletes newline.
* @param word: pointer to first char of word
**/
static void prepare_string(char * word) {
    if (word[strlen(word) - 1] == '\n') {
        word[strlen(word) - 1] = '\0';
    }
}

/**
* @brief Takes a word, options s and i and the desired output and prints whether it's a palindrome or not to that output.
* @details Checks if word is null, applies make_spaceless or make_caseinsesitive to word, if given in options and prints the result to output.
* @param word: pointer on the first char of the word that may be a palindrome or not
* @param s: 1 if word should be handled as case insensitive
* @param i: 1 if spaces in the word should be ignored
* @param output: where the result should be printed to
**/
static void allinclusive_palindrome(char * word, int s, int i, FILE * output) {
    char * workWord = strdup(word); //so original word can still be printed after possibly modifying it

    if (workWord == NULL) {
        exit_on_error("word is null");
    }
    
    if (s) {
        make_spaceless(workWord);
    }
    if (i) {
        make_caseinsensitive(workWord);
    }

    //delete newline from (probably the end of) both words to get one line of output
    prepare_string(workWord); 
    prepare_string(word);

    if (pure_palindrome(workWord)) {
        fprintf(output, "%s is a palindrom\n", word);
    } else {
        fprintf(output, "%s is not a palindrom\n", word);
    }

    free(workWord); //cleanup
}



/**
* @brief Main declares the necessary variables for options, input and output and initialises them with the values given by argv.
* If input is empty, it writes an error to stderr and returns 1. Otherwise allinclusive_palindrome is called on each line of the input.
* When there are no lines left, the stream is closed an the program returns 0.
* @param argc: argument counter
* @param argv: argument verctor
**/
int main(int argc, char **argv) {

    int s = 0;
    int i = 0;
    FILE * output = stdout;
    FILE * input;


    // @brief Gets options by using getopt and sets the indicators s, i and output declared in main accordingly.
    // If a wrong option is given, it writes an error to stderr and prints a correct usage message.
    char opt;
    while ((opt = getopt(argc, argv, "sio:")) != -1) {
        switch (opt) {
            case 's':
                    s = 1;
                    break;
            case 'i':
                    i = 1;
                    break;
            case 'o':
                    
                    output = fopen(optarg, "w");
                    break;
            default:
                    fprintf(stderr, "USAGE: %s [-s] [-i] [-o outfile] [file...]\n%s\n", argv[0], strerror (errno));
                    exit(EXIT_FAILURE);
        }
        
    }
    //Now optind (declared extern int by <unistd.h>) is the index of the first non-option argument. [https://stackoverflow.com/questions/9642732/parsing-command-line-arguments-in-c]
    

    if (optind >= argc) { //no non option arguments
        input = stdin;
    } else {
        input = fopen(argv[optind], "r");
    }
    

    if (input == NULL) {
        fprintf (stderr, "could not open: %s %s\n", argv[1], strerror (errno));
        return 1;
    }

    

    char * line = NULL;
    size_t len = 0;

    while (getline(&line, &len, input) != -1) { //call palindrome function on each line until there are no lines left
        allinclusive_palindrome(line, s, i, output);
    }

    fclose(input); //cleanup
    return 0;
}