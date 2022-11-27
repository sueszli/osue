
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>


/**
 * @author Cosic Adrian, 12025964
 * @brief Checks if keyword is contained in a line
 * @date 09/11/2021
 * @details mygrep [-i] [-o outfile] keyword [file...], the user has to give the program a keyword and at least one file or line to work with, the program
 *          will then check if the keyword is contained in each of the lines in the text file or in the given line and will output the line to either stdout or 
 *          to a user specified output file. The user can decide between 2 options. (-i) will ignore case sensitivity and (-o) will write the output to a file, which has to be specified.
 *          The first thing the program does is check if there are any commands. If not the programm will close
 *          Secondly all options (if any) are read and parameters are set. The keyword pointer (keywordPos) will shift according to the options. For (-i) the keyword pointer will increase by one.
 *          For a successful use of (-o) the keyword pointer will increase by 2 (option + file). The programm then checks if the keyword is actually there (if nothing given after the options)
 *          Now that the program knows what the keyword is, the word will be written in kword. depending on if the user wants to include case sensitivity, kword will be as given or completly lowercase
 *          The programm will then go through every command. it will check if its a file or not and will then act accordingly. If its not a file, the programm will treat a input as a line
 *          If its a file, the programm will then check every line for the keyword. Every line will be copied unto normalLine. and then depending on case sensitivity normalLine writes itsself onto line.
 *          line is the variable that will be checked for the keyword. normalLine will be output.
 *          is the keyword contained. the programm will then either put normalLine in stdout or will write in the given file
 * 
 * @param caseSen 0 if the programm ignores case sensitivity, 1 if it does not
 * @param keywordPos the pointer to the position of the keyword. will change depending on the options
 * @param output 1 if the output is written on a file, 0 if not
 * @param c char of the option, used in getopt
 * @param o_arg is the option argument of -o, should contain the file name of the file being written on
 * @param infile pointer to the input file
 * @param outfile pointer to the output file
 * @param line is the line being worked with. will be either all lowercase if case sensitivity is ignored, or will be the same as normalline
 * @param normalLine the line from input is being written here. copies onto line. only will be outputted, since this is the unmodified line
 * @param kword contains the keyword, will be lowercase or not, depending on user input
 * @return int 
 */


int main(int argc, char *argv[])
{

    int caseSen = 1;
    int keywordPos = 1;
    int output = 0;
    int c;
    char line[1000];                               //line: zeile, bei jeder iteration neu initialized, temp speicher um zu prüfen ob keyword drinnen ist
    char normalLine[1000]; 
    char *o_arg = NULL;
    FILE *infile;
    FILE *outfile;

    if (argv[1] == NULL)    //error if no input
    {
        printf("ERROR argv[0] %s\n : no input \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while ((c = getopt(argc, argv, "io:")) != -1) //geht die optionen durch und stellt werte dementsprechend ein, bei fehl angabe error
    {
        switch (c)
        {
        case 'i':
            caseSen = 0;
            break;
        case 'o':
            o_arg = optarg;
            if (o_arg == NULL)  //if no argument found, error
            {
                printf("ERROR argv[0] %s\n : no arguments for -o \n", argv[0]);
                exit(EXIT_FAILURE);
            }
            else
            {
                outfile = fopen(o_arg, "w");    //opens the file to write
                if (outfile == NULL)    //just in case
                {
                    printf("ERROR argv[0] %s\n : fopen failed -o, argument might be wrong \n", argv[0]);
                    exit(EXIT_FAILURE);
                }
                output = 1; //sets output to 1, will later be relevant
            }

            break;
        case '?': //invalid option
            printf("ERROR argv[0] %s\n : invalid option \n", argv[0]);
            exit(EXIT_FAILURE);
            break;
        default: //in case
            return 1;
        }
    }

    if (caseSen == 0) //depending on options will increase the pointer for the keyword
        keywordPos++;
    if (output == 1)
        keywordPos += 2;

    if (argv[keywordPos] == NULL)
    {
        printf("ERROR argv[0] %s\n : no lines or files given %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int i = 0;
    char kword[1000];
    while (argv[keywordPos][i]) //schreibt das keyword in variabel, konvertriert das keyword in lowercase wenn gefordert
    {
        if (caseSen == 0) //lowercase
        {
            kword[i] = (tolower(argv[keywordPos][i]));
            i++;
        }
        else //case sensitive
        {
            kword[i] = (argv[keywordPos][i]);
            i++;
        }
    }

    for (int i = keywordPos + 1; i < argc; i++) //geht jeden cmd input nach dem keyword durch
    {
        int j = 0;
        infile = fopen(argv[i], "r");
        if (infile==NULL) //checkt ob input eine file ist, bei infile == null ist es keine, geht anderen ablauf durch
        {
            if (caseSen == 0)
            {
                while (argv[i][j])
                { //konvertiert die zeile in lowercase
                    line[j] = (tolower(argv[i][j]));
                    normalLine[j] = (argv[i][j]);
                    j++;
                }
            }
            else
            {
                while (argv[i][j])
                {
                    line[j] = (argv[i][j]);
                    normalLine[j] = (argv[i][j]); //muss auch sein weil program gibt normalLine aus
                    j++;
                }
            }

            if (strstr(line, kword)) //output
            {
                if (output == 0) //will output in stdout
                {
                    printf("%s\n", normalLine);
                }
                else
                {
                    j = 0;
                    while (normalLine[j]) //goes through every char of the line
                    {
                        fwrite(&normalLine[j], sizeof(normalLine[0]), 1, outfile);
                        j++;
                    }
                    fwrite("\n", sizeof(char), 1, outfile); //will write a next line when the line is done
                }
            }
            memset(&normalLine[0], 0, sizeof(normalLine)); //cleart alle einträge
            memset(&line[0], 0, sizeof(line));
        }
        else //liest file input zeile für zeile
        {

            while (fgets(normalLine, 1000, infile)) //itereates through every line, NULL if done (no more lines left)
            {
                j = 0;
                if (caseSen == 0)
                {
                    while (normalLine[j]) //same as above, lowercase
                    { //konvertiert die zeile in lowercase
                        line[j] = (tolower(normalLine[j]));
                        j++;
                    }
                }
                else
                {
                    while (normalLine[j]) //case sensitive
                    {
                        line[j] = (normalLine[j]);
                        j++;
                    }
                }


                
                if (strstr(line, kword)) // output, if string contained
                {
                    if (output == 0)
                    {
                        printf("%s", normalLine);
                    }
                    else
                    {
                        j = 0;
                        while (normalLine[j])
                        {
                           fwrite(&normalLine[j], sizeof(normalLine[0]), 1, outfile);
                            j++;
                        }
                    }
                }

                memset(&normalLine[0], 0, sizeof(normalLine)); //cleart alle einträge
               memset(&line[0], 0, sizeof(line));
            }

    if (infile)
        fclose(infile);
        }
    }

    if (output == 1) //closes output and input, no leaks
        fclose(outfile);

    if (infile)
        fclose(infile);

    exit(EXIT_SUCCESS);
}
