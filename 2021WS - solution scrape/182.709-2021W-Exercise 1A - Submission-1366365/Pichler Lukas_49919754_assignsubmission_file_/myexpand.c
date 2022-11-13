#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/** 
 * @name    myexpand
 * @brief   A program that converts tabs to an appropriate amount of spaces in a file or the command line.
 * @details This program replaces all tabs of all input lines with the correct amount of spaces
 *          in order to keep the same intendation as with the tabs.
 *          It either works in file-mode or commandline-mode. In both modes it is possible 
 *          to adjust the tabcount via the option -t tabcount.
 *          Optionally, you can also write to an output file with the option -o outfile.
 *          All options work with commandline- or filemode.
 * @author  Lukas Pichler, 01617432
 * @date    14.11.2021
 */

char *myprog;

/**
 * @brief   A function that describes the usage of the program if a wrong option was provided.
 * @param   void
 * @return  void
 */
static void usage(void) {
  fprintf(stderr,"Usage: %s [-t tabcount] [-o outfile] file(s) \n", myprog);
  exit(EXIT_FAILURE);
}

int main(int argc, char*argv[]) {

  myprog = argv[0];
  int tabcount = 8;
  FILE *fp; ///< input file
  FILE *wf; ///< output file

  char *t_arg = NULL;
  char *o_arg = NULL;
  int c;
  while ( (c = getopt(argc, argv, "t:o:")) != -1 ){
    switch ( c ) {
      case 't': t_arg = optarg;
        break;
      case 'o': o_arg = optarg;
        break;
      case '?': usage();
        break;
      default:
        break;
    }
  }
  
  /**
   * if the option -t is used, the tabcount will be changed accordingly.
   */
  if (t_arg != NULL) {
    int num;
    sscanf (argv[2],"%d",&num);
    tabcount = num;
  }

  /**
   * Command Line Mode:
   * If no input files are provided, the program starts its commandline mode.
   * The program reads line by line from the console and evaluates them to the correct result.
   */
  if (argc - optind == 0) {
    if (o_arg != NULL) {
      if( (wf = fopen(o_arg, "w")) == NULL) {
        fprintf(stderr,"Error while opening file in program %s. o_arg: %s", myprog, o_arg);
        return EXIT_FAILURE;
      }
    }
    for (;;) {
      if (o_arg != NULL) {
        if( (wf = fopen(o_arg, "a")) == NULL) {
          fprintf(stderr,"Error while opening file in program %s", myprog);
          return EXIT_FAILURE;
        }
      }
      char str[1000];
      fgets(str, 1000, stdin);
      str[strcspn(str, "\n")] = 0;
      int index = 0;
      int out_index = 0;
      while (str[index] != 0) {
        if (str[index] == '\t') {
          ///< here the correct amount of whitespaces is calculated
          int whitespaces = (tabcount * (( out_index / tabcount) + 1)) - out_index;
          out_index += whitespaces;
          for(int j = 0; j < whitespaces; j++) {
            if (o_arg == NULL) {
              fwrite(" ", 1, 1, stdout);
            } else {
              fwrite(" ", 1, 1, wf);
            }
          }
        } else {
          if (o_arg == NULL) {
              fwrite(&str[index], 1, 1, stdout);
            } else {
              fwrite(&str[index], 1, 1, wf);
            }
            out_index++;
        }
        index++;  
      }
      if (o_arg != NULL) {
        fwrite("\n", 1, 1, wf);
        fclose(wf);
      } else {
        fwrite("\n", 1, 1, stdout);
      }
    }
    
  /**
   * Input File Mode
   * If one or multiple input files are provided, the program reads every file
   * line by line and evaluates them to get the correct result.
   */  
  } else {
    if (o_arg != NULL) {
        if ((wf = fopen(o_arg, "w")) == NULL) {
          fprintf(stderr,"Error while opening file in program %s", myprog);
          return EXIT_FAILURE;
        }
    }
    for(int filenr = optind; filenr < argc; filenr++) {
      if ((fp = fopen(argv[filenr], "r")) == NULL) {
        fprintf(stderr,"Error while opening file in program %s", myprog);
        return EXIT_FAILURE;
      }
      char *line = NULL;
      size_t linecap = 0;
      ssize_t linelen;
      while ((linelen = getline(&line, &linecap, fp)) > 0) {
        int out_index = 0;
        for(int i = 0; i < linelen; i++) {
            if (line[i] == '\t') {
              ///< here the correct amount of whitespaces is calculated
              int whitespaces = (tabcount * (( out_index / tabcount) + 1)) - out_index;
              out_index += whitespaces;
              for(int j = 0; j < whitespaces; j++) {
                if (o_arg == NULL) {
                  fwrite(" ", 1, 1, stdout);
                } else {
                  fwrite(" ", 1, 1, wf);
                }
              }
            } else {
              out_index++;
              if (o_arg == NULL) {
                fwrite(&line[i], 1, 1, stdout);
              } else {
                fwrite(&line[i], 1, 1, wf);
              }
            }
        }
      }
      if (o_arg != NULL) {
        fwrite("\n", 1, 1, wf);
      } else {
        fwrite("\n", 1, 1, stdout);
      }
    }  
    if (o_arg != NULL) {
      fclose(wf); 
    }
    fclose(fp);
  }
  return EXIT_SUCCESS;
}
