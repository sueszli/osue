/**
 * @file forkFFT.c
 * @author Alexander Stummer 11777763
 * @date 11.12.2021
 *
 * @brief this module uses the Cooley-Tukey Algorithm to perform 
 * Fast Fourier Transformations
 * 
 * This program takes no arguments. Instead it reads from stdin until EOF
 * is reached. It takes Floats of n^2 size with at minimum 1 value.
 * With those it recursively calls itself until there is one instance for
 * each value received. Then recombined by the algorithm until there are 
 * again values equal to the amount received at the start. The end result
 * is printed to stdout in the, with the imaginary part noted with an 'i'.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
/**
 * This function is the entire program
 * @brief	performs the Fast Fourier Transformation
 * @details	no global variables
 * @param	argc The argument counter
 * @param	argv The argument vector
 * @return	returns EXIT_SUCCESS on success and EXIT_FAILURE for any errors
 */
int main(int argc, char** argv){
	
	
	/**
	 *	################################
	 *
	 *	PROGRAM START
	 *
	 *	################################
	 */
	
	int c;
	while((c=getopt(argc,argv,"")) != -1){
		
		switch(c){
			default:
				fprintf(stderr, "[%s] error: this program takes no arguments\n", argv[0]);
				exit(EXIT_FAILURE);
				break;
		}
	}
	
	//init arrays
	//original length of arrays. size gets increased if input is larger than current array length
	int arrlen	= 4;
	//container for even values
	float* even;
	even	= (float*) malloc(arrlen * sizeof(float));
	//container for odd values
	float* odd;
	odd	= (float*) malloc(arrlen * sizeof(float));
	
	//check for malloc error
	if(even == NULL || odd == NULL){
		fprintf(stderr,"[%s] malloc error\n", argv[0]);
		free(even);
		free(odd);
		exit(EXIT_FAILURE);
	}
	
	//counter for stdin values
	int insize = 0;
	//ends while loop, continues at 0, ends at 1
	int lineflag = 0;
	
	//inits for getline
	char* line_buf		= NULL;
	size_t line_buf_size	= 0;
	ssize_t line_size	= 0;
	
	
	/**
	 *	################################
	 *	END OF INITIALIZATION
	 *	
	 *	START OF RPOCESSING INPUT
	 *	################################
	 */
		
	
	while(lineflag == 0){
		//init for getline
		line_buf 	= NULL;
		line_buf_size	= 0;
		line_size = getline(&line_buf, &line_buf_size, stdin);
		
		//true if input read
		if(line_size >= 0){
			//true if arrays too short
			if(insize >= arrlen*2){
				even = (float*) realloc(even, arrlen*2 * sizeof(float));
				odd = (float*) realloc(odd, arrlen*2 * sizeof(float));
				if(even == NULL || odd == NULL){
					fprintf(stderr, "[%s] realloc error\n",argv[0]);
				}
				arrlen = arrlen*2;
			}
			//true if value is stored in even
			if((insize%2) == 0){
				even[insize/2] = strtof(line_buf, NULL);
			}
			else{
				odd[(insize-1)/2] = strtof(line_buf, NULL);
			}
			//check for strtof error
			if(errno == ERANGE){
				fprintf(stderr, "[%s] strtof error\n%s\n", argv[0], strerror(errno));
				free(even);
				free(odd);
				exit(EXIT_FAILURE);
			}
			
			insize++;
			
		}
	
		else{
			//ends loop
			lineflag = 1;
			
			if(!feof(stdin)){
				fprintf(stderr, "[%s] getline error\n%s\n", argv[0], strerror(errno));
				free(even);
				free(odd);
				exit(EXIT_FAILURE);
			}
		}
	}
	
	/**
	 *	################################
	 *	END OF PROCESSING INPUT
	 *	
	 *	START OF SPECIAL CASES
	 *	################################
	 */
	
	//catches not enough arguments
	if(insize < 1){
		fprintf(stderr,"[%s] error: at least one input needed\n",argv[0]);
		free(even);
		free(odd);
		exit(EXIT_FAILURE);
	}
	//ends process if input is one number
	if(insize == 1){
		//fprintf(stderr,"bottom: %f\n",even[0]);
		printf("%f 0.0i\n", even[0]);
		free(even);
		free(odd);
		exit(EXIT_SUCCESS);
	}
	//catches invalid input amounts
	else if(insize%2 == 1){
		fprintf(stderr, "[%s] error: input amount not valid!\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	
	/**
	 *	################################
	 *	END OF SPECIAL CASES
	 *	
	 *	START OF FORKING
	 *	################################
	 */
	
	//program has a positive even amount of inputs
	else{
		
		//create pipes
		int* pipes[2][2];
		
		//pipes for even
		int evenPipes1[2];
		int evenPipes2[2];
		pipe(evenPipes1);
		pipe(evenPipes2);
		
		pipes[0][0] = evenPipes1;
		pipes[0][1] = evenPipes2;
		
		//pipes for odd
		int oddPipes1[2];
		int oddPipes2[2];
		pipe(oddPipes1);
		pipe(oddPipes2);
		
		pipes[1][0] = oddPipes1;
		pipes[1][1] = oddPipes2;
		
		for(int i=0; i<2; i++){
			
			//fork
			pid_t pid = fork();
			//split further program into parent and children
			switch(pid){
				//true if fork error
				case -1:
					fprintf(stderr, "[%s] fork error\n%s\n",argv[0],strerror(errno));
					free(even);
					free(odd);
					exit(EXIT_FAILURE);
					//just in case
					break;
				
				//true if child
				case 0:
					//pipe can read stdin
					close(pipes[i][0][1]);
					if(dup2(pipes[i][0][0], STDIN_FILENO)== -1){
						fprintf(stderr, "[%s] dup2 error1\n%s\n",argv[0],strerror(errno));
						free(even);
						free(odd);
						exit(EXIT_FAILURE);
					}
					//pipe can write stdout
					close(pipes[i][1][0]);
					if(dup2(pipes[i][1][1], STDOUT_FILENO)== -1){
						fprintf(stderr, "[%s] dup2 error2\n%s\n",argv[0],strerror(errno));
							free(even);
						free(odd);
						exit(EXIT_FAILURE);
					}
					
					if(execlp("./forkFFT","./forkFFT", NULL) == -1){
						fprintf(stderr,"[%s] forkFFT error\n%s\n",argv[0],strerror(errno));
						free(even);
						free(odd);
						exit(EXIT_FAILURE);
					}
					
					break;
				
				//true if parent
				default:
				
					close(pipes[i][0][0]);
					close(pipes[i][1][1]);
					//write to child
					if(i==0){
						for(int j=0; j<(insize/2); j++){
							int len = snprintf(NULL, 0, "%f", even[j]);
							char *str = (char*) malloc(len+1);
							snprintf(str, len+1, "%f", even[j]);
							write(pipes[i][0][1], str, len+1);
							write(pipes[i][0][1], "\n",1);
							
							free(str);
						}
					}
					if(i==1){
						for(int j=0; j<(insize/2); j++){
							int len = snprintf(NULL, 0, "%f", odd[j]);
							char *str = (char*) malloc(len+1);
							snprintf(str, len+1, "%f", odd[j]);
							write(pipes[i][0][1], str, len+1);
							write(pipes[i][0][1], "\n",1);
							
							free(str);
						}
					}
					close(pipes[i][0][1]);
					break;
			//end of switch
			//end of loop	
			}
			
		//end of loop
		}
		
		/**
		 *	################################
		 *	END OF FORKING
		 *	
		 *	START OF READING CHILD OUTPUT
		 *	################################
		 */
		
		//stores values from wait
		pid_t childid;
		int status;
		
		/**
		 * arrays store returned values as
		 * 	a	b*i
		 *	c	d*i
		 *	e	f*i
		 *	g	h*i
		 *
		 * etc. (example with insize = 8)
		 */
		
		//stores child output
		float REO[2][insize/2][2];
		
		//for strtof
		char* ptr;
		
		//wait for children
		for(int i=0; i<2; i++){
			childid = wait(&status);
			if(childid == -1){
				fprintf(stderr,"[%s] error in wait for a child\n",argv[0]);
				exit(EXIT_FAILURE);
			}
			if(status == -1){
				fprintf(stderr,"[%s] child error\n",argv[0]);
				exit(EXIT_FAILURE);
			}
			//get length to read
			int len = (snprintf(NULL, 0, "%f %fi\n", even[0],even[0])+2)*insize/2;
			//stores output as string
			char* temp1 = (char*) malloc(len);
			if(temp1 == NULL){
				fprintf(stderr,"[%s] malloc error for temp1\n",argv[0]);
				free(even);
				free(odd);
				exit(EXIT_FAILURE);
			}
			//for even
			if(i==0){
				//read from child
				read(pipes[0][1][0], temp1, len);
				ptr = temp1;
				//process all lines
				for(int j=0; j<(insize/2); j++){
					
					while(ptr[0] == '\n' || ptr[0] == 'i' || ptr[0] == ' '){
					ptr = ptr+1;
					}
					REO[0][j][0] = strtof(ptr, &ptr);
					
					while(ptr[0] == '\n' || ptr[0] == 'i' || ptr[0] == ' '){
						ptr = ptr+1;
					}
					
					REO[0][j][1] = strtof(ptr, &ptr);
				}
				
			}
			
			free(temp1);
			
			char* temp2 = (char*) malloc(len);
			if(temp2 == NULL){
				fprintf(stderr,"[%s] malloc error for temp2\n",argv[0]);
				free(even);
				free(odd);
				exit(EXIT_FAILURE);
			}
			//for odd
			if(i==1){
				//read from child
				read(pipes[1][1][0], temp2, len);
				ptr = temp2;
				//process all lines
				for(int j=0; j<(insize/2); j++){
					while(ptr[0] == '\n' || ptr[0] == 'i' || ptr[0] == ' '){
					ptr = ptr+1;
					}
					REO[1][j][0] = strtof(ptr, &ptr);
					
					while(ptr[0] == '\n' || ptr[0] == 'i' || ptr[0] == ' '){
						ptr = ptr+1;
					}
					REO[1][j][1] = strtof(ptr, &ptr);
				}
				
			}
			
			free(temp2);
		
		}
		//close unused pipes
		close(pipes[0][1][0]);
		close(pipes[1][1][0]);
		
		/**
		 *	################################
		 *	END OF READING CHILD OUTPUT
		 *	
		 *	START OF PROCESSING INPUT
		 *	################################
		 */
		
		//as described in Assignment
		double pi = 3.141592654;
		int n = insize;
		
		//create values for 0 - n/2
		for(int k=0; k<n/2; k++){
			float real = 0.0;
			float imag = 0.0;
			
			//for inside cos/sin
			float cs = ((-((2*pi)/n))*k);
			
			real += REO[0][k][0];
			imag += REO[0][k][1];
			
			//+a*c
			real += cosf(cs) * REO[1][k][0];
			//-b*d
			real -= sinf(cs) * REO[1][k][1];
			
			//+a*d
			imag += cosf(cs) * REO[1][k][1];
			//+b*d
			imag += sinf(cs) * REO[1][k][0];
			
			printf("%f %fi\n",real,imag);
		}
		
		//create values for n/2 - n
		for(int k=0; k<n/2; k++){
			
			float real = 0.0;
			float imag = 0.0;
			
			//for inside cos/sin
			float cs = ((-((2*pi)/n))*k);
			
			real += REO[0][k][0];
			imag += REO[0][k][1];
			
			//-a*c
			real -= cosf(cs) * REO[1][k][0];
			//-(-b*d)
			real += sinf(cs) * REO[1][k][1];
			
			//-a*d
			imag -= cosf(cs) * REO[1][k][1];
			//-b*d
			imag -= sinf(cs) * REO[1][k][0];
			
			printf("%f %fi\n",real,imag);
		}
		
		
	}	//end of else
	free(even);
	free(odd);
	exit(EXIT_SUCCESS);
}
