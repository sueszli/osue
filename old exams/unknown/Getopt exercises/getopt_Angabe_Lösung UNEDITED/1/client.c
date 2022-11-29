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


