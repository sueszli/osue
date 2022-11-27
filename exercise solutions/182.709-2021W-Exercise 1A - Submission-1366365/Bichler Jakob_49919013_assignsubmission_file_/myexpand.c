// @author Jakob Bichler (11806473)
// @date 2021-11-10
// @brief A program to replace tabs with a defined count of spaces

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "        ";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

char *repeat(const char *s, int x){
    if(s){
        int i, count = 0;

        while(s[count] != '\0'){
            ++count;
        }

        char *newArray = malloc(count * x + 1);
        if(newArray){
            char *na = newArray;
            for(i = 0; i < x; ++i) {
                const char *p=s;
                while(*p)
                    *na++ = *p++;
            }
            *na = '\0';
        }
        return newArray;
    } else {
        return NULL;
    }
}

char *dynamic_fgets(char **rtr, FILE *stream) {
    int bufsize = 1024; //Start at 1024 bytes
    char *buf = (char *) malloc(bufsize * sizeof(char));

    if (buf == NULL) {
        perror("Couldn't allocate memory for buf in dynamic_fgets\n");
    }

    do {
        fgets(buf, bufsize, stream);
        *rtr = realloc(*rtr, strlen(buf) + strlen(*rtr));
        if (*rtr == NULL) {
            perror("Couldn't allocate memory for *rtr in dynamic_fgets\n");
        }

        *rtr = strncat(*rtr, buf, bufsize);
        bufsize *= 2;
    } while (buf[strlen(buf) - 1] != '\n');

    return *rtr;
}

int main(int argc, char **argv) {
    int spaces = 8;
    int o = 0;
    char *filename;
    FILE *fp;
    FILE *fpr;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    char *oname = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, "t:o:")) != -1) //loop through passed options and params
        switch (c) {
            case 't':
                spaces = atoi(optarg); //set number of spaces
                if (spaces < 0){
                    fprintf(stderr, "Argument must be over 0.\n");
                    exit(EXIT_FAILURE);
                }

                break;

            case 'o':
                o = 1; //set output to file
                oname = optarg;
                break;
            case '?':
                if (optopt == 'o' )
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt); //error if no argument for -o flag
                else if (optopt == 't')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt); //error if no argument for -t flag
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);  //error if option not known
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);

                exit(EXIT_FAILURE);
            default:
                abort();
        }

    int nonOptionIndex = 0;

    for (index = optind; index < argc; index++) {
        if (nonOptionIndex == 0) {
            filename = argv[index];  //set filename from commandline arguments
        }
        if (nonOptionIndex > 0) {
            fprintf(stderr, "Too many arguments\n"); //Error if too many arguments are passed
            exit(EXIT_FAILURE);
        }
        nonOptionIndex += 1;
    }
    if (nonOptionIndex == 0){
    //    fprintf(stderr, "Not enough arguments\n"); //Error if no arguments are passed
    //    exit(EXIT_FAILURE);
    }
    fp = fopen(filename, "r");
    if (o == 1) {
        fpr = fopen(oname, "wb");
    }

    char *space_string = repeat(" ", spaces);

    if (fp == NULL) { //if no input file is passed or found
        while (1) {
            char *buf = (char *) malloc(sizeof(1)); //allocate empty char pointer
            strncpy(buf, "\0", 1);

            dynamic_fgets(&buf, stdin); //get console input with dynamic memory allocation
            
            char *ptr = str_replace(buf,"\t", space_string);
            if (ptr != NULL) {
                if (o == 1) {
                    fprintf(fpr, "%s", ptr);
                } else {
                    printf("%s", ptr);
                }
            }
            free(buf);
        }
    } else {
        while ((read = getline(&line, &len, fp)) != -1) {  //read lines from file until its empty
            char *ptr;
            ptr = str_replace(line,"\t", space_string);
            if (ptr != NULL) {
                if (o == 1) {
                    fprintf(fpr, "%s", ptr);
                } else {
                    printf("%s", ptr);
                }
            }
        }
    }
    //close files and free allocated memory
    if (fpr)
        fclose(fpr);
    if (fp)
        fclose(fp);
    if (line)
        free(line);
    exit(EXIT_SUCCESS); //exit with success
}
