/**
 * @file ispalindrom.c
 * @author Daniel Blattner <e12020646@students.tuwien.ac.at>
 * @date 02.11.2021
 *
 * @brief Main programm module.
 *
 * This program check if lines from a file or the terminal are palindroms. 
 * Arguments can define if the check is case (in)sensitive or if whitespaces are ignored.
 * The result for each line can be printed to the terminal or to a separate file.
 *
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>
#include <ctype.h>

/**
 * @struct arguments_t
 * @brief This struct indicate how often each argument is counted. Furthermore it safes 
 * the path of the optional output file.
**/
typedef struct
{
    /** if the value > 0 ignore white spaces in word **/
    unsigned int ignoreSpaces;
    /** if the value > 0 plaidrom check is case insensiive **/      
    unsigned int caseInsensitive;  
    /** if the value > 0 print result to file as defined in outfile **/ 
    unsigned int printFile;    
    /** path of file where results are printed **/     
    char *outFile;                  
} arguments_t;

/** name of programm as defined in argv[0] **/
static const char *prog_name;   

/**
 * @brief This function prints a custom error message to stderr. The message contain the name of the
 * programm, a specific messages (msg) and the string as defined in errno. After the programm is exited 
 * with the status EXIT_FAILURE. 
 * @details global variables: prog_name
 * @param msg String with a specific description of the error.
**/
static void printErrorMsg(char *msg)
{
    fprintf(stderr, "[%s] ERROR: %s: %s\n", prog_name, msg, strerror(errno));
    exit(EXIT_FAILURE);
}

/**
 * @brief This function prints the right usage of the arguments of this programm. This line is printed
 * when an error with the arguments occurred.
 * @details global variables: prog_name
**/
static void printProperUsage(void)
{
    fprintf(stderr,"Usage: %s [-s] [-i] [-o outfile] [file...]\n", prog_name);
    exit(EXIT_FAILURE);
}

/**
 * @brief This function is reading the arguments and create a arguments_t to safe and return the values.
 * If an invalid agruments is read, the proper usage is printed to the terminal. 
 * @details Multiple occurences of an arguments have no impact on the programm. Only the last output file 
 * (-o file) will be written with the results.
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @return Sturct with all necessary information of the arguments
**/
static arguments_t parseArguments(int argc, char *argv[])
{
    arguments_t parse = {0,0,0,NULL};
    int c;

    while( (c = getopt(argc,argv, "sio:")) != -1){
        switch (c) {
            case 's': //Ignore whitespaces
                parse.ignoreSpaces++;
                break;
            case 'i': //Case insensitive
                parse.caseInsensitive++;
                break;
            case 'o': //Result to file
                parse.printFile++;
                parse.outFile = optarg;
                break;
            case '?': //Invalid option 
                printProperUsage();
                break;
            default:
                printProperUsage();
                break;
        }
    }

    if(parse.ignoreSpaces > 1) printf("Info: Multiple '-s' arguments will be ignored\n");
    if(parse.caseInsensitive > 1) printf("Info: Multiple '-o' arguments will be ignored\n");
    if(parse.printFile > 1) printf("Info: Only last '-o' is valid\n");

    return parse;
}

/**
 * @brief This function reads all positional arguments and copy them to an given array. If no 
 * positional are avaliable, the array will not be changed.
 * @details This is only a shallow copy.
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @param files The array where the positional arguments are copied to. 
**/
static void parsePositionalArguments(int argc, char *argv[], char **files)
{
    assert(files != NULL);

    //No positional arguments
    if(argc == optind) return; 

    //Copy positional arguments
    for(int i = 0; i < argc-optind; i++) {
        assert(i < argc);
        files[i] = argv[optind+i];
    }
}

/**
 * @brief This function prints if the word is a palindrom or not accordingly to
 * the result boolean. The string is printed to a given stream. Error detection 
 * and handling is not processed in this function. 
 * @param stream The pointer to the stream.
 * @param word The word which was checked.
 * @param result The result of the inspection.
**/
static void printfResultToStream(FILE *stream, char *word, bool result)
{
    if(result){   
        fprintf(stream, "%s is a palindrom\n", word);
    }
    else{
        fprintf(stream, "%s is a not palindrom\n", word);
    }
}

/**
 * @brief This function prints the result of the word to a file, if the arguments 
 * include an output file. Otherwise it prints the result to the terminal (stdout). 
 * @details If the result can not be written to the output file, the programm will 
 * be aborted. 
 * @param stream The stream from which the lines are read.
 * @param word The word which was checked.
 * @param result The result of the inspection.
 * @param arg The arguments of the programm.
**/
static void printResult(FILE *stream, char *word, bool result, arguments_t arg)
{
    //Print result to outfile
    if(arg.printFile > 0) {
        FILE *out = fopen(arg.outFile, "a");
        if(out == NULL){
            free(word);
            fclose(out);
            fclose(stream);
            printErrorMsg("Failed to open file");
        }

        printfResultToStream(out,word,result);
        if(ferror(out)){
            free(word);
            fclose(out);
            fclose(stream);
            printErrorMsg("Failed to write to file");
        }

        fclose(out);
    }
    //Print result to terminal
    else {
        printfResultToStream(stdout,word,result);
    }
}

/**
 * @brief The functions takes the string (word) and change it according to the arguments
 * of the programm. If the arguments include a '-s', all whitespaces will be removed. If
 * the arguments include a '-i', all characters will be converted to its lower case. 
 * Therefore the word will be case insentisive.
 * @details When all whitespaces are removed, the string will be cut with a '\0' at the 
 * new end. The behavior of tolower is not defined, when the character is a umlaut 
 * (as seen in tolower(3)).
 * @param word The word which will be changed.
 * @param arg The arguments of the programm.
**/
static void applyArguments(char *word, arguments_t arg)
{
    assert(word != NULL);

    //Clear all whitespaces
    if(arg.ignoreSpaces > 0) {
        unsigned int index = 0;
        //Overwrites every whitespace with next char
        for(int i=0; i<strlen(word); i++) {
            if(word[i] != ' '){
                word[index++] = word[i];
            }
        }
        //Cut new string at end
        word[index] = '\0'; 
    }
    //Change all char to its respective lowercase
    if(arg.caseInsensitive > 0) {
        for(int i=0; i<strlen(word); i++){
            word[i] = tolower(word[i]);
        }
    }
}

/**
 * @brief This function applies first of all the needed changes of the word according to
 * the arguments. Then it will check if the modified word is a palindrom. The result will
 * be printed as defined in the arguments.
 * @details The word will not be changed during the process. If the modified word is empty,
 * the word will be viewed as a palindrom.
 * @param stream The stream from which the lines are read.
 * @param word The word which will be checked.
 * @param arg The arguments of the programm.
 * @return The result of the check.
**/
static bool checkLine(FILE *stream, char *word, arguments_t arg)
{
    assert(word != NULL);

    //Copy word to apply required changes as defined in arguments
    char *workWord = malloc(strlen(word)*sizeof(char));
    strcpy(workWord,word);

    //Applies arguments to workWord
    applyArguments(workWord,arg);
    
    bool result = true;
    unsigned int lenght = strlen(workWord);

    if(lenght != 0) {
        unsigned int halfLenght = lenght/2;
        //Check every char to its corresponding counterpart
        for(int i=0; i <= halfLenght; i++) {
            if(workWord[i] != workWord[lenght-i-1])
            {
                result = false;
                break;
            }
        }
    }
    
    //Print result according to the arguments
    printResult(stream, word, result, arg);

    return result;
}

/**
 * @brief This function get an readable stream. Each line from the stream will be checked if it 
 * is an palindrom. Before the line check the '\n' character will be removed. The result for 
 * each line will be printed according to the arguments. When all lines are palindroms, 
 * the function return true, otherwise false. 
 * @details A file read error will not be handeld. This must be processed by the calling function.
 * If the line buffer is too short for the whole line, its lenght will be doubled. This will be 
 * repeated until more memory can not be allocated. In that case an error message will be printed
 * to stderr.
 * @param stream The stream from which the lines are read.
 * @param arg The arguments of the programm.
 * @return If all lines in stream are palindroms.
**/
static bool checkStream(FILE *stream, arguments_t arg)
{
    //Maximum lenght of a line
    size_t maxLenght = 500; 
 
    char *line = malloc(maxLenght * sizeof(char));
    if(line == NULL){
        free(line);
        fclose(stream);
        printErrorMsg("Could not allocate memory");
    }

    bool allPalindrom = true;
    //Read line from stream
    while(fgets(line,maxLenght,stream) != NULL){
        //Line buffer is too small, double size of buffer and try again
        while(line[strlen(line)-1] != '\n' && feof(stream) == 0){
            char *biggerLine = realloc(line, 2*maxLenght*sizeof(char));
            if(biggerLine == NULL){
                free(biggerLine);
                fclose(stream);
                printErrorMsg("Could not allocate bigger memory");
            }
            if(fseek(stream, -(double)maxLenght, SEEK_CUR) != 0){
                free(biggerLine);
                fclose(stream);
                printErrorMsg("Could not change cursor of file");
            }
            line = biggerLine;
            maxLenght *= 2;
            if(fgets(line, maxLenght, stream) == NULL){
                free(line);
                fclose(stream);
                return allPalindrom;
            }
        }
        //Cut \n from line
        line[strcspn(line, "\n")] = '\0';
        //Check line for palindrom
        allPalindrom &= checkLine(stream,line,arg);
    }

    free(line);
    return allPalindrom;
}

/**
 * @brief This function tries to open a file from the given file path. If the file can not
 * be opened or read an error message will be printed to stderr. All lines will be tested
 * if they are palindroms. The result for each line will be printed according to the arguments.
 * @param filePath The path of the file, which should be read.
 * @param arg The arguments of the programm.
 * @return If all lines in file are palindroms.
**/
static bool checkFile(char *filePath, arguments_t arg)
{
    FILE *file = fopen(filePath, "r");
    if(file == NULL){
        fclose(file);
        printErrorMsg("Failed to open file");
    }

    //Check every line in file
    bool filePalindrom = checkStream(file,arg); 
    if(ferror(file)){
        fclose(file);
        printErrorMsg("Failed to read from file");
    }

    fclose(file);
    
    return filePalindrom;
}

/**
 * @brief This function read from the terminal (stdin) if no file paths are given. 
 * Otherwise all given file paths are opened and read. Each line will be separately 
 * checked, if it is a palindrom. The result for each line will be printed according
 * to the arguments.
 * @details When the input is the terminal (stdin), lines will be read indefinitely.
 * @param inFile The array of file paths, which lines are to be checked.
 * @param inFileLen The lenght of the array of file paths.
 * @param arg The arguments of the programm.
 * @return If all lines from all files are palindroms. 
**/
static bool isPalindrom(char *inFile[], unsigned int inFileLen, arguments_t arg)
{
    bool result = false;

    //Lines from terminal
    if(inFileLen == 0) {
        checkStream(stdin, arg);
    }
    //Lines from files (positional arguments)
    else {
        for (int i=0; i<inFileLen; i++)
        {
            result &= checkFile(inFile[i],arg);
        }
    }

    return result;
}

/**
 * @brief The programm starts here. This function sets the global variable prog_name 
 * as defined is argv[0]. Then the arguments will be processed and saved in a struct.
 * After that the positional arguments are extracted and saved in an array. Last but
 * not least each line from the terminal/files will be altered (defined by the 
 * arguments) and checked. The results will be printed accoring to the arguments.
 * @details The opterr flag will be set to 0, because the getopt errors are
 * handeld manually.
 * global variables: prog_name
 * @param argc The argument counter. 
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
**/
int main(int argc, char *argv[])
{
    //Programm name
    prog_name = argv[0]; 
    //Own error handeling in case of a getopt error
    opterr = 0; 
    //Extract arguments
    arguments_t arg = parseArguments(argc,argv); 

    unsigned int inFileLen = argc-optind;
    char *inFile[inFileLen];
    //Extracts positional arguments
    parsePositionalArguments(argc,argv, inFile); 

    //Determine input stream and check lines if they are 
    isPalindrom(inFile, inFileLen, arg);

    return EXIT_SUCCESS;
}
