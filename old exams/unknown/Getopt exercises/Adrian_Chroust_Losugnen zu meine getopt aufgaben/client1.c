#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
/*
./client [-p PORT] {-g|-s VALUE} ID
PORT: long [1024,UINT16_MAX]
VALUE: int [0,127]
ID: int [0,63]
*/

char *program_name;

static int usage() {
    printf("[%s]: Usage error\n", program_name);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    program_name = argv[0];

    bool p = false, g = false, s = false, id = false;
    long p_val = -1, s_val = -1, id_val = -1;

    int c;
    while ((c = getopt(argc, argv, "-p:gs:")) != -1) {
        switch (c) {
            case 'p':
                if (p) usage();
                p_val = strtol(optarg, NULL, 10);
                if (p_val < 1024 || p_val > UINT_MAX) usage();
                p = true;
                break;
            case 'g':
                if (g || s) usage();
                g = true;
                break;
            case 's':
                if (g || s) usage();
                s_val = strtol(optarg, NULL, 10);
                if (s_val < 0 || s_val > 127) usage();
                s = true;
                break;
            case 1:
                if (id) usage();
                 id_val = strtol(optarg, NULL, 10);
                if (id_val < 0 || id_val > 63) usage();
                id = true;
                break;
            default:
                usage();
        }
    }

    if (!g && !s) usage();
    if (!id) usage();
    printf("p: %i, g: %i, s: %i\n", p, g, s);
    printf("p_val: %li, s_val: %li, id_val: %li\n", p_val, s_val, id_val);
    return EXIT_SUCCESS;
}