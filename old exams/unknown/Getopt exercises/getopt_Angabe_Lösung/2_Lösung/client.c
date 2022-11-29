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


    // - ./client [-a optargA | -b optargB | -o ] -c [optargC] file... (max. 8 files)

    // - hint: optargC can only be specified by -coptargC (not -c optargC)

    // - optargA ... int [-50,300]
    // - optargB ... char
    // - optargC ... char[8] (exactly!)
    // - maximum of 8 pos-args, no minimum, cat all to one total_string


    if (argc < 2) usage("Too few argv.");
    if (argc > 11) usage("Too many argv.");

    bool a_occured = false;
    bool b_occured = false;
    bool c_occured = false;
    bool o_occured = false;

    int a = -51; // printf only if a != NULL
    char *b = NULL;
    char *c = NULL;
    bool o_set = false;
    char total_string[2048] = "";
    int posArgCount = 0;

    char c1;
    while ((c1 = getopt(argc,argv,"-a:b:c::o")) != -1){
        switch (c1){
            case 'a':
                if(a_occured) usage("passed in -a more than once.");
                if(b_occured || o_occured) usage("Specify exactly one of a|b|o as option.");
                a_occured = true;
                a = strtol(optarg,NULL,10); // missing the nptr == endptr edge case
                if (a < -50 || a > 300) usage("a element [-50,300]");
                break;
            case 'b':
                if(b_occured) usage("passed in -b more than once.");
                if(o_occured || a_occured) usage("Specify exactly one of a|b|o as option.");
                b_occured = true;
                b = optarg;
                break;
            case 'c':
                if(c_occured) usage("passed in -c more than once.");
                c_occured = true;
                c = optarg; // todo: test what happens, if optargC was not specified
                if (c != NULL) {
                    if (strlen(c) != 8) usage("strlen(optargC) must == 8.");
                }
                break;
            case 'o':
                if(o_occured) usage("passed in -o more than once.");
                if(a_occured || b_occured) usage("Specify exactly one of a|b|o as option.");
                o_occured = true;
                o_set = true;
                break;
            case 1:
                posArgCount++;
                if (posArgCount > 8) usage("too many posargs!");
                strcat(total_string,optarg);
                break;
            default:
                usage("(getopt detected an error, see description above.)");
        }
    }

    // if(!a_occured && !b_occured && !c_occured) usage("Specify exactly one of a|b|o as option.");
    if(!c_occured) usage("c must occur");

    if (a != -51) printf("a: %d\n",a);
    else printf("a: not initialized yet.\n");
    printf("b: %s\n",b);
    printf("c: %s\n",c);
    printf("o_set: %s\n", o_set ? "true" : "false");
    printf("total_string: %s\n",total_string);

    return 0;
}


/** Prints the usage message and exits with EXIT_FAILURE. */
void usage(const char *message){
    fprintf(stderr,"%s\n",message);
    fprintf(stderr,"Usage: %s [-a optargA | -b optargB | -o ] -c [optargC] file... (max. 8 files)\n",program_name);
    exit(EXIT_FAILURE);
}


