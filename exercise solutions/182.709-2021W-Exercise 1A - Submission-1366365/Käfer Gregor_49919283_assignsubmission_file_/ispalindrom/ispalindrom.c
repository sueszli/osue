#include "ispalindrom.h"

/**
 take a string of text and reverse it
 @param text text that want to be reversed
**/
void reverseText(char *text) {
	
	char temp[BUFFER];
	strcpy(temp, text);
	int count = 0;
	for(int i = strlen(text)-1; i>=0; i--){
		text[count] = temp[i];
		count++;
	}
}

/**
 take a string of text and check if its a palindrom or not
 @param text text that want to be check
 @param iFlag if user give -i argument or not
 @param sFlag if user give -s argument or not
**/
void isPalindrom(char *text, int iFlag, int sFlag) {
	char temp[BUFFER];
	char temp2[BUFFER];
	char textTemp[BUFFER];
	char* tempRemove = malloc(sizeof(char) * BUFFER);
	
	const char* palindrom = " is a palindrom";
	const char* notpalindrom = " is not a palindrom";
	
	if(text[strlen(text)-1] == '\n') {
		text[strlen(text)-1] = 0;
	}
	strcpy(textTemp, text);
	/** if iFlag given then change all character in the text into lower case **/
	if(iFlag) {
		for(int i = 0; i<strlen(text); i++) {
			textTemp[i] = tolower(textTemp[i]);
		}
	}
	
	/** if sFlag given then remove whitespace from text **/
	if(sFlag) {
		int count = 0;
		for(int i = 0; i<strlen(textTemp); i++) {
			if(!(text[i] == ' ')){ 
				tempRemove[count] = textTemp[i];
				count++;
			}
		}
		strcpy(temp, tempRemove);
		strcpy(temp2, tempRemove);
	} else {	
		strcpy(temp, textTemp);
		strcpy(temp2, textTemp);
	}
	reverseText(temp2);
	
	/** check if temp and reverse temp are the same or not and add palindrom/notpalindrom to text **/
	if(strcmp(temp,temp2) == 0) {
		free(tempRemove);
		strcat(text, palindrom);
	}
	else {
		free(tempRemove);
		strcat(text, notpalindrom);
	}
	
}

/**
 read user input from stdin and print into stdout/output file
 @param iFlag if user give -i argument or not
 @param sFlag if user give -s argument or not
 @param oFlag if user give -o argument or not
 @param outputFile output file name if oFlag is given
**/
void readFromStdin(int iFlag, int sFlag, int oFlag, char *outputFile) {
	char temp[BUFFER];
	int count = 0;
	while(fgets(temp, BUFFER, stdin)) {
		isPalindrom(temp, iFlag, sFlag);
		if(!oFlag) fprintf(stdout, "%s\n", temp);
		
		/** if oFlag is given then print it into output file **/
		else {
			FILE *fp;
			if(count == 0) {
				fp = fopen(outputFile, "w");
				fprintf(fp, "%s\n", temp);
				count++;
				fclose(fp);
			} else {
				fp = fopen(outputFile, "a");
				fprintf(fp, "%s\n", temp);
				count++;
				fclose(fp);
			}
		}	
	
	}
}

/**
 same as readFromStdin but read it from a file instead of stdin
 @param iFlag if user give -i argument or not
 @param sFlag if user give -s argument or not
 @param oFlag if user give -o argument or not
 @param outputFile output file name if oFlag is given
 @param inputFile input file name
 @param begin if its the first file or not
**/ 
void readFromFile(int iFlag, int sFlag, int oFlag, char* outputFile, char* inputFile, int begin) {
	
	char temp[BUFFER];
	int count = 0;
	FILE *fp;
	fp = fopen(inputFile, "r");
	while(fgets(temp, BUFFER, fp) != NULL) {
		temp[strcspn(temp, "\r\n")] = 0;
		isPalindrom(temp, iFlag, sFlag);
		if(!oFlag) fprintf(stdout, "%s\n", temp);
		else {
			FILE *fpOut;
			if(count == 0 && begin == 0) {
				fpOut = fopen(outputFile, "w");
				fprintf(fpOut, "%s\n", temp);
				count++;
				fclose(fpOut);
			} else {
				fpOut = fopen(outputFile, "a");
				fprintf(fpOut, "%s\n", temp);
				count++;
				fclose(fpOut);
			}
		}
	}
	fclose(fp);
}

/**
 simple usage function that print usage error message and exit the program with EXIT_FAILURE
**/
void usage(void){
	char *progName = "ispalindrom";
	fprintf(stderr,"Usage: %s [-s] [-i] [-o outfile] [file..] \n",progName);
	exit(EXIT_FAILURE);
}

int main(int argc,char *argv[]){
	
	int iFlag = 0;
	int oFlag = 0;
	int sFlag = 0;
	int i;
	char* outputFile;
	
	while((i=getopt(argc,argv,"sio:"))!= -1){
		switch(i){
			case 'i' :
			    if(iFlag){
                    usage();
			    }
			    else{
                    iFlag++;
			    }
                break;
			case 'o' :
			    if(oFlag){
                    usage();
			    }
			    else{
                    oFlag++;
					outputFile = strdup(optarg); 
			    }
                break;
			case 's' :
				if(sFlag) {
					usage();
				}
				else {
					sFlag++;
				}
				break;
			case '?' : usage();
				break;
			default : assert(0);
		}
	}	
	

	if((argc - iFlag - sFlag == 1) || (oFlag && (argc - iFlag - sFlag - oFlag == 2))) {
		readFromStdin(iFlag, sFlag, oFlag, outputFile);
	} else {
		int count = 0;
		if(iFlag) count++;
		if(sFlag) count++;
		if(oFlag) count = count +2;
		int totalFile = argc - count - 1;
		for(int i = 0; i<totalFile; i++) {
			readFromFile(iFlag, sFlag, oFlag, outputFile, argv[argc-1-i],i);
		}
	}
	free(outputFile);
	
	exit(EXIT_SUCCESS);
}