#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

/**\file forksort
 * @author Peter Nyikos
 * @brief This program implements a recursive variant of mergesort.
 * @details it does this by splitting itself up via fork() and then re-executung itself,
 * after having set up some pipes.
 * @date 11/12/2021
 */
 
 char *myprog;					/**< String name of the program*/		
 
 /**
 * @brief This function is used in case the program is passed wrong parameters.
 * @details It prints the correct usage and exits with failure.
 * The global variable *myprog is used to print the program name.
 */
static void usage(void){
	fprintf(stderr,"Usage: %s\n", myprog);
	exit(EXIT_FAILURE);	
}

 
 int main(int argc, char *argv[])
{
	myprog = argv[0];
	
	if(argc != 1){
		usage();
	}
	
	int pipefdOut1[2];		/**< Pipe for outputting to child1*/
	int pipefdOut2[2];		/**< Pipe for outputting to child2*/
	int pipefdIn1[2];		/**< Pipe for reading from child1*/
	int pipefdIn2[2];		/**< Pipe for reading from child2*/
	char *line1 = NULL;		/**< line for sending/recieving from child1*/
	char *line2 = NULL;		/**< line for sending/recieving from child2*/
	size_t len1 = 0;		/**< length of line1*/
	size_t len2 = 0;		/**< length of line2*/
	ssize_t nread1 = 0;		/**< nr of chars in line1*/
	ssize_t nread2 = 0;		/**< nr of chars in line2*/
	int shouldExit = 0;		/**< exitcondition of loop*/


	//check if at least one line is in stdin
	if((nread1 = getline(&line1, &len1, stdin)) == -1){
		fprintf(stderr,"%s: Please input atleast 1 line\n", myprog);
		exit(EXIT_FAILURE);
	}

	//If only one line is in stdin, we do not need to fork, and can return immediatley.
	if((nread2 = getline(&line2, &len2, stdin)) == -1){
			write(STDOUT_FILENO,line1,nread1);
			free(line1);
			free(line2);
			exit(EXIT_SUCCESS);
	}
	
	//create pipes for child 1
	if (pipe(pipefdOut1) == -1){
		fprintf(stderr,"%s: Failed while creating outpipe1\n", myprog);
	   exit(EXIT_FAILURE);
    }
	if (pipe(pipefdIn1) == -1){
		fprintf(stderr,"%s: Failed while creating inpipe1\n", myprog);
	   exit(EXIT_FAILURE);
    }
	
	//fork child 1
	pid_t c1id = fork();
	if(c1id == -1){
		exit(EXIT_FAILURE);
	}
	if(c1id == 0){
			//re-wire the pipes of the child
			if(dup2(pipefdOut1[0],STDIN_FILENO) == -1){
				fprintf(stderr,"%s: Failed while reassingning inpipe1\n", myprog);
				exit(EXIT_FAILURE);
			}
			close(pipefdOut1[0]);
			close(pipefdOut1[1]);
			
			if(dup2(pipefdIn1[1],STDOUT_FILENO) == -1){
				fprintf(stderr,"%s: Failed while reassingning outpipe1\n", myprog);
				exit(EXIT_FAILURE);
			}
			close(pipefdIn1[1]);
			close(pipefdIn1[0]);
			//recursivley execute the program
			execl(myprog,myprog,NULL);		
	}
	close(pipefdOut1[0]);
	close(pipefdIn1[1]);
	
	//create pipes for child 2
	if (pipe(pipefdOut2) == -1){
		fprintf(stderr,"%s: Failed while creating outpipe 2\n", myprog);
	   exit(EXIT_FAILURE);
    }
	if (pipe(pipefdIn2) == -1){
		fprintf(stderr,"%s: Failed while creating inpipe 2\n", myprog);
	   exit(EXIT_FAILURE);
    }

	//fork child 2
	pid_t c2id = fork();
	if(c2id == -1){
		exit(EXIT_FAILURE);
	}
	if(c2id == 0){
			//re-wire the pipes of the child
			if(dup2(pipefdOut2[0],STDIN_FILENO) == -1){
				fprintf(stderr,"%s: Failed while reassingning inpipe2\n", myprog);
				exit(EXIT_FAILURE);
			}
			close(pipefdOut2[0]);
			close(pipefdOut2[1]);
			
			if(dup2(pipefdIn2[1],STDOUT_FILENO) == -1){
				fprintf(stderr,"%s: Failed while reassingning outpipe2\n", myprog);
				exit(EXIT_FAILURE);
			}
			close(pipefdIn2[1]);
			close(pipefdIn2[0]);
			//recursivley execute the program
			execl(myprog,myprog,NULL);		
	}
	close(pipefdOut2[0]);
	close(pipefdIn2[1]);
	
	//send the first two lined to the children via the pipes
	write(pipefdOut1[1],line1,nread1);
	write(pipefdOut2[1],line2,nread2);
	
	//read lines and send them via the pipes until EOF is detected.
	while(1==1) {
		if((nread1 = getline(&line1, &len1, stdin)) == -1){
			break;
		}
		
		if((nread2 = getline(&line2, &len2, stdin)) == -1){
			write(pipefdOut1[1],line1,nread1);
			break;
		}	
		
		write(pipefdOut1[1],line1,nread1);
		write(pipefdOut2[1],line2,nread2);	   
	}
	
	//close sending-pipes, to simulate EOF for the children
	close(pipefdOut1[1]);
	close(pipefdOut2[1]);
		
	int status;
	
	//wait for the children to exit
	if(waitpid(c1id,&status,WUNTRACED | WCONTINUED) == -1){
		fprintf(stderr,"%s: Failed while waiting for child1\n", myprog);
		exit(EXIT_FAILURE);
	}
	if(status == EXIT_FAILURE){
		fprintf(stderr,"%s: A child has failed\n", myprog);
		exit(EXIT_FAILURE);
	}
	if(waitpid(c2id,&status,WUNTRACED | WCONTINUED) == -1){
		fprintf(stderr,"%s: Failed while waiting for child2\n", myprog);
		exit(EXIT_FAILURE);
	}
	if(status == EXIT_FAILURE){
		fprintf(stderr,"%s: A child has failed\n", myprog);
		exit(EXIT_FAILURE);
	}

	//create streams from the FD of the pipes
	FILE *child1;
	if((child1 = fdopen(pipefdIn1[0],"r")) == NULL){
		fprintf(stderr,"%s: Failed while opening file1\n", myprog);
		exit(EXIT_FAILURE);
	} 
	FILE *child2;
	if((child2 = fdopen(pipefdIn2[0],"r")) == NULL){
		fprintf(stderr,"%s: Failed while opening file2\n", myprog);
		exit(EXIT_FAILURE);
	} 

	//read the first two lines (if there is not at least one line per stream, something has gone wrong)
	if((nread1 = getline(&line1, &len1, child1)) == -1){
		fprintf(stderr,"%s: Failed while getting line1\n", myprog);
		exit(EXIT_FAILURE);
	}
	if((nread2 = getline(&line2, &len2, child2)) == -1){
		fprintf(stderr,"%s: Failed while getting line2\n", myprog);
		exit(EXIT_FAILURE);
	}

	//mergesort:
	shouldExit = 0;
	int getnewline = 0;
	while (shouldExit == 0) {
		getnewline = 0;

		//write down the line, that comes earlier in the alphabet
		if(strcmp(line1,line2) <= 0){
			fwrite(line1, nread1, 1, stdout);
			getnewline = 1;
		}else{
			fwrite(line2, nread2, 1, stdout);
			getnewline = 2;
		}
		
		//get a new line from the stream where it was taken from
		if(getnewline == 1){
			if((nread1 = getline(&line1, &len1, child1)) == -1){
				fwrite(line2, nread2, 1, stdout);
				while((nread2 = getline(&line2, &len2, child2)) != -1){
					fwrite(line2, nread2, 1, stdout);
				}
				shouldExit = 1;
			}
		}else if(getnewline == 2){
			if((nread2 = getline(&line2, &len2, child2)) == -1){
				fwrite(line1, nread1, 1, stdout);
				while((nread1 = getline(&line1, &len1, child1)) != -1){
					fwrite(line1, nread1, 1, stdout);
				}
				shouldExit = 1;
			}
		}else{
			fprintf(stderr,"%s: Failed with invalid state\n", myprog);
			exit(EXIT_FAILURE);
		}	  
	}

	//free resources
	fclose(child1);
	fclose(child2);
	free(line1);
	free(line2);
	close(pipefdIn1[1]);
	close(pipefdIn2[1]);
	close(pipefdOut1[0]);
	close(pipefdOut2[0]);
	close(pipefdIn1[0]);
	close(pipefdIn2[0]);
	
	
	exit(EXIT_SUCCESS);
}








 