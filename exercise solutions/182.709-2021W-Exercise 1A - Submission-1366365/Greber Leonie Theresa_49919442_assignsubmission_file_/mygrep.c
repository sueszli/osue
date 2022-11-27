/**
 * @file mygrep.c
 * @author Leonie Greber (11801674)
 * 
 * @date 11.10.2021
 * 
 * @brief Main programm module
 * 
 * @details This programm implements a version of the c-function grep. 
 * It prints all lines from stdin or one or more inputfiles which contain a specified keyword.
 * 
**/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>

//programm name
const char *myprog;

/**
 * @brief Prints usage message and exits program with failure.
 */
static void usage(void) {
	fprintf(stderr,"Usage: %s mygrep [-i] [-o outfile] keyword [file...]", myprog);
	exit(EXIT_FAILURE);
}

/**
 * @brief Converts all chars of String to uppercases.
 * 
 * @param str a c-string, which lowercase letters should be changed into uppercase letters
 */
static void toUpper (char * str) {
	for(; *str != '\0'; str++){
		*str = toupper(*str);
	}
}


/**
 * @brief Prints lines which contain specific keyword in file or stdout (definded by parameter out).
 * @param case_insensitive, if 1 function is case insensitive otherwise case sensitive
 * @param in, file, where the input is read from
 * @param out, file, where the output is writen to
 * @param keyword, keyword which we search in each line
 */
static void mygrep (int case_insensitive, FILE *in, FILE *out, char *keyword) {
    char *line = NULL;
    size_t size = 0;

    char * copied_line = NULL;

    while (getline(&line, &size, in) != -1) {

		if(strlen(line) == 0){
			continue;
		}

        copied_line = strdup(line);
		
        if (case_insensitive == 1){
            toUpper(copied_line);
            toUpper(keyword);
        }

        if ((strstr(copied_line, keyword)) != NULL){
            if (fputs(line, out) == EOF){
                fclose(out);
                free(line);
                free(copied_line);
                fprintf(stderr, "fputs failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
        }
    }
    
    free(line);
    free(copied_line);
    fclose(in);
}


/**
 * @brief Program entry point.
 * @details prints all lines from inputfiles which contain a keyword
 * global variables: myprog (name of the program)
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS upon succesful execution.
 */
int main (int argc, char **argv) {
    myprog = argv[0];
    char *outputfile = NULL;
    FILE *in, *out;
    int c;
    int optionI = 0, optionO = 0;

    while ((c = getopt(argc, argv, "io:")) != -1)
    {
        switch (c)
        {
        case 'i':
            optionI++;
            break;
        case 'o':
            outputfile = optarg; //optarg contains option-argument(char *)
            optionO++;
            break;
        case '?':
            usage(); //invalid programm option
            break;
        default:
            assert(0);
            break;
        }
    }

  /*check occurence of options*/
  if (optionI > 1 || optionO > 1) {
      usage();
  }
  
  /*set output*/
  if (outputfile == NULL) {
	out = stdout;
  } else {
	out = fopen(outputfile, "w"); //write-only
  }
  if (out == NULL) {
	fprintf(stderr, "fopen failed: %s\n", strerror(errno));
	exit(EXIT_FAILURE);
  }
  
  /*get keyword*/
  char *keyword;

  if ((argc - optind) >= 1) {
      keyword = argv[optind++];  //increment, so that optind is index of first inputfile (if exists)
  } else {
      if (out != stdout) {
          fclose(out);
      }
      usage();
  }

  /*check input and calc mygrep*/

  if (optind >= argc){
      //no inputfiles
      in = stdin;
      mygrep(optionI, in, out, keyword);
  } else {
      int numberOfFiles = argc - optind; //optind = index of the next element in argv

      while (numberOfFiles != 0)
      {
          in = fopen(argv[optind++], "r");
          
          if (in == NULL) {
              fprintf(stderr, "fopen failed: %s\n", strerror(errno));
              exit(EXIT_FAILURE);
            }
          mygrep(optionI, in, out, keyword);
          numberOfFiles--;
      }  
  }


  if (out != stdout) {
        fclose(out);
    }

    return EXIT_SUCCESS;
}



