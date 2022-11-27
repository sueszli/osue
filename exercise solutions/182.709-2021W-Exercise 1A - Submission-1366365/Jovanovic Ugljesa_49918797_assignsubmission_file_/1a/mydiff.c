#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>

/**
 * @brief prints usage
 *
 * @param program name
 */
static void usage(const char*);

/**
 * @brief main function
 *
 * @param argument count
 * @param argument vector
 * @return exit code
 */
int main(int argc, char **argv)
{
    const char* program = argv[0];
    int (*cmp)(const char*, const char*, size_t);
    FILE* out = stdout;
    FILE *f1, *f2;
    int opt;

    cmp = strncmp;
    while ((opt = getopt(argc, argv, "io:")) != -1) {
        switch (opt) {
            case 'i':
                cmp = strncasecmp;
                break;
            case 'o':
                out = fopen(optarg, "w");
                if (out == NULL) {
                    fprintf(stderr, "%s: Could not opne output file: %s\n", program, strerror(errno));
                    exit(EXIT_FAILURE);
                }
                break;
            default:
                usage(program);
                exit(EXIT_FAILURE);
        }
    }

    if ((optind + 2) > argc) {
        usage(program);
        exit(EXIT_FAILURE);
    }

    f1 = fopen(argv[optind], "r");
    if (f1 == NULL) {
        fprintf(stderr, "%s: Could not open first input file: %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    f2 = fopen(argv[optind + 1], "r");
    if (f2 == NULL) {
        fprintf(stderr, "%s: Could not open second input file: %s\n", program, strerror(errno));
        exit(EXIT_FAILURE);
    }

    int c1 = fgetc(f1);
    int c2 = fgetc(f2);
    int linenum = 1;
    int diff = 0;
    while(c1 != EOF && c2 != EOF)
    {
        if (c1 == '\n' || c2 == '\n') {
            if (diff > 0) {
                fprintf(out, "Line %d, characters: %d\n", linenum, diff);
            }

            if (c1 != '\n') {
                while (c1 != '\n' && c1 != EOF) c1 = fgetc(f1);
            }
            if (c2 != '\n') {
                while (c2 != '\n' && c2 != EOF) c2 = fgetc(f2);
            }

            linenum++;
            diff = 0;
        }

        {
            char tmp1 = (char)c1;
            char tmp2 = (char)c2;
            if (cmp(&tmp1, &tmp2, 1) != 0) {
                diff++;
            }
        }

        c1 = fgetc(f1);
        c2 = fgetc(f2);
    }
}

void usage(const char* program)
{
    fprintf(stderr, "SYNOPSIS:\n"
    "    %s [-i] [-o outfile] file1 file2\n", program);
}
