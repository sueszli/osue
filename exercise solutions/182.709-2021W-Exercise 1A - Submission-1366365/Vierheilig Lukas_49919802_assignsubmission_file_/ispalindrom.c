/**
* @author: Lukas Vierheilig 
* @file: ispalindrom.c 
* @brief: Assignment_1A 
* @details: Small programm to test if string is palindrom 
* @date: 01/11/2021 
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

/**
 * @fn:void usage(void)
 * @brief: prints synopsis to stdout
*/
void usage(void);

/**
 * @fn: int checkPalindrom(char *str, int len)
 * @brief: checks if given string is a palindrom
 * @return: returns a int which indecate how many errors are in the string, if read as palindrom
 * @param str: char ptr to string which should be analyzed
 * @param len: length of given string
*/
int checkPalindrom(char *str, int len);

/**
 * @fn: char *removeWhiteSpace(char *str, long int *len)
 * @brief: removes all whitespace from given str
 * @return: ptr to clensed str needs to be freed
 * @param str: ptr to string which should be clensed
 * @param len: length of given str
*/
char *removeWhiteSpace(char *str, long int *len);

/**
 * @fn: char *convertToUpper(char *str, int len)
 * @brief: converts all chars in c string to upper case
 * @return: ptr to str with all chars in upper case. needs to be freed
 * @param str: ptr to c string which should be converted to upper case
 * @param len: len of given str
*/
char *convertToUpper(char *str, int len);

int main(int argc, char **argv)
{
    char *arg_o = NULL;
    int opt_s = 0, opt_i = 0, c;
    FILE *out = NULL;

    while ((c = getopt(argc, argv, "o:si")) != -1)
    {
        switch (c)
        {
        case 'o':
            arg_o = optarg;
            break;
        case 's':
            opt_s++;
            break;
        case 'i':
            opt_i++;
            break;
        case '?':
            usage();
            break;
        default:
            usage();
            break;
        }
    }

    if (arg_o == NULL)
    {
        if ((out = fdopen(STDOUT_FILENO, "w")) == NULL)
        {
            fprintf(stderr, "fdopen failed on STDOUT: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        if ((out = fopen(arg_o, "w")) == NULL)
        {
            fprintf(stderr, "fopen failed on arg_o: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if (opt_s > 1)
    {
        fprintf(stderr, "option s called multiple times\n");
        usage();
    }
    if (opt_i > 1)
    {
        fprintf(stderr, "option i called multiple times\n");
        usage();
    }

    if (argv[optind] == NULL)
    {
        char *line = NULL;
        size_t len = 0;
        ssize_t nread = 0;
        ssize_t origread = 0;
        char *buf;

        while ((nread = getline(&line, &len, stdin)) != -1)
        {
            buf = line;
            origread = nread;

            if (opt_i)
            {
                buf = convertToUpper(line, nread);
            }

            if (opt_s)
            {

                buf = removeWhiteSpace(buf, &nread);
            }

            if (checkPalindrom(buf, nread))
            {
                char buf[origread];
                memset(buf, '\0', origread);
                strncpy(buf, line, origread);

                fprintf(out, "%s is not a palindrom\n", buf);
            }
            else
            {
                char buf[origread];
                memset(buf, '\0', origread);
                strncpy(buf, line, origread - 1);

                fprintf(out, "%s is a palindrom\n", buf);
            }
        }
        free(buf);
    }
    else
    {

        char *line = NULL;
        size_t len = 0;
        ssize_t nread = 0;
        ssize_t origread = 0;
        FILE *in = NULL;
        char *buf;
        while (argv[optind] != NULL)
        {
            if ((in = fopen(argv[optind], "r")) == NULL)
            {
                fprintf(stderr, "fopen failed on in: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            while ((nread = getline(&line, &len, in)) != -1)
            {
                buf = line;
                origread = nread;

                if (opt_i)
                {
                    buf = convertToUpper(line, nread);
                }
                if (opt_s)
                {
                    buf = removeWhiteSpace(buf, &nread);
                }
                char res[origread];
                memset(res, '\0', origread);
                strncpy(res, line, origread - 1);

                if (checkPalindrom(buf, nread))
                {
                    fprintf(out, "%s is not a palindrom\n", res);
                }
                else
                {
                    fprintf(out, "%s is a palindrom\n", res);
                }
            }
            optind++;
        }
        fclose(in);
        free(buf);
    }
    fclose(out);
    exit(EXIT_SUCCESS);
}

void usage(void)
{
    printf("Usage: ispalindrom [-s] [-i] [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}

int checkPalindrom(char *str, int len)
{
    char buf[len];
    memset(buf, '\0', len);
    strncpy(buf, str, len);
    int res = 0;

    for (size_t i = 0; i < len / 2; i++)
    {
        if (buf[i] != buf[len - 2 - i])
        {
            res++;
        }
    }
    return res;
}

char *removeWhiteSpace(char *str, long int *len)
{
    char *clensed = malloc((*len + 1) * sizeof(char));
    if (clensed == NULL)
    {
        fprintf(stderr, "malloc failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    char buf[*len + 1];
    memset(buf, '\0', *len + 1);
    memset(clensed, '\0', *len + 1);
    strncpy(buf, str, *len);
    int index = 0;

    for (size_t i = 0; i < *len; i++)
    {
        if (isblank((unsigned char)buf[i]))
        {
            continue;
        }
        else
        {
            clensed[index] = buf[i];
            index++;
        }
    }
    *len = index;
    return clensed;
}

char *convertToUpper(char *str, int len)
{
    char *buf = malloc((len + 1) * sizeof(char));
    if (buf == NULL)
    {
        fprintf(stderr, "malloc failed: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    memset(buf, '\0', len + 1);
    strncpy(buf, str, len);

    for (size_t i = 0; i < len; i++)
    {
        if (isblank((unsigned char)buf[i]) == 0)
        {
            buf[i] = toupper((unsigned char)buf[i]);
        }
    }
    return buf;
}
