/// @author 12024014
/// @name mygrep
/// @brief used as a mini grep to parse lines containing a keyword
/// @date 10.11.2021

#define  _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <getopt.h>

/// @brief str to lowercase
/// @param str pointer to string which shall be lowercased
char *str_to_lower(char *str)
{
    size_t len = strlen(str);
    char *str_l = calloc(len + 1, sizeof(char));

    for (size_t i = 0; i < len; ++i)
    {
        str_l[i] = tolower((unsigned char)str[i]);
    }
    return str_l;
}

/// @brief write the str to out
/// @param str string which shall be printed
/// @param out if NULL output to stdout else output to file in out
void write_to_out(char *str, char *out)
{
    if (out == NULL)
    {
        printf("%s", str);
    }
    else
    {
        FILE *pFile = fopen(out, "a");
        fprintf(pFile, "%s", str);
        fclose(pFile);
    }
}

/// @brief if the str contains the keyword it will be written either to the specified out-file or stdout
/// @param str str which could contain keyword
/// @param keyword word to search for
/// @param case_sensitive if search should be case_sensitive
/// @param out output written to file or stdout
void output_str_contains(char *str, char *keyword, bool case_sensitive, char *out)
{
    char *str2 = case_sensitive ? str : str_to_lower(str);

    if (strstr(str2, keyword) != NULL)
    {
        write_to_out(str, out);
    }
}

int main(int argc, char *argv[])
{
    int opt;

    bool case_sensitive = true;
    char *out = NULL;

    //Parse arguments
    while ((opt = getopt(argc, argv, "io:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            case_sensitive = false;
            break;
        case 'o':
            out = optarg;
            break;
        default:
            assert(0);
        }
    }

    if ((argc - optind) == 0)
    {
        fprintf(stderr, "%s: No keyword specified! Use:\n%s [-i] [-o outputfile] keyword [inputfile ...]\n", argv[0], argv[0]);
        return EXIT_FAILURE;
    }

    char *keyword = case_sensitive ? argv[optind++] : str_to_lower(argv[optind++]);

    //Read input from stdin or from specified files
    if (argv[optind] == NULL)
    {
        for (;;)
        {
            char *line;
            size_t len = 0;
            getline(&line, &len, stdin);
            output_str_contains(line, keyword, case_sensitive, out);
        }
    }
    else
    {
        while (argv[optind] != NULL)
        {
            FILE *pFile = fopen(argv[optind++], "r");
            if (pFile == NULL)
            {
                fprintf(stderr, "%s: No such file! Use:\n%s [-i] [-o outputfile] keyword [inputfile ...]\n", argv[0], argv[0]);
                exit(EXIT_FAILURE);
            }
            char *line;
            size_t len = 0;

            while (getline(&line, &len, pFile) != -1)
            {
                output_str_contains(line, keyword, case_sensitive, out);
            }
            write_to_out("\n", out);
            fclose(pFile);
        }
    }

    return 0;
}
