/*
@autor: Jessl Lukas 01604985
@modulename: forkFFT.c
@created 20.11.2021

@brief: This programm takes an input, splits it into an odd and even part. With those two parts (they need to have the same length)
we will use a fast fourier transformation. We programm the fast fourier transformation recursivley after the "Cooley-Tukey" Algorithm.*/

#include "forkFFT.h"

/*	@param argc: number of arguments given to this programm
	@param argv: array of the arguments
	
	@brief: Splits the input into even and odd parts, where even contains every input with rows % 2 = 0 and odd with rows %2 = 1.
	If needed (length > 1) calls fork to split the progress into smaller parts. Writes the output to stdout.
*/

int main(int argc, char *argv[]){
	
	char* eveninput = malloc(sizeof(char));
	if(eveninput == NULL){
		fprintf(stderr, "Error in File: %s, could not allocate memory for eveninput %s\n",argv[0], strerror(errno));
		free (eveninput);
		exit(EXIT_FAILURE);
	}
	
	char* oddinput = malloc(sizeof(char));
	if(oddinput == NULL){
		fprintf(stderr, "Error in File: %s, could not allocate memory for oddinput %s\n",argv[0], strerror(errno));
		closeallusedMemory(eveninput,oddinput);
		exit(EXIT_FAILURE);
	}
	
	char* input = readfromCommandoLine(argv);

	int indexodd = 0, indexeven = 0;
	
	char* evenodd = strtok(input, "\n");
	while(evenodd != NULL){
		if(indexeven != indexodd){
			oddinput = realloc(oddinput, (strlen(oddinput) + strlen(evenodd) + 1) * sizeof(char));
			if(oddinput == NULL){
				fprintf(stderr, "Error in File: %s, could not reallocate memory for odd %s\n",argv[0], strerror(errno));
				free(input);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}	
			strcat(oddinput,evenodd);
			strcat(oddinput, "\n");
			indexodd++;
		} else{
			eveninput = realloc(eveninput, (strlen(eveninput) + strlen(evenodd) + 1) * sizeof(char));
			if(eveninput == NULL){
				fprintf(stderr, "Error in File: %s, could not reallocate memory for even %s\n",argv[0], strerror(errno));
				free(input);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			strcat(eveninput,evenodd);
			strcat(eveninput, "\n");
			indexeven++;
		}
		evenodd = strtok(NULL, "\n");
	}

	free(input);

	eveninput[strlen(eveninput)-1] = '?';
	oddinput[strlen(oddinput)-1] = '?';  
	
	if(indexeven != indexodd){
		fprintf(stderr, "Error in File: forkFFT.c, input does not have an even length\n");
		closeallusedMemory(eveninput,oddinput);
		exit (EXIT_FAILURE);
	}		
			
	int length = indexeven;



	if(length == 1){
		
		float even;
		float odd;

		even = strtof(eveninput,NULL);
		odd = strtof(oddinput,NULL);	

		closeallusedMemory(eveninput,oddinput);
		
		printf("%.1f %.1f*i\n",(even + odd), 0.0);
		printf("%.1f %.1f*i\n",(even - odd), 0.0);
		
		return EXIT_SUCCESS;
	
	} else{
		
		int pipefd1[2];
		
		if(pipe(pipefd1) == -1){
			fprintf(stderr, "Error in File: %s, could not create pipe for child 1 %s\n",argv[0], strerror(errno));
			closePipes2(pipefd1[0],pipefd1[1]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}

		int pipefd2[2];
		
		if(pipe(pipefd2) == -1){
			fprintf(stderr, "Error in File: %s, could not create pipe 2 for child 1 %s\n",argv[0], strerror(errno));
			closePipes2(pipefd1[0],pipefd1[1]);
			closePipes2(pipefd2[0],pipefd2[1]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
		

		write(pipefd1[1], eveninput, strlen(eveninput));
		
		pid_t pid1 = fork();
		if(pid1 == -1){
			fprintf(stderr, "Error in File: %s, could not create fork for child 1 %s\n",argv[0], strerror(errno));
			closePipes2(pipefd1[0],pipefd1[1]);
			closePipes2(pipefd2[0],pipefd2[1]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}else if(pid1 == 0){
			if(close(pipefd1[1])==-1){
				fprintf(stderr, "Error in File: %s, could not close pipefd1 write end in child 1%s\n",argv[0], strerror(errno));
				closePipes3(pipefd1[0], pipefd2[0], pipefd2[1]);
				closeallusedMemory(eveninput, oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd2[0])== -1){
				fprintf(stderr, "Error in File: %s, could not close pipefd2 read end in child 1 %s\n",argv[0], strerror(errno));
				closePipes2(pipefd1[0], pipefd2[1]);
				closeallusedMemory(eveninput, oddinput);
				exit(EXIT_FAILURE);
			}
			
			dup2(pipefd1[0], STDIN_FILENO);
			dup2(pipefd2[1], STDOUT_FILENO);	
			
			if(close(pipefd1[0])== -1){
				fprintf(stderr, "Error in File: %s, could not close pipefd1 write end in child 1%s\n",argv[0], strerror(errno));
				close(pipefd2[1]);
				closeallusedMemory(eveninput, oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd2[1])== -1){
				fprintf(stderr, "Error in File: %s, could not close pipefd2 write end in child 1%s\n",argv[0], strerror(errno));
				closeallusedMemory(eveninput, oddinput);
				exit(EXIT_FAILURE);
			}
			
			execlp(argv[0],argv[0], NULL);
			
			//Should not reach this line, else error	
			fprintf(stderr, "Error in File: %s, could not start child 1 recursively %s\n",argv[0], strerror(errno));
			closeallusedMemory(eveninput, oddinput);
			exit(EXIT_FAILURE);	
		} 
			
		int pipefd3[2];
		
		if(pipe(pipefd3) == -1){
			fprintf(stderr, "Error in File: %s, could not create pipe for child 2 %s\n",argv[0], strerror(errno));
			closePipes3(pipefd1[0], pipefd1[1], pipefd2[0]);
			closePipes3(pipefd2[1], pipefd3[0], pipefd3[1]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
			
		int pipefd4[2];
		if(pipe(pipefd4) == -1){
			fprintf(stderr, "Error in File: %s, could not create pipe 2 for child 2 %s\n",argv[0], strerror(errno));
			closePipes3(pipefd1[0], pipefd1[1], pipefd2[0]);
			closePipes3(pipefd2[1], pipefd3[0], pipefd3[1]);
			closePipes2(pipefd4[0], pipefd4[1]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
		
		write(pipefd3[1], oddinput, strlen(oddinput));
			
		pid_t pid2 = fork();
		if(pid2 == -1){
			fprintf(stderr, "Error in File: %s, could not create fork for child 2 %s\n",argv[0], strerror(errno));
			closePipes3(pipefd1[0], pipefd1[1], pipefd2[0]);
			closePipes3(pipefd2[1], pipefd3[0], pipefd3[1]);
			closePipes2(pipefd4[0], pipefd4[1]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}else if(pid2 == 0){
			if(close(pipefd1[0]) == -1){
				fprintf(stderr, "Error in File: %s, could not close read pipe fd1 in child 2 %s\n",argv[0], strerror(errno));
				closePipes3(pipefd1[1], pipefd2[0], pipefd2[1]);
				closePipes2(pipefd3[0], pipefd3[1]);
				closePipes2(pipefd4[0], pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd1[1]) == -1){
			fprintf(stderr, "Error in File: %s, could not close wirte pipe fd1 in child 2 %s\n",argv[0], strerror(errno));
				closePipes3(pipefd2[0], pipefd2[1], pipefd3[0]);
				closePipes3(pipefd3[1], pipefd4[0], pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd2[0]) == -1){
				fprintf(stderr, "Error in File: %s, could not close read pipe fd2 in child 2 %s\n",argv[0], strerror(errno));
				closePipes3(pipefd2[1], pipefd3[0], pipefd3[1]);
				closePipes2(pipefd4[0], pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd2[1]) == -1){
			fprintf(stderr, "Error in File: %s, could not close wite pipe fd1 in child 2 %s\n",argv[0], strerror(errno));
				closePipes2(pipefd3[0], pipefd3[1]);
				closePipes2(pipefd4[0], pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
		
			if(close(pipefd3[1])==-1){
				fprintf(stderr, "Error in File: %s, could not close write pipe fd3 in child 2 %s\n",argv[0], strerror(errno));
				closePipes3(pipefd3[0], pipefd4[0], pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd4[0])==-1){
				fprintf(stderr, "Error in File: %s, could not close read pipe fd4 in child 2 %s\n",argv[0], strerror(errno));
				closePipes2(pipefd3[0], pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			
			dup2(pipefd3[0], STDIN_FILENO);
			dup2(pipefd4[1], STDOUT_FILENO);	
			
			
			if(close(pipefd3[0])==-1){
				fprintf(stderr, "Error in File: %s, could not close read pipe fd3 in child 2 %s\n",argv[0], strerror(errno));
				close(pipefd4[1]);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			if(close(pipefd4[1])==-1){
				fprintf(stderr, "Error in File: %s, could not close write pipe fd4 in child 2 %s\n",argv[0], strerror(errno));
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			
			execlp(argv[0],argv[0], NULL);
			//Should not reach this line, else error	
			fprintf(stderr, "Error in File: %s, could not start child 1 recursively %s\n",argv[0], strerror(errno));
			closeallusedMemory(eveninput, oddinput);
			exit(EXIT_FAILURE);
		}
		
				
		waitpid(pid1,NULL,0);
		waitpid(pid2,NULL,0);
				
		if(close(pipefd1[0])== -1){
			fprintf(stderr, "Error in File: %s, could not close pipefd1 write end in parent %s\n",argv[0], strerror(errno));
			closePipes3(pipefd1[1], pipefd2[0], pipefd2[1]);
			closePipes2(pipefd3[0], pipefd3[1]);
			closePipes2(pipefd4[0], pipefd4[1]);
			closeallusedMemory(eveninput, oddinput);
			exit(EXIT_FAILURE);
		}
		if(close(pipefd2[1])== -1){
			fprintf(stderr, "Error in File: %s, could not close pipefd2 write end in parent %s\n",argv[0], strerror(errno));
			closePipes3(pipefd1[0], pipefd1[1], pipefd2[0]);
			closePipes2(pipefd3[0], pipefd3[1]);
			closePipes2(pipefd4[0], pipefd4[1]);
			closeallusedMemory(eveninput, oddinput);
			exit(EXIT_FAILURE);
		}
		
		if(close(pipefd3[0])==-1){
			fprintf(stderr, "Error in File: %s, could not close write pipe fd3 in parent %s\n",argv[0], strerror(errno));
			closePipes3(pipefd1[1], pipefd2[0], pipefd3[1]);
			closePipes2(pipefd4[0], pipefd4[0]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
		if(close(pipefd4[1])==-1){
			fprintf(stderr, "Error in File: %s, could not close read pipe fd4 in parent %s\n",argv[0], strerror(errno));
			closePipes2(pipefd1[1], pipefd2[0]);
			closePipes2(pipefd3[1], pipefd4[0]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
			
		char* buf= calloc(1,1);
				
		free(eveninput);
		eveninput = malloc(length * (4* sizeof(char) +  2* sizeof(float))); 		//Chars = " " "*" "i" "\n" and the 2 floats
		
		while(read(pipefd2[0], buf, 1)>0){
			strcat(eveninput,buf);
		}

		free(oddinput);
		oddinput = malloc(length * (4* sizeof(char) +  2* sizeof(float))); 		//Chars = " " "*" "i" "\n" and the 2 floats
		
		while(read(pipefd4[0], buf, 1)>0){
			strcat(oddinput,buf);
		}

		free(buf);		
		
		if(close(pipefd1[1])==-1){
			fprintf(stderr, "Error in File: %s, could not close pipefd1 write end in parent %s\n",argv[0], strerror(errno));
			close(pipefd2[0]);
			closePipes2(pipefd3[1], pipefd4[0]);
			closeallusedMemory(eveninput, oddinput);
			exit(EXIT_FAILURE);
		}
		if(close(pipefd2[0])== -1){
			fprintf(stderr, "Error in File: %s, could not close pipefd2 read end in parent  %s\n",argv[0], strerror(errno));
			closePipes2(pipefd3[1], pipefd4[0]);
			closeallusedMemory(eveninput, oddinput);
			exit(EXIT_FAILURE);
		}
		
		if(close(pipefd3[1])==-1){
			fprintf(stderr, "Error in File: %s, could not close read pipe fd3 in parent %s\n",argv[0], strerror(errno));
			close(pipefd4[0]);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
		if(close(pipefd4[0])==-1){
			fprintf(stderr, "Error in File: %s, could not close write pipe fd4 in parent %s\n",argv[0], strerror(errno));
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
				
				
		//Both arrays now have the values in a form of float" "float"*i"\n etc etc				
				
		//creates double arrays
		float** even = malloc (length * sizeof(float) );
		if(even == NULL){
			fprintf(stderr, "Error in File: %s, could not allocate memory for even %s\n",argv[0], strerror(errno));
			free (even);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
		
		float** odd = malloc (length * sizeof(float) * 2);
		if(odd == NULL){
			fprintf(stderr, "Error in File: %s, could not allocate memory for even %s\n",argv[0], strerror(errno));
			free (even);
			free (odd);
			closeallusedMemory(eveninput,oddinput);
			exit(EXIT_FAILURE);
		}
		
		
		//Gives each row the memory for 2 float numbers
		int i = 0;
		for(i = 0; i < length; i++){
			even[i] = malloc(2 * sizeof(float));
		
			if(even[i] == NULL){
				fprintf(stderr, "Error in File: %s, could not allocate memory for even %s\n",argv[0], strerror(errno));
				free (even);
				free (odd);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
			odd[i] = malloc(2 * sizeof(float));
			
			if(odd[i] == NULL){
				fprintf(stderr, "Error in File: %s, could not allocate memory for even %s\n",argv[0], strerror(errno));
				free (even);
				free (odd);
				closeallusedMemory(eveninput,oddinput);
				exit(EXIT_FAILURE);
			}
		}

		inputtofloatcomplex(even,odd,eveninput,oddinput);
	
		ctfft(even, odd, length);
				
		}
	
	
	return EXIT_SUCCESS;
}

/*	@param argv: arguments given to this programm

	@brief: reads everything from the commandline untill EoF. Each line seperated with a "\n".
*/
char* readfromCommandoLine(char** argv){
	
char* line = (char*) calloc (1, sizeof(char));
	char buffer;

	while((buffer = fgetc(stdin)) != EOF) {
		
		if(buffer == '?'){
			break;
		}
		
		char str[2];
        str[1] = '\0';
        str[0] = buffer;
		
		if(ferror(stdin)){
			fprintf(stderr, "Error in File: %s, could not read from stdin %s\n",argv[0], strerror(errno));
			free(line);
			exit(EXIT_FAILURE);
		}
		
		strcat(line, str);
		
		line = realloc(line, sizeof(char) * (strlen(line) + 1));
		if(line == NULL){
			fprintf(stderr, "Error in File: %s, could not reallocate memory for line %s\n",argv[0], strerror(errno));
			free (line);
			exit(EXIT_FAILURE);
		}
	}

	return line;
}

/* 	@param pipefd1/fd2/fd3 : filedescriptor for the pipes

	@brief: closes 3 pipes.
*/
void closePipes3(int pipefd1, int pipefd2, int pipefd3){
	close(pipefd1);
	close(pipefd2);
	close(pipefd3);
}

/* 	@param pipefd1/fd2 : filedescriptor for the pipes

	@brief: closes 2 pipes.
*/

void closePipes2(int pipefd1, int pipefd2){
	close(pipefd1);
	close(pipefd2);
}


/*	@param even : Char pointer
	@param odd : Char pointer

	@brief : frees the allocated memory
*/
void closeallusedMemory(char* even, char* odd){
	free (even);
	free (odd);
}

/* 	@param even: Char array, empty at the start
	@param odd: Char array, empty at the start
	@param eveninput: Contains the input of each row %2 = 0
	@param oddinput: Contains the input of each row %2 = 1
	
	@brief: Seperates real from complex values and writes them to their corresponding position in the array
*/
void inputtofloatcomplex(float** even, float** odd, char* eveninput, char* oddinput){
	int e = 0, o = 0;
	
	int pos = 0;
	float value = 0.0;
	
	char* endpntr ;
	
	char* evennumber = strtok(eveninput, "\n");
	while(evennumber != NULL){
		while(pos != 2){
			value = strtof(evennumber, &endpntr);
			evennumber = endpntr;
			even[e][pos] = value;
			pos ++;
		}
		e++;
		pos = 0;
		evennumber = strtok(NULL,"\n");
	}
	
	char*oddnumber = strtok(oddinput, "\n");
	while(oddnumber != NULL){
		while(pos != 2){
			value = strtof(oddnumber, &endpntr);
			oddnumber = endpntr;
			odd[o][pos] = value;
			pos ++;
		}
		o++;
		pos = 0;
		oddnumber = strtok(NULL,"\n");
	}
}

/* 	@param even = is a double array that contains all even numbers 
	@param odd = is a double array that contains all odd numbers
	@param length = length of the arrays

	@brief This method uses the Cooley-Tukey fast fourier Transformation algorithmus and prints it to stdout
*/
void ctfft(float** even, float** odd, int length){
	
	float xk[2*length][2];
	float pre, pim;
	float qre, qim;
	int m = 0;
	while(m < length){
		pre = even[m][0];
		pim = even[m][1];
		qre = odd[m][0] * cos(-2 * m * PI / (2*length) ) - odd[m][1] * sin(-2 * m * PI / (2*length) );
		qim = odd[m][1] * cos(-2 * m * PI / (2*length) ) + odd[m][0] * sin(-2 * m * PI / (2*length) );
		xk[m][0] = pre + qre;
		xk[m][1] = pim + qim;
		xk[m + length][0] = pre - qre;
		xk[m + length][1] = pim - qim;
		m++;
	}

	for(m = 0; m<2*length; m++){
		printf("%.1f %.1f*i\n", xk[m][0], xk[m][1]);
	}
}






