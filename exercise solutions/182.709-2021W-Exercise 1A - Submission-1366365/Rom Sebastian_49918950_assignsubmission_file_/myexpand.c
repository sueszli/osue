/**
 * @file myexpand.c
 * @author Sebastian Rom 11809903
 * @date 12.11.2021
 * @brief This C Program imitates the linux expand command.
 *
 * @details myexpand takes text from either stdin or a/multiple files.
 * All Tabs will be replaced by a specific number of whitespace characters.
 * The number of whitespaces in a tab can be set by [-t Tabstop].
 * Also the output can be saved to a file using the [-o Filename] Parameter.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

int tabstop = 8;//default
char *output = NULL;
char *input = NULL;

/**
 * @brief This Method calculates where the next Character after a tab goes.
 * @param index The Index of the tab in a String.
 * @return      The Index where the next Character goes.
 */
static int calculatePosition(int index){
    return (tabstop * ((index / tabstop) + 1));
}

/**
 * @brief Goes through a String and counts its length. For each tab it adds the tabstop, thus calculating the max length.
 * @param string The String for which the new length should be calculated.
 * @return       The calculated length of the String with tabs replaced.
 */
static int calculateMaxLength(const char *string){
    int tabs = 0;
    for(int i=0;string[i]!=0;i++){
        if(string[i]=='\t'){
            tabs++;
        }
    }
    return (strlen(string) + tabs * tabstop);
}

/**
 * @brief Takes a String and replaces all tabs with a calculated number of whitespaces.
 * @details The length of the whitespaced String is calculated and a buffer with the calculated length is reserved.
 * Then the String gets copied into the buffer while replacing all tabs with a specific amount of whitespaces.
 * @param line  The String to be expanded.
 * @return      The expanded String.
 */
static char* expandLine(char *line){
    char *buffer = NULL;
    int len = calculateMaxLength(line);
    buffer = calloc(len, sizeof(char));
    int index = 0;
    for(int i=0;line[i]!=0;i++){
        if(line[i]=='\t'){
            buffer[index] = ' ';
            int p = calculatePosition(index);
            for(int u=(index+1);u<p;u++){
                index = u;
                buffer[index] = ' ';
            }
            index = p;
        }else{
            buffer[index++] = line[i];
        }
    }

    return buffer;
}

/**
 * @brief Sets the Input/Output options and calls expandLine on every Line of the Input, writing it to output.
 * @return 1 on success, -1 on input error, -2 on output error
 */
static int expand(void) {

    FILE *fpOut;
    FILE *fpIn;
    char *line = NULL;
    size_t len = 0;

    fpIn = (input==NULL)?stdin:fopen(input, "r");
    if(fpIn == NULL) {
        return -1;
    }

    fpOut = (output==NULL)?stdout:fopen(output, "a");
    if(fpOut == NULL){
        return -2;
    }

    char *expandedLine = NULL;

    while (getline(&line, &len, fpIn) != -1) {
        expandedLine = expandLine(line);
        fprintf(fpOut,"%s",expandedLine);
    }
    fclose(fpIn);
    fclose(fpOut);
    if (line) {
        free(line);
    }
    if(expandedLine){
        free(expandedLine);
    }

    return 1;

}

/**
 * @brief Checks the program parameter, sets all options and invokes expand.
 * @details Checks all input parameter and exits with an error message if necessary.
 * If multiple Files have been passed as arguments to expand this method calls expand for each of those files.
 * @param argc  argc
 * @param argv  Run Arguments
 * @return      0 on success, else 1;
 */
int main(int argc, char *argv[]) {

    int c;
    while ((c = getopt(argc, argv, "t:o:")) != -1) {
        switch (c) {
            case 't':
                if(optarg==NULL){
                    fprintf(stderr, "myexpand: tab size needs a numeric argument!\n");
                    return EXIT_FAILURE;
                }
                long ts = strtol(optarg, NULL, 10);

                if(ts > 0 && ts <= INT_MAX){
                    tabstop = ts;//Ensuring long can safely be casted to int
                }else{
                    fprintf(stderr, "myexpand: tab size contains invalid character(s): '%s'\n", optarg);
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                output = optarg;
                break;
            case '?':
                fprintf(stderr, "myexpand: Unknown option '%c'\n", c);
                return EXIT_FAILURE;
            default:
                assert(0);
        }
    }

    if(optind<argc){
        for(int i=optind;i<argc;i++){
            input = argv[i];
            int r = expand();
            if (r == -1) {
                fprintf(stderr, "myexpand: %s: No such file or directory\n", input);
            }else if(r == -2){
                fprintf(stderr, "myexpand: %s: Cannot write to file\n", output);
            }
        }
    }else{
        if (expand() != 1) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}