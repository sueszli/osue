#include "expand.h"

/**
 *  Since strsep is not C99, here is re-implementation
 *  Behaviour is as described in man (3) strsep.
 *
 */
static char *mystrsep(char **restrict stringp, const char *restrict delim);

int expand(FILE *input, FILE *output, int tabstop){
    assert(input != NULL);
    size_t n = 0;
    int linelength = 0;
    //line, segment and tail point to memory allocated at line
    char* line = NULL;
    char* segment = NULL;
    char** tail = NULL;
    char* outputBuffer = NULL;
    char* swap = NULL;
    int buflength = 0;

    char spaces[tabstop+1];
    for(int i=0; i<tabstop; i++)
        spaces[i] = ' ';
    spaces[tabstop] = '\0';

    //read a line
    while((linelength = getline(&line, &n, input))!=-1){

        //allocate plenty of memory
        if((swap = realloc(outputBuffer, (strchrocc(line, '\t')*(tabstop-1) + n)*sizeof(char))) == NULL){
            fprintf(stderr, "failed to allocate memory: %s", strerror(errno));
            goto fail;
        }
        outputBuffer = swap;
        outputBuffer[0] = '\0';

        tail = &line;
        segment = mystrsep(tail, "\t");
        strcat(outputBuffer, segment); //strcat does not protect against bof, but outputBuffer is large enough.
        while((segment=mystrsep(tail, "\t"))!=NULL){
            buflength = strlen(outputBuffer);
            strcat(outputBuffer, &spaces[buflength - (buflength/tabstop)*tabstop]);
            strcat(outputBuffer, segment); //here as well
        }

        //print
        if(fprintf(output, "%s", outputBuffer)<0){
            fprintf(stderr, "could not print to output");
            goto fail;
        }
    }
    //getline returne -1 because something went wrong
    if(!feof(input)){
        fprintf(stderr, "Error reading line from input; %s", strerror(errno));
        goto fail;
    }

    return 0;

    fail:
    free(line);
    free(outputBuffer);
    return 1;
}

static char *mystrsep(char **restrict stringp, const char *restrict delim){
    char *token = *stringp;
    if(*stringp==NULL){
        return NULL;
    }
    *stringp += strcspn(*stringp, delim);
    if(**stringp == '\0'){
        *stringp=NULL;
    }else{
        **stringp = '\0';
        (*stringp)++;
    }
    return token;
}
