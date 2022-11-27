/**
 * @file ispalindrom.c
 * @author Lucas Wolkersdorfer/11922587
 * @date 14.11.2021
 * @brief Assignement 1B ispalindrom solution
 * 
 * @details started like explained in print_usage;
 * prints if the entered word is a palindrom or not into either a dedicated output file or stdout.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <signal.h>
/**
 * @brief either 0 or 1, if 1 program terminates. "sig_atomic_t" because its called from the signal_handler
 * 
 */
static volatile sig_atomic_t quit = 0;


/**
 * @brief struct that holds the three possible flags -o, -i and -s.
 * 0 means they are turned off, 1 means they are turned on
 * also has out, specifies wheter to output to stdout(default) or a file(set with -o)
 * 
 */
struct Flags{
    FILE *out;
    unsigned int i;
    unsigned int s;
    unsigned int o;
};

/**
 * @brief struct that holds the flags entered by the user and where to output text to;
 * 
 */
static struct Flags settings;

static int palindrom(char *str, struct Flags settings);
static void toLowerCase(char *str);
static void removeSpace(char *str);
static void print_usage(char *str);
static int check_file(char *str);
static void handle_input(struct Flags settings, FILE *file);

/**
 * @brief sets the variable quit to 1
 * @details quit is a global variable
 * @param signal number of the signal, not used
 */
static void handle_signal(int signal) { quit = 1; }

/**
 * @brief main logic of the program.
 * @details starts with getopt to get all the options the user set and safes them in a struct.
 * then sets up the signal handler then calls handle_input with either stdin or with files that the user input.
 * 
 * @param argc arg counter
 * @param argv arg vector
 * @return returns EXIT_SUCCESS or EXIT_FAILURE 
 */
int main(int argc, char **argv)
{

    static int option;

    settings.out = stdout; //initializes flags struct
    settings.i = 0;
    settings.s = 0;
    settings.o = 0;

    while ((option = getopt(argc, argv, "o:si")) != -1)
    { // gets all flags and their arguments

        switch (option)
        { // if we call a flag twice we call print usage
        case 'o':
            if (settings.o == 1)
            {
                fclose(settings.out); // if we call o flag twice we need to close the file before we exit
                print_usage(argv[0]);
            }
            settings.out = fopen(optarg, "w");
            settings.o = 1;
            check_file(optarg);
            break;
        case 's':
            if ((settings.s) == 1)
            {
                print_usage(argv[0]);
            }
            else
            {
                settings.s = 1;
            }
            break;
        case 'i':
            if ((settings.i) == 1)
            {
                print_usage(argv[0]);
            }
            else
            {
                settings.i = 1;
            }
            break;
        default:
            print_usage(argv[0]);
        }
    }

    static struct sigaction sa; // signal handler so we can exit while loop with control c
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    if (argv[optind] == NULL)
    { // if this is null there are no files to read passed and we read from stdin
        handle_input(settings, stdin);
    }
    else
    { // else we iterate over all given files, check them and we read from those files not stdin
        int counter = optind;
        while (argv[counter] != NULL)
        {
            check_file(argv[counter]);
            FILE *input = fopen(argv[counter], "r");
            handle_input(settings, input);
            counter++;
        }
    }
    fclose(settings.out); // closes those files properly
    exit(EXIT_SUCCESS);
}
/**
 * @brief iterates over string and if letter is upper case replace it with same letter in lower case
 *  
 * @param str string whos character will all be put to lower case
 */
static void toLowerCase(char *str)
{

    int length = strlen(str);

    for (int i = 0; i < length; i++) 
    {
        str[i] = tolower(str[i]);
    }
}

/**
 * @brief iterates over string and removes all spaces
 * 
 * @param str the string from which we delete the spaces
 */
static void removeSpace(char *str)
{

    char *cpy = str;
    do // while the two char pointers are the same both are incremented
    {
        while (*cpy == ' ') //after every increment checks wheter cpy has a space
        {
            ++cpy;
        }
    } while ((*str++ = *cpy++)); // str is set to cpy after the current statement is processed
                                // which means if copy is skipping the space in the inner while
                                // str is only set to cpy when cpy doesnt have a space
                                // pretty neat
}

/**
 * @brief iterates over string and checks if its a palindrom; removes uppercase letters or spaces
 * if such is specified in the settings.
 * 
 * @param str string that is checked
 * @param settings flags so we know if we need to care about space/uppercase-
 * @return int 0 if string wasnt a palindrom, 1 if it was
 */
static int palindrom(char *str, struct Flags settings)
{

    if (settings.i)
    {
        toLowerCase(str);
    }

    if (settings.s)
    {
        removeSpace(str);
    }

    int count, length;
    count = 0;
    length = strlen(str) - 1;

    while (length > count) // we start with the beginning and the end of the word, look at both characters there
    {                       // and then increment the start and decrement the end point until we are done
        if (str[length] != str[count])
        {
            return 0;
        }
        length--;
        count++;
    }

    return 1;
}

/**
 * @brief prints how the program is correctly run
 * 
 * @param name program name
 */
static void print_usage(char *name)
{
    printf("Usage: %s [-s] [-i] [-o outfile] [file...]", name);
    exit(EXIT_FAILURE);
}

/**
 * @brief checks if a file that is opened exists
 * 
 * @param filename file that is opened to check if it exists
 * @return int returns 1 if file exists and exits with EXIT_FAILURE otherwise
 */
static int check_file(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp)
    {
        fclose(fp);
        return 1;
    }
    else
    {
        printf("unknown file: %s", filename);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief gets all lines that are input and calls palindrom to check if it is one.
 * prints result to either stdout or outfile, depending on o flag.
 * creates a copy and calls palindrom with copy, so we dont change the original string.
 * can also terminate through signal handler in while loop or EXIT_FAILURE in palindrom. 
 * 
 * @param settings struct with all flags and the output file
 * @param input stdin or a file; we read the lines from here
 */
static void handle_input(struct Flags settings, FILE *input)
{

    char *buffer = NULL; 
    size_t len = 0;

    while (getline(&buffer, &len, input) != -1 && !quit) // we can set len to 0 and buffer to NULL, getline calls realloc anyway
    {

        if (buffer[strlen(buffer) - 1] == '\n') // removes /n at the end of the string
        {
            buffer[strlen(buffer) - 1] = '\0';
        }
        char *cpy = strdup(buffer); // we make a copy(free it later), so we dont change the original string

        if (palindrom(cpy, settings))
        {
            fprintf(settings.out, "%s is a palindrom\n", buffer);
        }
        else
        {
            fprintf(settings.out, "%s is not a palindrom\n", buffer);
        }
        free(cpy);
    }
    free(buffer);
    fclose(input);
}
