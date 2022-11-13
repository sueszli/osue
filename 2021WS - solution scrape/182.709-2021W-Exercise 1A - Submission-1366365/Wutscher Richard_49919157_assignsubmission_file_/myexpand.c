/*! @brief Expands a given texts tabs into spaces
 *  
 *  @details Replaces every occurance of the \t
 *  symbol in a given text with an adequate 
 *  number of spaces according to the configured
 *  tabstop
 * 
 *  @author Richard Wutscher (12022492)
 *  @date 14.11.2021
 */
#include "myexpand.h"

/*! \brief Calculates spaces to the next tabstop
 *
 *	Calculates the number of spaces from the current position to
 *  the next tabstop
 *
 * 	@param position The current position in the line
 * 	@param tabstop The tabstop intervall
 *  @return The number of spaces required to insert, to reach the
 *  next tabstop
 */
int nextTabStop(int position, int tabstop){
    return (tabstop * ((position / tabstop) + 1));
}

/*! \brief Expands a given string
 *
 *	Replaces every \t character in a given string
 *  with the adequate amount of spaces according
 *  to the tabstop
 *
 * 	@param input The string that must be myexpanded
 * 	@param tabstop The tabstop intervall
 *  @return A pointer to the myexpanded string
 */
char *formatFile(char *input, int tabstop){
    int linestart = 0;


    int positionInLine = 0;
    int newLength = 0;

    for (int i = 0; i < strlen(input); i++) {
        char character = *(input + i);
        if (character == '\n') {
            positionInLine = linestart;
            newLength++;
        }else if (character == '\t') {
            int nextStop = nextTabStop(positionInLine, tabstop);
            int numSpaces = nextStop-positionInLine;
            newLength += numSpaces;
            positionInLine += numSpaces;
        }else{
            newLength++;
            positionInLine ++;
        }
        
    }

	char *output = malloc(newLength+1);
    positionInLine = linestart;

    int o = 0;
	for (int i = 0; i < strlen(input); i++) {
        char character = *(input + i);
        if (character == '\n') {
			output[o] = '\n';
            o++;
            positionInLine = linestart;
        }else if (character == '\t') {
            int nextStop = nextTabStop(positionInLine, tabstop);
            int numSpaces = nextStop-positionInLine;
            for (int j = 0; j < numSpaces; j++) {
                output[o] = ' ';
                o++;
            }
            positionInLine += numSpaces;
        } else {
			output[o] = character;
            o++;
            positionInLine++;
        }
    }
    output[o] = '\0';

    return output;
}

/*!	\brief Main function
 *
 *	This program is to be used as followed:
 *  
 *  myexpand [-t tabstops] [-o outfile] [file...]
 *	
 *	@param argc Number of arguments passed
 *	@param argv Array of arguments
 */
int main(int argc, char **argv) {

    char *outFile;
    int tabstop = 8;
    char *input;
    int opt;

    while ((opt = getopt(argc, argv, "t:o:")) != -1) {
        switch (opt) {
        case 't':
            tabstop = strtol(optarg, NULL,10);
            break;
        case 'o':
            outFile = optarg;
            break;
        default: //'?'
            fprintf(stderr, "Usage: %s [-t tabstops] [-o outfile] [file...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    FILE* write_file;

    if(outFile){
        write_file = fopen (outFile, "w+");
    }


    if(argc - optind == 0){
        char buffer[1024];
        size_t contentSize = 1;
        char *input = malloc(sizeof(char) * 1024);
        input[0] = '\0';
        while(fgets(buffer, 1024, stdin)) {
            char *old = input;
            contentSize += strlen(buffer);
            input = realloc(input, contentSize);
            if(input == NULL) {
                fprintf(stderr, "[%s] Error reallocating memory\n", argv[0]);
                free(old);
                exit(EXIT_FAILURE);
            }
            strcat(input, buffer);
        }

        char *output = formatFile(input, tabstop);
        if(outFile){ // warum nicht if(write_file){}
            fprintf (write_file, "%s", output);
        }else{
            printf("%s\n", output);
        }
        free(input);

    }else{
        for (int i = optind; i < argc; i++){
            int file = open(argv[i], O_RDONLY);

            if(file){
                struct stat sb;
                if (fstat(file, &sb) == -1){
                    fprintf(stderr, "[%s] Unable to read file (fstat)\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                input = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, file, 0);
                char *output = formatFile(input, tabstop);
                munmap(output, sb.st_size);
                close(file);

                if(outFile){ // warum nicht if(write_file){}
                    fprintf (write_file, "%s", output);
                    if(i != argc-1){
                        fprintf (write_file, "\n");
                    }
                }else{
                    printf("%s\n", output);
                }
                free(output);
                
            }else{
                close(file);
                fprintf(stderr, "[%s] Unable to read file (open)\n", argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }


    if(outFile){    // warum nicht if(write_file){}
        fclose (write_file);
    }

    // if(useOutFile)
    // printf("%s\n", outFile);
    exit(EXIT_SUCCESS);
}
