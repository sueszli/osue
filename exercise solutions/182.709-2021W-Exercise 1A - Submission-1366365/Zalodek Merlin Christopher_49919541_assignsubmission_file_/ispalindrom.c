/**
 * @author Merlin Zalodek, 12019836
 * @date 07.11.2021
 * @brief implementation of task 1a - ispalindrom
 * @details 
 *  The program ispalindrom reads files line by line. For each line it checks whether it is a palindrom or not (i.e. whether the text read backwards is identical to itself)
 *  using 6 global variables
 *  3 flags (s,i,o) which were given by user
 *  outputFileName - user can declare a name for output file 
 *  programName = "ispalindrom" given from argv[0]
 *  outputFile - if option -o is active then outputFile will be written in userFile or userInput */

#include <stdlib.h> ///< Standard Library
#include <stdio.h> ///< Input and output functions, types and macros (e.g. stderr)
#include <unistd.h>
#include <ctype.h> ///< for isprint() & tolower()
#include <string.h>
#include <assert.h>


int s_flag = 0; ///< ignore white-spaces option
int i_flag = 0; ///< ignore lower / upper case option
int o_flag = 0; ///< output-file option
char *outputFileName = NULL;
char *programName;
FILE *outputFile;


/**
 * @brief a string will be converted to lower letters
 * @param string will be changed
 * @return string only with lower letters
*/
static char * ignoreCaseSensitive(char *string)
{
  for (int i = 0; string[i]; i++)
  {
    string[i] = tolower(string[i]);
  }
  return string;
}

/**
 * @brief method which removes white spaces of a given string
 * @param string will be changed
 * @return string without white spaces
*/
static char * removeWhiteSpaces(char *string)
{
	int i = 0, j = 0;
	while (string[i])
	{
		if (string[i] != ' ')
      string[j++] = string[i];
		i++;
	}
	string[j] = '\0';
	return string;
}

/**
 * @brief method which checks if a string is a palindrom
 * @param string will be checked if a palindrom is given
 * @return 1 if palindrom
 * @return 0 if not palindrom
 * @return 2 if string is NULL or empty line is given
*/
static int palindromCheck(char *string)
{
  if(string[0] == '\n' || string == NULL) return 2; ///< empty line (enter) or NULL
  // Starting from leftmost and rightmost corners of the string
  int left = 0;
  int right = strlen(string) - 1;

  // Keep comparing characters while they are same
  while (right > left)
  {
    if (string[left++] != string[right--])
    {
      return 0;
    }
  }
  return 1;
}

/**
 * @brief method which opens a file (dynamic input stream); calls palindromCheck; writes result in stdout or output file (palindrom/or not)
 * @details uses global variable programName - purpose: print name in case of an Error
 * @details uses global variable outputFile - purpose: export results in a user given external file
 * @details uses global variables  s_flag for checking  ignore white-spaces option is wanted by user
 *                                 i_flag for checking if ignore lower / upper case option is wanted by user
 *                                 o_flag for checking if output-file option is wanted by user
 * @param inputFileName file which includes text lines which should be checked if they are palindromes
 * @return 1 if successful
*/
static int userFile(char *inputFileName)
{
  FILE *inputFile = fopen(inputFileName, "r");
  if (inputFile == NULL)
  {
    fprintf(stderr, "%s : %s : Cannot access / open given file name.", programName, inputFileName);
    exit(EXIT_FAILURE);
  }
  size_t size = 0; ///< how large is the buffer
  ssize_t length = 0; ///< how long actual string
  char * line = NULL;

  while ((length = getline(&line, &size, inputFile)) >= 0)
  {
    strtok(line, "\n"); ///< removes next line-symbol of string
    char originalLine[length]; ///< output string
    strcpy(originalLine,line);

    if (s_flag == 1) removeWhiteSpaces(line);
    if (i_flag == 1) ignoreCaseSensitive(line);
    if (palindromCheck(line) == 1)
    {
      if (o_flag == 1)
        fprintf(outputFile,"%s is a palindrom\n",originalLine);
      else 
        fprintf(stdout,"%s is a palindrom\n",originalLine);
    }
    else if (palindromCheck(line) == 0)
    {
      if (o_flag == 1)
        fprintf(outputFile,"%s is not a palindrom\n",originalLine);
      else
        fprintf(stdout,"%s is not a palindrom\n",originalLine);
    }
    else if (palindromCheck(line) == 2){}
    else
    {
      fprintf(stderr, "%s : %s : Error on function palindromCheck", programName, originalLine);
    }
  }
  free(line);
  fclose(inputFile);
  return 1;
}

/**
 * @brief method which reads from stdin (Dynamic Memory Allocation); calls palindromCheck; writes result in stdout or output file (palindrom/or not)
 * @details uses global variable programName - purpose: print name in case of an Error
 * @details uses global variable outputFile - purpose: export results in a user given external file
 * @details uses global variables  s_flag for checking  ignore white-spaces option is wanted by user
 *                                 i_flag for checking if ignore lower / upper case option is wanted by user
 *                                 o_flag for checking if output-file option is wanted by user
 * @return 1 if successful
*/
static int userInput(void) // use ctrl + D to terminate the STDIN file
{
  char *userInput = NULL;
  size_t size = 64; ///< how large is the buffer
  ssize_t length = 0; ///< how long actual string
  userInput = malloc(size*sizeof(char));
  if (userInput == NULL)
  {
    fprintf(stderr, "%s : %s : Cannot allocate userInput", programName, userInput);
    exit(EXIT_FAILURE);
  }

  while ((length = getline(&userInput, &size, stdin)) >= 0)
  {
    strtok(userInput, "\n"); ///< removes next line-symbol of string
    char originalLine[length]; ///< output string
    strcpy(originalLine,userInput);

    if (s_flag == 1) removeWhiteSpaces(userInput);
    if (i_flag == 1) ignoreCaseSensitive(userInput);
    if (palindromCheck(userInput) == 1)
    {
      if (o_flag == 1)
        fprintf(outputFile,"%s is a palindrom\n", originalLine);
      else 
        fprintf(stdout,"%s is a palindrom\n", originalLine);
    }
    else if (palindromCheck(userInput) == 0)
    {
      if (o_flag == 1)
        fprintf(outputFile,"%s is not a palindrom\n", originalLine);
      else
        fprintf(stdout,"%s is not a palindrom\n", originalLine);
    }
    else if (palindromCheck(userInput) == 2){}
    else
    {
      fprintf(stderr, "%s : %s : Error on function palindromCheck", programName, originalLine);
    }
  }
  free(userInput);
  return 1;
}

/**
 * @brief main method which includes function getopt() for handling options this program
 * @details uses global variable programName - purpose: print name in case of an Error (initialization)
 * @details uses global variable outputFileName - purpose: export results in an external file with this name (initialization)
 * @details uses global variable outputFile - purpose: export results in a user given external file (initialization)
 * @details uses global variables  s_flag for checking  ignore white-spaces option is wanted by user (initialization)
 *                                 i_flag for checking if ignore lower / upper case option is wanted by user (initialization)
 *                                 o_flag for checking if output-file option is wanted by user (initialization)
 * @param argc : Number of parameters
 * @param argv : includes argument values
 * @return 1 if successful
*/
int main (int argc, char **argv)
{
  int c;

  opterr = 0;
  programName = argv[0];
  while ((c = getopt (argc, argv, "sio:")) != -1)
    switch (c)
      {
      case 's':
        s_flag = 1;
        break;
      case 'i':
        i_flag = 1;
        break;
      case 'o':
        outputFileName = optarg;
        o_flag = 1;
        break;
      case '?':
        if (optopt == 'o')
        {
          fprintf (stderr, "%s : Option -%o requires an argument / an output file.", programName, optopt);
          exit(EXIT_FAILURE);
        }
        else if (isprint(optopt)) ///< checks whether a character is a printable character or not.
        { 
          fprintf (stderr, "%s : Unknown option `-%c'.", programName, optopt);
          exit(EXIT_FAILURE);
        }
        else
        {
          fprintf (stderr, "%s : Unknown option character `\\x%x'.", programName, optopt);
          exit(EXIT_FAILURE);
        }
        default:
          assert(0);
      }


  if (o_flag == 1) outputFile = fopen(outputFileName, "w"); ///< open output file stream
  if ((outputFile == NULL) && (o_flag == 1))
  {
    fprintf(stderr, "%s : %s : Error at outputFile.", programName, outputFileName);
    exit(EXIT_FAILURE);
  }

  if (optind == argc) ///< no arguments given, reading from stdin
  {
    userInput();
  }

  for (int index = optind; index < argc; index++) ///< input files
  {
    userFile(argv[index]);
  }
  
  if (o_flag == 1) fclose(outputFile); ///< close output file stream
  exit(EXIT_SUCCESS);
}
