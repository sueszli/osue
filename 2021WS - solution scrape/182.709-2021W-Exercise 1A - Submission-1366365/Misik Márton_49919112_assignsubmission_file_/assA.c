#include "assA.h"

int main(int argc, char *argv[]){
    
    //Argument handling
    bool sflag = false;
    bool iflag = false;
    bool oflag = false;
    char* outfile;
    int c = 0;
    int flagCounter = 1;
    while((c = getopt(argc, argv, "sio:")) != -1){
        switch (c)
        {
        case 's':
            sflag = true;
            flagCounter++;
            break;
        case 'i':
            iflag = true;
            flagCounter++;
            break;
        case 'o':
            oflag = true;
            outfile = optarg;
            flagCounter+= 2;
            break;
        default:
            fprintf(stdout, "Please provide an input through the terminal.");
            break;
        }
    }
    
    //Preallocate memory space
    char* finalOutput = malloc(sizeof(char));
    if (finalOutput == NULL){
        fprintf(stderr, "Malloc failed");
        exit(EXIT_FAILURE);
    }

    //Reading from console
    if ((argc - flagCounter) == 0){
        char buffer[BUFF_SIZE];
        while((fgets(buffer, sizeof(buffer), stdin)) != NULL){
            char *returned = evaluate(buffer, sflag, iflag);
            appendToFullResult(&finalOutput, returned);
        }
    }
    //Reading from file/s
    else {
        fileManager(&finalOutput, argv, flagCounter, argc - flagCounter, sflag, iflag);
    }
    
    //Output result
    if(oflag){
        fprintf(stderr, outfile);
        fprintf(stderr, "\n");
        FILE *output = fopen(outfile, "w");
        if(output == NULL){
            fprintf(stderr, "fopen failed");
            exit(EXIT_FAILURE);
        }
        if ((fputs(finalOutput, output)) < 0){
            fprintf(stderr, "fputs failed");
            exit(EXIT_FAILURE);
        }
        fflush(output);
        fclose(output);
    }
    else {
        if ((fputs(finalOutput, stdout)) < 0){
            fprintf(stderr, "fputs failed");
            exit(EXIT_FAILURE);
        }
        fflush(stdout);
    }
    free(finalOutput);
    return 0;
}


