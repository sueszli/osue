#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef DEBUG
#define log(fmt, ...) \
    fprintf(stderr, "[%s:%d] " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__);
#else
#define log(msg) /* NOP */
#endif

#define error(s)                                     \
    fprintf(stderr, "%s: %s\n", s, strerror(errno)); \
    exit(EXIT_FAILURE);

#define argumentError(s)                                                       \
    fprintf(stderr, "%s\n", s);                                                \
    fprintf(stderr, "%s\n", "---");                                            \
    fprintf(stderr, "%s\n", "SYNOPSIS:");                                      \
    fprintf(stderr, "\t%s\n", "ispalindrom [-s] [-i] [-o outfile] [file...]"); \
    exit(EXIT_FAILURE);

static char *trim(char *line) {
    char *out = strdup(line);

    // copy valid chars to duplicate and end with '\0'
    char *op = out;
    char *ip = line;
    while (*ip != '\0') {
        if (!isspace(*ip)) {
            *op = *ip;
            op++;
        }
        ip++;
    }
    *op = '\0';

    // free everything following the '\0' (optional)
    // https://stackoverflow.com/questions/74350465/how-to-free-memory-following-a-0-placed-somewhere-in-a-string-in-c
    char *newMemory = realloc(out, strlen(out) + 1);
    if (newMemory) {
        out = newMemory;
    } else {
        error("Realloc failed");
    }

    return out;
}

static char *toLowerCase(char *line) {
    char *out = strdup(line);

    char *p = out;
    while (*p != '\0') {
        *p = tolower(*p);
        p++;
    }
    return out;
}

static _Bool isPalindrom(char *line) {
    const size_t length = strlen(line);
    const size_t halfLength = length >> 1;

    if (length == 0) {
        return false;
    }

    for (size_t i = 0; i < halfLength; i++) {
        char left = line[i];
        char right = line[length - (1 + i)];
        if (left != right) {
            return false;
        }
    }
    return true;
}

static void printArgs(int argc, char **argv) {
    log("argc: %d", argc);
    log("%s", "argv:");
    char **ap = argv;
    uint8_t i = 0;
    while (*ap != NULL) {
        log("\t[%d]: %s", i++, *ap);
        ap++;
    }
    log("%s", "\n\n\n");
}

static void testPalindrom(void) {
    char *input = "never odd or even";
    printf("\"%s\" ", input);

    _Bool ignoreLetterCasing = true;
    _Bool ignoreWhitespaces = true;

    if (ignoreLetterCasing) {
        input = toLowerCase(input);
    }
    if (ignoreWhitespaces) {
        input = trim(input);
    }

    if (isPalindrom(input)) {
        printf("is a palindrom\n");
    } else {
        printf("is not a palindrom\n");
    }
    printf("\n\n\n");
}

int main(int argc, char **argv) {
    printArgs(argc, argv);

    uint8_t ignoreLetterCasing = 0;
    uint8_t ignoreWhitespaces = 0;
    uint8_t writeToFile = 0;
    char *outputPath;

    int option;
    struct option config[] = {{"outfile", required_argument, 0, 'o'}};
    while (true) {
        int option_index = 0;
        option = getopt_long(argc, argv, "sio:", config, &option_index);

        if (option == -1) {
            break;  // not in switch to avoid goto statement
        }
        switch (option) {
            case 's':
                ignoreWhitespaces++;
                break;
            case 'i':
                ignoreLetterCasing++;
                break;
            case 'o':
                writeToFile++;
                outputPath = optarg;
                break;
            default: /* '?' */
                argumentError("Invalid option was used.");
        }
    }

    // -------------------------

    if (ignoreLetterCasing > 1 || ignoreWhitespaces > 1 || writeToFile > 1) {
        argumentError("The same option was used twice or more.");
    }

    _Bool readFromFiles = optind < argc;
    if (readFromFiles) {
        // read input from files
        log("%s", "input will be read from the following files:");
        while (optind < argc) {
            log("\t%s ", argv[optind++]);
        }
    } else {
        // let user enter input
    }

    if (ignoreWhitespaces) {
        log("%s", "'s' option -> whitespaces will be ignored");
        // apply function to input
    }
    if (ignoreLetterCasing) {
        log("%s", "'i' option -> casing will be ignored");
        // apply function to input
    }

    // do computation

    if (writeToFile) {
        log("%s: %s", "'o' option -> result will be written into output path",
            outputPath);
        // write output into 'outputPath'
    } else {
        // print output onto console
    }
    return EXIT_SUCCESS;
}
