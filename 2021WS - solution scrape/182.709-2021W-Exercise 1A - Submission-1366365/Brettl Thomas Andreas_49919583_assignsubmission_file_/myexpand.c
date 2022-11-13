#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

void usageError() {
    fprintf(stderr,"Usage: myexpand [-t tabstop] [-o outfile] [file...]\n");
    exit(EXIT_FAILURE);
}

void allocError() {
    fprintf(stderr,"Memory allocation failed!\n");
    exit(EXIT_FAILURE);
}

void errorMsg(const char* msg) {
    fprintf(stderr,"%s\n",msg);
    exit(EXIT_FAILURE);
}

bool validInputFile(char* filename) {
    if(access(filename,F_OK) == 0)
        return true;
    return false;
}

int main(int argc, char** argv) {
    int opt_t = 0, opt_o = 0;
    int tabstop = 8;
    char* outfile;

    int c;
    while((c = getopt(argc,argv,"t:o:")) != -1) {
        switch(c) {
            case 't':
                opt_t++;
                char* succ;
                tabstop = strtol(optarg,&succ,10);
                break;
            case 'o':
                opt_o++;
                outfile = optarg;
                break;
            case '?':
                usageError();
                break;
            default:
                assert(0);
        }
    }

    if(opt_o > 1 || opt_t > 1)
        usageError();

    for(int i=optind; i<argc; i++) {
        if(!validInputFile(argv[i]))
            errorMsg("One or more files do not exist or cannot be opened!");
    }

    int paramCount = argc-optind;

    char** infiles;
    infiles = (char**) malloc(paramCount * sizeof(char*));
    if(infiles == NULL)
        allocError();

    for(int i=optind,a=0; i<argc; i++, a++) {
        infiles[a] = (char*) malloc(strlen(argv[i]) * sizeof(char));
        if(infiles[a] == NULL) {
            for(int b=(a-1);b>=0;b--)
                free(infiles[b]);
            free(infiles);
            allocError();
        }
        strcpy(infiles[a],argv[i]);
    }

    int outputLen = 1;
    char* output = NULL;

    bool readFromStdin = false;

    if(paramCount == 0) {
        paramCount = 1;
        free(infiles);
        readFromStdin = true;
    }
    for(int i=0;i<paramCount;i++) {
        FILE* f;
        if(readFromStdin)
            f = stdin;
        else
            f = fopen(infiles[i],"r");
        char ch;
        while((ch=fgetc(f)) != EOF) {
            if (ch == '\t') {
                char expand[tabstop + 1];
                memset(expand, ' ', (tabstop + 1) * sizeof(char));
                expand[tabstop] = '\0';

                outputLen += tabstop;
                if ((output = realloc(output, outputLen * sizeof(char))) == NULL)
                    exit(EXIT_FAILURE);
                for (int i = (outputLen - tabstop - 1), a = 0; i < (outputLen - 1); i++, a++)
                    output[i] = expand[a];

            } else {

                outputLen++;
                if ((output = realloc(output, outputLen * sizeof(char))) == NULL)
                    exit(EXIT_FAILURE);
                output[outputLen - 2] = ch;
                
            }

            output[outputLen - 1] = '\0';
        }
        fclose(f);
    }

    if(output != NULL) {
        if(opt_o) {
            FILE *f = fopen(outfile, "w");
            fputs(output, f);
            fclose(f);
        }
        else
            printf("%s",output);
    }

    if(!readFromStdin) {
        for(int i=0;i<paramCount;i++) {
            free(infiles[i]);
        }
        free(infiles);
    }

    return 0;
}
