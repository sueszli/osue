#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>

/**\file mydiff
 * @author Peter Nyikos
 * @brief This Program compares 2 files and prints out, how these differ.
 * @details It reads in two files and compares them. 
 * If two lines differ, then the line number and the number of differing characters is printed.
 * SYNOPSIS: mydiff [-i] [-o outfile] file1 file2
 * The program shall read each file line by line und compare the characters. 
 * If two lines have different length, then the comparison shall stop upon reaching the end of the shorter line.
 * Therefore, the lines abc\n und abcdef\n shall be treated as being identical.
 * This program accepts lines of any length.
 * If the option -o is given, the output is written to the specified file (outfile).
 * Otherwise, the output is written to stdout.
 * If the option -i is given, the program shall not differentiate between lower and upper case letters, i.e.
 * the comparison of the two lines shall be case insensitive.
 * @date 13/11/2021
 */

/**\var
 * This variable containes the name of the program and is set when main() gets called.
 */
char *myprog;

/**
 * @brief This function is used in case the program is passed wrong parameters.
 * @details It prints the correct usage and exits with failure.
 * The global variable *myprog is used to print the program name.
 */
static void usage(void){
	fprintf(stderr,"Usage: %s [-i] [-o outfile] file1 file2", myprog);
	exit(EXIT_FAILURE);	
}

/**
 * @breif This function is used to get some data about a file.
 * @details Speciffically the Maximum number of characters in a Row, and the Number of Rows.
 * @param *file the file about which the data is collected.
 * @param returnVal[2] an integer Array in which the data gets saved. 
 * returnVal[0] contains Row-Lenght Info.
 * returnVal[1] contains the Number of Rows.
 * @return 0 if success, -1 if failure
 */
static int getFileMetaData(FILE *file, int returnVal[2]){
	int c;		
	int count;	/**< Current Chars on this Row*/
	int max;	/**< Max Chars on any Row*/
	int rows;	/**< Nr of Rows in total*/
	count = 0;
	rows = 0;
	max = 0;
	while(1 == 1){
		c = fgetc( file );
		if( c == '\n'){
			if(max < count){
				max = count;
			}
			rows++;
			count = 0;
		} else if(c == EOF){
			if(max < count){
				max = count;
			}
			if(count!=0){
				rows++;
			}
			break;
		} else{
		count++;
		}
	}
	
	if(fseek(file,0,SEEK_SET)!=0){
		return -1;
	}
	returnVal[0] = max+2; //max is increased by 2 to account for different Line Endings
	returnVal[1] = rows;
	return 0;
}

/**
 * @brief This function is used to compute the Number of different Chars between 2 strings.
 * @details This function functions similar to strcmp, but was implemented, because the return Value
 * of Strcmp does not show how many Chars are different.
 * The strings get compared char by char until either:
 * -One string ends.
 * -One string contains '\n' (LF).
 * -One string contains '\r' (CR).
 * @param *ln1 the first string to compare
 * @param *ln2 the second string to compare
 * @param ignoreCase used to determine if the function should behave case-sensitive. 
 * (ignoreCase==0) means the function is case-sensitive
 * (ignoreCase!=0) means the function is not case-sensitive
 * @return the number of Chars that differ between the strings.
 */
static int getNrOfDifferentChars(char *ln1, char *ln2, int ignoreCase){
	int n;			/**< number of Chars in shorter String*/
	int count = 0;  /**< How many Chars differ between the strings*/
	if(strlen(ln1) > strlen(ln2)){
		n = strlen(ln2);
	}else{
		n = strlen(ln1);
	}
	
	for(int i = 0; i < n; i++){
		
		if(ln1[i] == '\n' || ln1[i] == '\r' || ln2[i] == '\n' || ln2[i] == '\r' ){
			break;
		}
		
		if(ignoreCase == 0){
			if(ln1[i] != ln2[i]){
				count++;
			}
		}else{
			if((ln1[i] == ln2[i])
			|| ((ln1[i] >= 'A') && (ln1[i] <= 'Z') && (ln2[i] == ln1[i]+32))
			|| ((ln1[i] >= 'a') && (ln1[i] <= 'z') && (ln2[i] == ln1[i]-32))){
				
			}else {
				count++;
			}
		}
			
	
	}
	
	return count;
}


int main(int argc, char *argv[])
{
	myprog = argv[0];
	int opt_i = 0;		/**< How many Times option i was passed*/
	int opt_o = 0;		/**< How many Times option o was passed*/
	char *o_arg = NULL;	/**< The name(Path) of the outputfile*/

	
	int c;
	while ( (c = getopt(argc, argv, "io:")) != -1 ){
		switch ( c ) {
			case 'i': 
				opt_i++;
				break;
			case 'o':
				opt_o++;
				o_arg = optarg;
				break;
			case '?': /* invalid option */
				usage();
				break;
			default:
				assert(0);
		}
	}
	
	if((opt_i > 1) || (opt_o > 1) || ((argc - opt_i - (opt_o*2) - 1) != 2)){
		usage();
	}
	
	FILE *in1;			/**< File 1 of the Comparison*/
	FILE *in2;			/**< File 2 of the Comparison*/
	FILE *out = NULL;	/**< (Optional) Output File*/
	
	in1 = fopen(argv[argc-2], "r");
	if(in1 == NULL){
		fprintf(stderr, "%s: Input File 1 can not be opened!", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	in2 = fopen(argv[argc-1], "r");
	if(in2 == NULL){
		fprintf(stderr, "%s: Input File 2 can not be opened!", argv[0]);
		fclose(in1);
		in1 = NULL;
		exit(EXIT_FAILURE);
	}
	
	if(opt_o == 1){
		out = fopen(o_arg,"w");
		if(out == NULL){
			fprintf(stderr, "%s: Output File can not be opened!", argv[0]);
			fclose(in1);
			in1 = NULL;
			fclose(in2);
			in2 = NULL;
			exit(EXIT_FAILURE);
		}
	}
	
	//Get MaxLenght of Rows & No of Rows in File
	int file1Meta[2];	/**< Metadata of File 1*/
	int file2Meta[2];	/**< Metadata of File 2*/
	if(getFileMetaData(in1,file1Meta)!=0){
		fprintf(stderr, "%s: Error while reading File 1", argv[0]);
		fclose(in1);
		in1 = NULL;
		fclose(in2);
		in2 = NULL;
		fclose(out);
		out = NULL;
		exit(EXIT_FAILURE);
	}
	if(getFileMetaData(in2,file2Meta)!=0){
		fprintf(stderr, "%s: Error while reading File 2", argv[0]);
		fclose(in1);
		in1 = NULL;
		fclose(in2);
		in2 = NULL;
		fclose(out);
		out = NULL;
		exit(EXIT_FAILURE);
	}
	
	
	//Set String Length to Maxlength of Respective Files
	char *line1 = (char *) malloc(file1Meta[0] * sizeof (char)); /**< Line from File 1*/
	char *line2 = (char *) malloc(file2Meta[0] * sizeof (char)); /**< Line from File 2*/
	
	//Find out No of Rows of smallest File
	int noOfRows;	/**< No of rows in the smaller File*/
	if(file1Meta[1] > file2Meta[1]){
		noOfRows = file2Meta[1];
	}else{
		noOfRows = file1Meta[1];
	}
		
	//Iterate through both files row by row and compare	
	for(int i = 0; i < noOfRows; i++){
		
		if(fgets(line1,file1Meta[0],in1)==NULL){
			fprintf(stderr, "%s: Error while reading Line 1", argv[0]);
			fclose(in1);
			in1 = NULL;
			fclose(in2);
			in2 = NULL;
			fclose(out);
			out = NULL;
			free(line1);
			line1 = NULL;
			free(line2);
			line2 = NULL;
			exit(EXIT_FAILURE);
		}
		if(fgets(line2,file2Meta[0],in2)==NULL){
			fprintf(stderr, "%s: Error while reading Line 2", argv[0]);
			fclose(in1);
			in1 = NULL;
			fclose(in2);
			in2 = NULL;
			fclose(out);
			out = NULL;
			free(line1);
			line1 = NULL;
			free(line2);
			line2 = NULL;
			exit(EXIT_FAILURE);
		}

		int res;	/**< Results of the current Row*/
		
		res = getNrOfDifferentChars(line1,line2,opt_i);	
		if(res != 0){
				
			if(opt_o == 0){
				printf("Line: %d, characters: %d\n", i+1, res);
			}else{
				fprintf(out,"Line: %d, characters: %d\n", i+1, res);
			}
		}
	
	}
	
	free(line1);
	line1 = NULL;
	free(line2);
	line2 = NULL;
	fclose(in1);
	in1 = NULL;
	fclose(in2);
	in2 = NULL;
	if(opt_o == 1){
		fclose(out);	
		out = NULL;
	}
	
		
	exit(EXIT_SUCCESS);
}