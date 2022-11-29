#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

/** name of the executable (for printing messages) */
char *program_name = "<not yet set>";

void usage(const char *message);

int main(int argc, char **argv)
{

    /* set program_name */
    if (argc > 0) {
        program_name = argv[0];
    }

    /*

    - ./client [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]

    - hint: optargC can only be specified by -coptargC (not -c optargC)

    - all optargs are char* (for simplicity)
    - hardest test-cases:
        - -d specified, although -b not specified (FAIL)
        - -d not specified, -b specified (PASS)

    - 

    */

    /*
    What I would have done:

    min argc: 2 (program name + -c)
    max argc: [8]
    check:
    if (argc < 2)
    if (argc > 9)

    getopt for -a:
    case 'a':
        if
    */


    /*
    getopt recall plan:

    flags at the start of the program:
     - one flag for each option
        - check that flag after getopt, e.g. whether the required option appeared,
            or inside getopt, whether the same option came twice
     - strtol to parse string to integers
     - get that - in front of argstring
    while getopt not -1


     - check strtol in a very weird way!! There is so much that can go wrong...

    file args: with case 1

    default case

    types: XOR options, conditional option argument

    optarg holds the pointer to the option value

    HOW to check for empty option if no value possible
    */

    printf("a: %s\n",a);
    printf("b: %s\n",b);
    printf("c: %s\n",c);
    printf("d_set: %s\n",d_set ? "true" : "false");
    printf("e_set: %s\n",e_set ? "true" : "false");

    return 0;
}


/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message){
    fprintf(stderr,"%s\n",message);
    fprintf(stderr,"Usage: %s [-a optargA] [-e] -c [optargC] [-b optargB [-d] ]\n",program_name);
    exit(EXIT_FAILURE);
}


