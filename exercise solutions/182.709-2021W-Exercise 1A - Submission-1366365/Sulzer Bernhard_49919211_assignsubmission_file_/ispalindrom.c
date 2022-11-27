/**
 * @file ispalindrome.c
 * @author Bernhard Sulzer <bernhard.sulzer@student.tuwien.ac.at>
 * @date 2021-11-14
 * 
 * @brief Check if a string is a palindrom (sic)
 * @details
 * 
 * 
 **/

#include <unistd.h> // getopt
#include <stdio.h>
#include <errno.h>
#include <string.h> // strerror
#include <stdlib.h> // exit()
#include <ctype.h> // tolower


void process_file(FILE *inputfile, FILE *out, int ignorespace, int casesensitive);
int check_palindrome(char *string, int length, int ignorespace, int casesensitive);


/**
 * Program entry point.
 * @brief Process all arguments given.
 * @details Read possible palindromes from input files or stdin, write results to output file or stdout.
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char **argv) {
    int ignorespace = 0;
    int casesensitive = 1;
    char *outFileName = NULL;
    
    char c;
    while((c = getopt(argc, argv, "sio:")) != -1) {
        switch(c) {
        case 's':
            ignorespace = 1;
            break;
        case 'i':
            casesensitive = 0;
            break;
        case 'o':
            outFileName = optarg;
            break;
        default:
            fprintf(stderr, "Usage: ispalindrom [-s] [-i] [-o outfile] [file...]\n");
            exit(EXIT_FAILURE);
        }
    }
    
    
    FILE *out;
    // Use output file if option was defined else stdout
    if(outFileName != NULL) {
        if ( (out = fopen(outFileName, "w+")) == NULL ) {
            fprintf(stderr, "[%s] ERROR: fopen failed for file %s in line %i: %s\n",
            argv[0], outFileName, __LINE__, (char *)strerror(errno));
            exit(EXIT_FAILURE);
        }
    } else {
        out = stdout;
    }
    
    // Use input files if defined (positional arguments) else stdin
    if (optind < argc) {
        FILE *inputfile;
        for(int i=optind; i<argc; i++) {
            if ( (inputfile = fopen(argv[i], "r")) == NULL ) {
                fprintf(stderr, "[%s] ERROR: fopen failed for file %s in line %i: %s\n",
                argv[0], argv[i], __LINE__, (char *)strerror(errno));
                exit(EXIT_FAILURE);
            }
            process_file(inputfile, out, ignorespace, casesensitive);
            fclose(inputfile);
        }
    } else {
        process_file(stdin, out, ignorespace, casesensitive);
    }
    
    
    if(outFileName != NULL)
        fclose(out);
    
    return EXIT_SUCCESS;
}

/**
 * Program entry point.
 * @brief Check a file line-by-line for palindromes.
 * @details Read each line until the linebreak, and check (without trailing newline) if the string is a palindrome
 * @param inputfile Read palindrome lines.
 * @param out Write results.
 * @param ignorespace Do not consider spaces w
 * @param casesensitive
 * @return Returns EXIT_SUCCESS.
 */
void process_file(FILE *inputfile, FILE *out, int ignorespace, int casesensitive) {
    char *line = NULL;
    size_t len = 0;
    int read;
    while ((read = getline(&line, &len, inputfile)) != -1) {
        // Remove trailing newline
        if(line[read-1] == '\n')
            read--;
        
        // Skip empty lines
        if(read == 0)
            continue;
        
        char *result = check_palindrome(line, read, ignorespace, casesensitive) ? "is":"is not";
        fprintf(out, "%.*s %s a palindrom\n", read, line, result);
    }
}

/**
 * Program entry point.
 * @brief Check a string for palindromes.
 * @details Also considers ignorespace, casesensitive when checking for palindromes
 * @param string Non null-terminated string.
 * @param length Length of the string.
 * @return Returns EXIT_SUCCESS.
 */
int check_palindrome(char *string, int length, int ignorespace, int casesensitive) {
    int a=0, b=length-1;
    
    while(a<b) {
        if(ignorespace) {
            // Skip past all whitespace characters
            while(string[a] == ' ' && a<length)
                a++;
            while(string[b] == ' ' && b>=0)
                b--;
            
            if(a>=b)
                return 1;
        }
        
        if(casesensitive) {
            if(string[a] != string[b])
                return 0;
        } else {
            if(tolower(string[a]) != tolower(string[b]))
                return 0;
        }
        a++;
        b--;
    }
    
    return 1;
}
