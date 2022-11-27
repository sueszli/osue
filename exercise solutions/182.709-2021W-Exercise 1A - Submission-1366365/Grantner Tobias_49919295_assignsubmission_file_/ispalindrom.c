/**
 * @file ispalindrom.c
 * @author Tobias Grantner (12024016)
 * @brief This program is used to read lines from a file or stdin, check if these lines are palindroms and write the result to an output file or stdout.
 * @date 2021-11-11
 */

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/**
 * @brief Flag defining that spaces should not be considered in input strings.
 */
#define FLAG_S 1

/**
 * @brief Flag defining that the program should not differenciate between lower and upper case letters in input strings.
 */
#define FLAG_I 2

/**
 * @brief Flag defining that output should be written to a file instead of stdout and which file it should be written to.
 */
#define FLAG_O 4

/**
 * @brief Prints error message and exits the program with code EXIT_FAILURE.
 */
#define ERROR(message)                                         \
    {                                                          \
        fprintf(stderr, "[%s] ERROR: %s\n", argv[0], message); \
        exit(EXIT_FAILURE);                                    \
    }

/**
 * @brief Prints strerror error message and exits the program with code EXIT_FAILURE.
 */
#define ERR() ERROR(strerror(errno))

/**
 * @brief Closes the input and the output stream.
 */
#define CLOSE_ALL()     \
    {                   \
        fclose(input);  \
        fclose(output); \
    }

/**
 * @brief Checks whether the given string is a palindrom or not.
 * 
 * @param palindrom String to be checked
 * @param length Length of the given string
 * @return int 0 if not a palindrom, 1 if palindrom
 */
static int check_palindrom(const char *palindrom, ssize_t length)
{
    ssize_t start = 0;
    ssize_t end = length - 1;

    while (start < end)
    {
        if (palindrom[start] != palindrom[end])
        {
            return 0;
        }
        start++;
        end--;
    }

    return 1;
}

/**
 * @brief Removes the spaces from the given source and writes the result in the destination.
 * 
 * @param destination Where the result should be written to
 * @param source The string source
 * @param slength Length of the source string
 * @return ssize_t Length of the result
 */
static ssize_t remove_spaces(char *destination, const char *source)
{
    ssize_t length = 0;
    while (*source != '\0')
    {
        if (*source != ' ')
        {
            *destination = *source;
            length++;
            destination++;
        }
        source++;
    }
    *destination = '\0';
    return length;
}

/**
 * @brief Converts all upper case characters in a string to lower case characters
 * 
 * @param destination Where the result should be written to
 * @param source The string source
 */
static void to_lower_string(char *destination, const char *source)
{
    while (*source != '\0')
    {
        *destination = tolower(*source);
        source++;
        destination++;
    }
    *destination = '\0';
}

/**
 * @brief Applies the given flags to the source string and writes the result to the destination string.
 *        Flags used here are FLAG_S and FLAG_I.
 * 
 * @param destination Where the result should be written to
 * @param source The string source
 * @param slength Length of the source string
 * @param flags Flags that should be applied
 * @return ssize_t Length of the result
 */
static ssize_t apply_flags(char *destination, const char *source, ssize_t slength, int flags)
{
    strcpy(destination, source);

    if (flags & FLAG_S)
    {
        slength = remove_spaces(destination, destination);
    }
    if (flags & FLAG_I)
    {
        to_lower_string(destination, destination);
    }

    return slength;
}

/**
 * @brief This is the main-function of the program and contains argument parsing, file reading and writing and all the main logic of the program.
 * 
 * @param argc Number of passed arguments
 * @param argv Passed arguments
 * @return int Exit code
 */
int main(int argc, char **argv)
{
    char c;
    char *out_file;
    int flags = 0;

    while ((c = getopt(argc, argv, "sio:")) != -1)
    {
        switch (c)
        {
        case 's':
            if (flags & FLAG_S)
                ERROR("Option 's' is specified more than once.");
            flags |= FLAG_S;
            break;
        case 'i':
            if (flags & FLAG_I)
                ERROR("Option 'i' is specified more than once.");
            flags |= FLAG_I;
            break;
        case 'o':
            if (flags & FLAG_O)
                ERROR("Option 'o' is specified more than once.");
            flags |= FLAG_O;
            out_file = optarg;
            break;
        default:
            ERROR("Unknown option.")
        }
    }

    FILE *input = stdin;
    FILE *output = stdout;

    if (flags & FLAG_O)
    {
        output = fopen(out_file, "w");
        if (output == NULL)
            ERR();
    }

    int in_files = argc - optind; // args without letter begin after last optind

    do
    {
        if (in_files > 0)
        {
            int close_result = fclose(input);
            if (close_result < 0)
                ERR();

            input = fopen(argv[optind], "r");
            if (input == NULL)
                ERR();
        }

        char *line = NULL;
        size_t length = 0;
        ssize_t nread;
        char *applied;

        while ((nread = getline(&line, &length, input)) != -1)
        {
            if (!feof(input))
            {
                nread--; // minus one because '\n' is read
            }

            line[nread] = '\0'; // replace '\n' with end of string

            applied = malloc(nread * sizeof(*applied));

            if (applied == NULL)
                ERROR("malloc failed.");

            nread = apply_flags(applied, line, nread, flags);

            if (check_palindrom(applied, nread))
            {
                if(fprintf(output, "%s is a palindrom\n", line) < 0) ERR();
            }
            else
            {
                if(fprintf(output, "%s is not a palindrom\n", line) < 0) ERR();
            }

            free(applied);

            if (ferror(output))
            {
                CLOSE_ALL();
                ERR();
            }
        }

        if (ferror(input))
        {
            CLOSE_ALL();
            ERR();
        }

        optind++;
    } while (optind < argc);

    return EXIT_SUCCESS;
}
