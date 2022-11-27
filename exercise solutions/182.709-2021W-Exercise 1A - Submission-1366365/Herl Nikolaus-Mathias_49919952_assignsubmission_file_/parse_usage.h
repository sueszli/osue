#include <stdio.h>

// todo:
// struct containing all the parsed inputs (/pointers)
typedef struct Input_Str {                 
    FILE *outFILE;
    FILE **inFILEs;
    int n_infiles;
} Input;

Input parse_usage(int argc, char **argv);

