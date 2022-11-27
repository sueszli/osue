/**
 * @file mycompress.c
 * @author Branislav Balvan <e12023159@student.tuwien.ac.at>
 * @date 12.11.2021
 *
 * @brief Program that compresses stream of characters.
 * 
 * @details This program compresses a stream of characters either from standard input
 * or files, which can be passed as non-option program arguments. The comp-
 * ression summarizes character of the same type in succession to the speci-
 * fic character and the number of repetitions. The compressed stream can
 * be outputted either to standard output or specified file.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>

char *myprog; /**< The program name.*/

/**
 * Mandatory error function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details Global variables: myprog
 */
void error(void){
  fprintf(stderr, "Usage: %s [-o outfile] [file...]\n", myprog);
  exit(EXIT_FAILURE);
}

/**
 * @brief This function opens the file of the given name.
 * @details Used for opening inputs files for the program.
 * @param name Name of the file.
 * @return Returns pointer to the file for reading.
 */
FILE* openInputFile(char *name){
  FILE *new = fopen(name, "r");
  if(new == NULL){
    fprintf(stderr, "Cannot find file: %s\n", name);
    exit(EXIT_FAILURE);
  }

  return new;
}

/**
 * @brief This counts digits of a number.
 * @details Used for counting the digits for correct written character assumption.
 * @param num This number's digits will be counted.
 * @return Returns number of digits.
 */
int getDigits(int num){
	int cnt = 0;
	do {
		num /= 10;
		++cnt;
	}while(num != 0);
	
	return cnt;
}

/**
 * Main function of the module.
 * @brief The main section of the compression program compress the given stream of characters.
 * @details First the arguments get parsed and depending on them the correct input
 * and output source will be chosen. From input all the characters will be combined
 * into the selected output destination.
 * global variables: myprog
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
  myprog = argv[0];
  int opt, o_flag = 0;
  char *optr = NULL;
  FILE *outf_ptr = NULL;

  while((opt = getopt(argc, argv, ":o:")) != -1){
    switch (opt){
      case 'o':
        o_flag++;
        optr = optarg;
        break;
      case ':':
        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        error();
      case '?':
       if (isprint(optopt)){
          fprintf(stderr, "Unknown option `-%c'.\n", optopt);
          error();
        }else{
          fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
          error();
        }
    }
  }

  if(o_flag > 1){
    fprintf(stderr, "Option -o was used %i times, but cannot be used more than once. \n", o_flag);
    error();
  }

  //Open output file for writing.
  if(optr != NULL){
    outf_ptr = fopen(optr, "w");
  }else{
	outf_ptr = stdout;
	}

  long cn = 0, wn = 0, cc = 0;
  int c,l = EOF;

  /* Get all of the non-option arguments (input files). */
  if (optind < argc) 
  {
    // There are files to be read.
    FILE *inpf_ptr;

    while (optind < argc){
      inpf_ptr = openInputFile(argv[optind]);
      // Read from current file.
      
      while((c = fgetc(inpf_ptr)) != EOF){
		  cn++;
		  if(c == l){
				cc++;
			}else{
				if(l != EOF ){
					//Output either to output file or stdout.
					fputc(l, outf_ptr);
					fprintf(outf_ptr,"%ld",cc);
					wn += 1 + getDigits(cc);
				}
				cc = 1;
			    l = c;
			}
		}
	
		if (optind + 1 >= argc){
			fputc(l, outf_ptr);
			fprintf(outf_ptr,"%ld\n",cc);
			wn += 1 + getDigits(cc);	
		}
		fclose(inpf_ptr);
		optind++;
    }
  }else{
    // Read from input.
    do{
      if((c = getchar()) != EOF)
        ++cn;
      if(c == l){
        ++cc;
      }else{
        if(l != EOF){
          //Output either to output file or stdout.
          fputc(l, outf_ptr);
          fprintf(outf_ptr,"%ld",cc);
          wn += 1 + getDigits(cc);
        }
        cc = 1;
        l = c;
      }
    }while(c != EOF);
    printf("\n");
  }

  if(outf_ptr != NULL)
    fclose(outf_ptr);
  
  float r = ( (float) wn / cn) * 100.0f;
  fprintf(stderr,"\nRead:    %ld characters \nWritten: %ld characters \nCompression ratio: %.1f%%\n", cn, wn, r);
}
