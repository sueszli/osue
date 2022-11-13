
/*! @brief Replaces Tabs with spaces
 *  @file myexpand.c
 *  @author Nemanja Tesanovic 12026642
 *  @date 14.11.2021
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define handle_error(name, msg) \
           do { fprintf(stderr, "%s  %s", name, msg); exit(EXIT_FAILURE); } while (0)

/*! @brief Replaces all tabs of the specified file or text with spaces 
 *  
 *  Replaces all tabs of the specified file or text with spaces while also maintaining the same distances between characters by calculating the required tabs.
 *  For those calculations the tabstop parameter is required.
 *  
 *  @param textOrName Either the name of the program which is used for error messages or a whole text from which the tabs shall be removed
 *  @param tabstop The tabstop is used to calculate the numbers of spaces which have to be added to get all the chars to the same position
 *  @param inFile The file from which the contents are to be read and the tabs replaced. If this parameter is NULL the String should be in the textOrName parameter
 *  @return The String with all tabs replaced by spaces
*/
char *replaceTabs(char *textOrName, int tabstop, char *inFile) {

    char *text = NULL;
    if (inFile != NULL) {
        int ifd;
        struct stat isb;

        ifd = open(inFile, O_RDONLY);
        if (ifd == -1) handle_error(textOrName, "An error occured while opening the file.");

        if (fstat(ifd, &isb) == -1)           /* To obtain file size */
            handle_error(textOrName, "fstat");

        text = mmap(NULL, isb.st_size, PROT_READ, MAP_PRIVATE, ifd, 0);
    } else {
        text = textOrName;
    }
    

    
    int tabsCount = 0;
    for(int i = 0; i < strlen(text); i++) {
        if (text[i] == '\t') tabsCount++;
    }


    int x = 0;
    int length = 0;
    int spacesForTabs[tabsCount];
    int currentTab = 0;
    for(int i = 0; i < strlen(text); i++) {
        char c = text[i];
        switch (c) {
            case '\t': ;
                int p = tabstop * ((x / tabstop) + 1);
                length += p-x;
                spacesForTabs[currentTab] = p-x;
                currentTab++;
                x = p;
                break;
            case '\n': 
                x = 0;
                length++;
                break;
            default: 
                length++; 
                x++;
        }
        
    }
    
    char *erg = malloc(sizeof(char) * (length+1));
    int p = 0;
    currentTab = 0;
    for (int i = 0; i < strlen(text); i++) {
        char c = text[i];
        switch (c) {
            case '\t': 
                for (int j = 0; j < spacesForTabs[currentTab]; j++) {
                    erg[p] = ' ';
                    p++;
                }
                currentTab++;
                break;
            default: 
                erg[p] = text[i];
                p++;
        }
    }
    erg[p] = '\0';
    
    if (inFile != NULL) munmap(text, strlen(text));

    return erg;
}

/*! @brief Takes texts, replaces all tabs with spaces and saves or prints the new text
 *  
 *  Reads a string either from files specified as positional arguments or from STDIN and replaces all their tabs with spaces.
 *  The resulting strings are either saved in a output file if specified with -o or printed to STDOUT.
 *  The tabstop used for calculating the amount of spaces needed to replace any given tab can be as an argument with the -t flag (default 8).
 *  
 *  @param argc The number of arguments
 *  @param nodes The array containing all arguments
 *  @return default return value
*/
int main(int argc, char *argv[]) {
    char *myprog = argv[0];
    char *t_arg = NULL;
    char *o_arg = NULL;
    int c;
    int tabstop = 8;

    while ( (c = getopt(argc, argv, "t:o:")) != -1 ){
        switch ( c ) {
            case 't': t_arg = optarg;
                break;
            case 'o': o_arg = optarg;
                break;
            default: /* invalid option */
                break;
        }
    }
    char *delimiter = NULL;
    if (t_arg != NULL) {
        int tabs = strtol(t_arg, &delimiter, 10);
        if (t_arg != delimiter) tabstop = tabs; 
    }

    if (argc - optind == 0) {
        char buffer[1024];
        size_t contentSize = 1; 
        
        char *content = malloc(sizeof(char) * 1024);
        if(content == NULL) handle_error(myprog, "Failed to allocate content");

        content[0] = '\0'; // make null-terminated
        while(fgets(buffer, 1024, stdin))
        {
            char *old = content;
            contentSize += strlen(buffer);
            content = realloc(content, contentSize);
            if(content == NULL)
            {
                free(old);
                handle_error(myprog, "Failed to reallocate content");
            }
            strcat(content, buffer);
        }

        if(ferror(stdin))
        {
            free(content);
            handle_error(myprog, "Error reading from stdin.");
        }

        char *erg = replaceTabs(content, tabstop, NULL);
        if (o_arg != NULL) {
            FILE *fp = fopen(o_arg, "a");
            fprintf(fp, "%s", erg);
            fclose(fp);
        } else {
            printf("%s\n", erg);
        }
        free(erg);
        free(content);

    } else {
        for(int i = optind; i < argc; i++) {
            char *erg = replaceTabs(myprog, tabstop, argv[i]);
            if (o_arg != NULL) {
                FILE *fp = fopen(o_arg, "a");
                fprintf(fp, "%s", erg);
                if (i != argc-1) fprintf(fp,"\n");
                fclose(fp);
            } else {
                printf("%s\n", erg);
            }
            
            free(erg);
        }
    }
    

    return 0;
}