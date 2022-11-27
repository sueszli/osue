#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include "ispalindrom.h"

/**
 * @author Kidery Dominik, 41781147
 * @date 11/10/2021
 * @brief Checks if an input is a palindrom or not
 * @details The algorithm goes through the inputted word or list with a fairly simple principle, we declare 3 variables which are the key parts of this,
 *          the (f)ront, (b)ack and (c)ounter. Going from front to back, we increase the counter everytime we have similar elements at their places,
 *          until reaching the halfway point. From there, we check if the c and f variable match, if so, we have a palindrom. Otherwise we've got mismatches and no palindrom at our hand.
 *          The program has been divided into multiple help functions, each with their own use according to the extra parameters we can add into the input.
 *          -s removes whitespaces by shifting our array when we hit one, -i dismisses case sensitivity and -o writes our results into a provided output file.
 *          listTraversal simple takes over the part of going through a list, this avoids code duplication since the program always runs in a do-while loop.
 *          We get the additional parameters from our switch-cases in the main function, which set our parameters to their according values given the inputs.
 *          The checking part happens in the checkPalin function, which goes, as described earlier, through the input.
 * 
 * @param input Our character array, a String
 * @param outputFile The file, given in the parameters, we write our results into
 * @param f The front variable
 * @param b The back variable
 * @param c The counter variable
 * @param i The parameter for case-sensitivity
 * @param s The parameter for whitespaces
 * @param o The parameter for writing our results into an output file or console
 * @param pal The parameter for deciding if we've got a palindrom or not
 * @param copied The copied input array, for giving a proper output (original input)
 */

void input(char*);
void copied(char*);

static void listTraversal(char *input, FILE *outputFile, int f, int b, int c, int i, int s, int o){
    char copied[b];
    strcpy(copied, input);

    if(s == 1){ //Call help method remove_whitespace, removing all space inbetween the words thus creating a new input, setting back variable according to new length
        remove_whitespaces(input);
        b = strlen(input);
    }

    int pal = checkPalin(input, f, b, c, i);

    if(o == 1){
        writeToFile(copied, outputFile, pal);
    } else {
        writeToConsole(copied, pal);
    }
}

static void remove_whitespaces(char *input){ //method to remove the whitespaces in given string when called with -s
    char* cleaned = input;

    do{ //do-while loop for removing the whitespaces, goes through it 
        while(*cleaned == ' '){
            ++cleaned;
        }
    } while((*input++ = *cleaned++));
}

static int checkPalin(char *input, int f, int b, int c, int i){
    int pal = 0;

    if(i == 1){
        for(f=0;f < b/2;f++){ //Loop for going through word letter by letter, if matches = increase counter by 1
            char front = input[f];
            char back = input[b-f-1];
            if(tolower(front) == tolower(back)){
                c++;
            }
        }
        if(f == c){
            pal = 1;
        }
    }
    else{
        for(f=0;f < b/2;f++){ //Loop for going through word letter by letter, if matches = increase counter by 1
            if(input[f] == input[b-f-1]){
                c++;
            }
        }
        if(f == c){
            pal = 1;
        }
    }
    return pal;
}

static void writeToConsole(char *copied, int pal){

        if(pal == 1){ //if our counter equals to the front variable (which stops at half the word) = palindrom, i.e. anna: c=2, f increases to 2, stops going through loop == true
            printf("%s is a palindrom\n",copied);
        } else{
            printf("%s is not a palindrom\n",copied);
        }
}

static void writeToFile(char *copied, FILE *outputFile, int pal){ //Writing the output to a specified output file, doing so with fwrite and writing each character, then adding palindrom or not
    int line = 0;

    while(copied[line]){
        fwrite(&copied[line], sizeof(copied[line]), 1, outputFile);
        line++;
    }

    if(pal == 1){
        fputs(" is a palindrom\n", outputFile);
    } else {
        fputs(" is not a palindrom\n", outputFile);
    }
}

int main(int argc, char *argv[]){

    char input[1024]; //Initialize the "String" (char array)
    int f,b,c = 0; //Initialize three variables to check from front and back (we increment those, if they're equal = palindrome), counter variable
    int opt = 0;
    int i = 0;
    int s = 0;
    int o = 0;
    char *o_arg = NULL;
    int fileUse = 0;
    int position = 1;
    FILE *inputFile;
    FILE *outputFile;

    while((opt = getopt(argc,argv,"sio:")) != -1){ //while loop for getting each parameter we provide along with calling the program
        switch (opt){
        case 'i': 
            i = 1;
            break;
        case 's':
            s = 1;
            break;
        case 'o':
            o = 1;
            o_arg = optarg;

            if(o_arg == NULL){
                printf("Error in -o argument, missing file!");
                exit(EXIT_FAILURE);
            } else {
                outputFile = fopen(o_arg, "w");
            }
            break;
        case '?':
            printf("Wrong input arguments!");
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
    }

    if(i == 1) {
        position++;
    }

    if(s == 1){
        position++;
    }

    if(o == 1){
        position += 2;
    } 

    if(optind < argc){
            fileUse = 1;
        }

    do { //do-while loop for continuous use when providing no input file, can be exited by pressing enter with no input given
            
        if(fileUse == 0){
            fgets(input, sizeof(input), stdin); //put our word into the char array = making it a string
        } else {
            for (int loop = position; loop < argc; loop++){
                inputFile = fopen(argv[loop], "r");

                if(inputFile != NULL){
                while(fgets(input, sizeof(input), inputFile)){
                        strtok(input, "\n"); //Parts the string at each \n which we receive with fgets as terminal symbol
                        b = strlen(input); //set our back variable to end of the word
                        listTraversal(input, outputFile, f, b, c, i, s, o);
                    }
                }
                fclose(inputFile);
            }
            fclose(outputFile);
            exit(EXIT_SUCCESS);
        }

        input[strlen(input) - 1]='\0'; //replace last place with \0 (terminal symbol, fgets adds \n as ts)
        b = strlen(input); //set our back variable to end of the word

        if(input[0] == '\0'){
            printf("Exiting program\n");
	    if(o == 1){
                fclose(outputFile);
            }
            exit(EXIT_SUCCESS);
        } 

        char copied[b];
        strcpy(copied, input);

        if(s == 1){ //Call help method remove_whitespace, removing all space inbetween the words thus creating a new input, setting back variable according to new length
            remove_whitespaces(input);
            b = strlen(input);
        }

        int pal = checkPalin(input, f, b, c, i);

        if(o == 1) {
            writeToFile(copied, outputFile, pal);
        } else {
            writeToConsole(copied, pal);
        }

        memset(input, 0, sizeof(input));
        f = 0; 
        b = 0;
        c = 0;
    } while(input[0] != '\n');
    exit(EXIT_SUCCESS);
}
