#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static char *progname;
static int ignoreWhiteSpaces = 0;
static int casesensitiveOff = 0;



static void check_palindrom(char *line, int index_left, int index_right);

static void is_palindrom(FILE *input_stream);

/** custom get line length which skippes new line character through counting */
static int get_line_length(char *line);

/* buffer[strcspn(buffer, "\n")] = 0; */
static void myfprint(char *line);

static void formatline(char* line, int length);

static char* to_lowercase(char* line);

static char* remove_whitespaces(char* line);


// SYNOPSIS    ispalindrom [-s] [-i] [-o outfile] [file...]
//  -s ... cause the program to ignore whitespaces
//  -i ... turn off case sensitive
//  -o ... write to outputfile insted stdout
//  file ... one or more input files
int main(int argc, char **argv) {
	
	FILE *input_stream = NULL;
    FILE *output_stream = NULL;


    char *outputfile;
	progname = argv[0];

	
    int opt = 0;
    while ((opt = getopt(argc, argv, "sio:k:")) != -1) {

        switch(opt) {
            case 's':
                ignoreWhiteSpaces = 1;
                //printf("[DEBUG] ignoring whitespacces\n");
                break;
            case 'i':
                casesensitiveOff = 1;
                //printf("[DEBUG] ignoring cassesensitivity\n");
                break;
            case 'o':
                //printf("filename: %s\n", optarg); 
                if(optarg != NULL) {
                    outputfile = optarg;
                    //printf("[DEBUG] outputfile name: %s\n", outputfile);
                } else {
                    fprintf(stderr,"[%s] Error: Please enter output file name\n", progname);
                    return EXIT_FAILURE;
                }
                break;
        }
    }

    for(; optind < argc; optind++){     
        printf("input file: %s\n", argv[optind]); 
        input_stream = fopen(argv[optind], "r");
        is_palindrom(input_stream);
    }
    printf("[FINISH] done palindrom check\n");


	
	//printf ("[%s] ERROR: please enter one or more input files", progname);
	return EXIT_SUCCESS;
}


static void is_palindrom(FILE *input_stream) {

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, input_stream)) != -1) {
        //printf("Retrieved line of length %zu:\n", read);
        //printf("%s", line);

        int index_right = 0;
        int index_left = 0;
        int length = get_line_length(line);
        //printf("line length is %d\n", length);
        //printf("line length half %d\n", length%2);
        if(length % 2 == 0) {
            index_left = length/2;
            index_right = length/2 + 1;
            check_palindrom(line, index_left, index_right);
        } else {
            index_left = length/2 -1;
            index_right = length/2+1;
            check_palindrom(line, index_left, index_right);
        }

        //check_palindrom();
    }

    fclose(input_stream);
    if (line) {
        free(line);
    }
    //printf("[DEBUG] ready is palindrom function\n");
}


static int get_line_length(char *line) {
    char c;
    int len = strlen(line);
	
	for(int i=0; i<len; i++) {
		c = line[i];

		if(c=='\n'){
			//printf("[DEBUG] current length: %d\n", i-1);
            return i-1;
		}
	}
    //printf("[DEBUG] after for: %d\n", len);
    return len;
}

static char* to_lowercase(char* line) {
    //printf("%s\n", line);

    size_t len = strlen(line);
    char *lower = calloc(len+1, sizeof(char));

    for (size_t i = 0; i < len; ++i) {
        lower[i] = tolower((unsigned char) line[i]);
    }
    //printf("[DEBUG] to_lowercase %s\n", lower);
    return lower;
}

static char* remove_whitespaces(char* line) {
    char *newstr = malloc(strlen(line)+1);
    char *np = newstr, *op = line;
    do {
        if (*op != ' ')
        *np++ = *op;
    } while (*op++);
    //printf("[DEBUG] remove_whitespaces %s\n", newstr);
    return newstr;
}


static void formatline(char* line, int length) {
    char dest[length];
    strncpy(dest, line, length);
    //printf("dest: %s\n", dest);
    printf("%s", dest);
}


static void check_palindrom(char *line, int index_left, int index_right) {
    char c;
    int len = get_line_length(line);
    //line[strcspn(line, "\n")] = 0;
    char *checkline = (char*)malloc(strlen(line)+1);
    strcpy(checkline, line);
    
    if (casesensitiveOff) {
        checkline = to_lowercase(checkline);
    }

    if(ignoreWhiteSpaces) {
        checkline = remove_whitespaces(checkline);
    }
	
	while(index_left >= 0) {
		if(index_right > len) {
            formatline(line, len);
            printf(" is not a palindorm\n");
            break;
        }

        if (checkline[index_left] == checkline[index_right]) {
            index_left--;
            index_right++;
        } else {
            formatline(line, len);
            printf(" is not a palindorm\n");
            return;
        }
	}
    formatline(line, len);
    printf(" is palindorm\n");
}


static void myfprint(char *line) {
    int len = strlen(line);
    char c;
	
	for(int i=0; i<len; i++) {
        c = line[i];
        //printf("first %c\n", c);
		if(c =='\n'){
			//printf("[DEBUG] current length: %d\n", i-1);
            break;
		}
        putchar(c);
        //printf("%c", line[i]);
	}   
}

