/**
 * @author Artem Chornyi, 11922295
 * @brief Recursively forks children with calculate FFT algorithm
 * @details every forked child is either 'left' or 'right' child
 *			i.e. child who gets even and odd indexes respectively
 * @date 11.12.21
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <aio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "./rai_number.h"
#include <math.h>

// Defines if errors should be printed to stderr
#define VERBOSE (1)

// custom defined ONERR codes, which define the behaviour
// of error handling
#define ONERR_TERM (1) // on error run exit(EXIT_FAILURE);
#define ONERR_PRINTRSN (2) // on error print its cause (reason)
#define ONERR_LAZYEXIT (4) // set exit flag to 1, to exit after error-handling
						   // function was ran

#define free_and_null(arg) free(arg);arg = NULL
#define fclose_and_null(arg) fclose(arg);arg = NULL



char *filename;
size_t totalInputs = 0;
int lchild_readfd, lchild_writefd, rchild_readfd, rchild_writefd; 
FILE *lchild_fread = NULL, *lchild_fwrite = NULL, *rchild_fread = NULL, *rchild_fwrite = NULL;
int critical_error = 0;


/**
 * @brief Calculates k-th element of output array. 
 *
 * @param k-th output value of left child.
 * @param k-th output value of right child.
 * @param k-th index
 *
 * @return k-th rai_number to output to parent.
 */
static rai_number R_k(rai_number n1, rai_number n2, int k){
	rai_number rai_cossin = {.real=cos(-2*Pi/totalInputs*k), .imag=sin(-2*Pi/totalInputs*k)};
	rai_number R_o_cossin_mult = rai_mult(rai_cossin, n2);
	
	return rai_add(n1, R_o_cossin_mult);
}
/**
 * @brief Calculates (k+n/2)-th element of output array.
 *
 * @param k-th output value of left child.
 * @param k-th output value of right child.
 * @param k-th index
 *
 * @return (k+n/2)-th rai_number to output to parent.
 */
static rai_number R_k_n2(rai_number n1, rai_number n2, int k){
	rai_number rai_cossin = {.real=cos(-2*Pi/totalInputs*k), .imag=sin(-2*Pi/totalInputs*k)};
	rai_number R_o_cossin_mult = rai_mult(rai_cossin, n2);
	
	return rai_sub(n1, R_o_cossin_mult);
}

static void handle_sigterm(int signal){	
	exit(EXIT_FAILURE);
}

/**
 * @brief On SIGTERM signal => execute handle_sigterm 
 */
static void setup_signals(void){
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));

	sa.sa_handler = handle_sigterm;
	sigaction(SIGTERM, &sa, NULL);
}

/**
 * @brief Method which is designed to give info/terminate/lazy-terminate
 * 		  on errors, based on ONERR argument.
 *
 * @details Give info (ONERR_PRINTRSN) -> print reason of an error.
 *			Terminate (ONERR_TERM) -> exit with EXIT_FAILURE status.
 *			Lazy-terminate (ONERR_LAZYEXIT) -> set critical_error value to 1
 *				which indicates that "we should terminate asap".
 *
 * @params Code which specifies how to handle an error.
 * @params Format of a custom message to be printed.
 * @params (...) arguments to format message
 */
static void handle_error(const int ONERR, const char* fmt, ...){
	if(VERBOSE == 1){
		fprintf(stderr, "[%s] Error: ", filename);
		va_list args;

		va_start (args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);

		fprintf(stderr, "\n");
		//fprintf(stderr, "[%s] Error: " fmt, filename);
		if((ONERR & ONERR_PRINTRSN) != 0){
			fprintf(stderr, "Reason: %s\n", strerror(errno));
		}
		fflush(stderr);
	}
	if((ONERR & ONERR_TERM) != 0){
		exit(EXIT_FAILURE);	
	}
	if((ONERR & ONERR_LAZYEXIT) != 0){
		critical_error = 1;
	}

}

/**
 * @brief Terminate the program, if ANY argument
 *        was specified (except for filename).
 *
 * @param number of arguments
 * @param vector of arguments
 */
static void parse_args(int argc, char **argv){
	filename = argv[0];
	if(argc > 1){
		handle_error(0, "Program takes no arguments", filename);
		fprintf(stderr, "Usage: %s\n", filename);
		exit(EXIT_FAILURE);
	}
}

/**
 * @brief frees all global resources
 */
static void free_all(void){
	if(lchild_fread != NULL){
		fclose_and_null(lchild_fread);
	}
	if(lchild_fwrite != NULL){
		fclose_and_null(lchild_fwrite);
	}
	if(rchild_fread != NULL){
		fclose_and_null(rchild_fread);
	}
	if(rchild_fwrite != NULL){
		fclose_and_null(rchild_fwrite);
	}
	close(lchild_readfd);
	close(lchild_writefd);
	close(rchild_readfd);
	close(rchild_writefd);

}

/**
 * @brief Kills both left and right children. (via SIGTERM)
 *
 * @details gets childrens' pids from waitpit(-1)
 */
static void terminate_children(void){
	int wstatus;
	pid_t childPid;
	
	childPid = waitpid(-1, &wstatus, WNOHANG);
	if(!WIFEXITED(wstatus)){
		kill(childPid, SIGTERM);
	}

	childPid = waitpid(-1, &wstatus, WNOHANG);
	if(!WIFEXITED(wstatus)){
		kill(childPid, SIGTERM);
	}
}

static void handle_exit(void){
	terminate_children();
	free_all();
}


static void set_atexit(void){
	if(atexit(handle_exit) != 0){
		handle_error(0, "atexit() failed");
	}
}


/**
 * @brief Sets up pipes, forks left and right children, which exec() this programm a new
 *        but with modified stdin, stdout. Important part are saved to global variables
 *		  i.e. pipe fds and their opened FILE handles.
 *
 * @details for every parent-child communicataion there are 2 pairs of pipes: 
 * 			- .child_pipefds[0] -> the pair, with which child writes to parent
 * 			- .child_pipefds[1] -> the pair, with which parent writes to child
 * 			Why not only 1 pair of pipes per child? -> because this way we can
 * 				trigger EOF to child processes, and keep child being able to write to parent.
 */
static void setup_children_and_pipes(void){
	int lchild_pipefds[2][2];
	if(pipe(lchild_pipefds[0]) != 0){
		handle_error(ONERR_LAZYEXIT | ONERR_PRINTRSN, "Failed to create pipes");
		return;
	}
	if(pipe(lchild_pipefds[1]) != 0){
		close(lchild_pipefds[0][0]);
		close(lchild_pipefds[0][1]);
		handle_error(ONERR_LAZYEXIT | ONERR_PRINTRSN, "Failed to create pipes");
		return;
	}
	
	lchild_readfd = lchild_pipefds[0][0];
	lchild_writefd = lchild_pipefds[1][1];
	
	pid_t pid = fork();
	if(pid == 0){
		// LEFT CHILD
		free_all();

		close(lchild_pipefds[0][0]);
		close(lchild_pipefds[1][1]);

		dup2(lchild_pipefds[1][0], STDIN_FILENO);
		dup2(lchild_pipefds[0][1], STDOUT_FILENO);
		
		execlp(filename, filename, NULL);
	} else if(pid == -1){		
		handle_error(ONERR_LAZYEXIT | ONERR_PRINTRSN, "Could not create child process");
		return;
	}

	close(lchild_pipefds[1][0]);
	close(lchild_pipefds[0][1]);


	int rchild_pipefds[2][2];
	if(pipe(rchild_pipefds[0]) != 0){
		handle_error(ONERR_LAZYEXIT | ONERR_PRINTRSN, "Failed to create pipes");
		return;
	}
	if(pipe(rchild_pipefds[1]) != 0){
		close(rchild_pipefds[0][0]);
		close(rchild_pipefds[0][1]);
		handle_error(ONERR_LAZYEXIT | ONERR_PRINTRSN, "Failed to create pipes");
		return;
	}

	rchild_readfd = rchild_pipefds[0][0];
	rchild_writefd = rchild_pipefds[1][1];
	
	pid = fork();
	if(pid == 0){
		// RIGHT CHILD
		free_all();

		close(lchild_pipefds[0][0]);
		close(lchild_pipefds[1][1]);
		close(rchild_pipefds[0][0]);
		close(rchild_pipefds[1][1]);

		dup2(rchild_pipefds[1][0], STDIN_FILENO);
		dup2(rchild_pipefds[0][1], STDOUT_FILENO);
		
		execlp(filename, filename, NULL);
	} else if(pid == -1){
		handle_error(ONERR_LAZYEXIT | ONERR_PRINTRSN, "Could not create child process");
		return;
	}

	// PARENT
	close(rchild_pipefds[1][0]);
	close(rchild_pipefds[0][1]);

	lchild_fread = fdopen(lchild_pipefds[0][0], "r"); 
	lchild_fwrite = fdopen(lchild_pipefds[1][1], "w"); 
	rchild_fread = fdopen(rchild_pipefds[0][0], "r"); 
	rchild_fwrite = fdopen(rchild_pipefds[1][1], "w");
}

static void write_rai_number(FILE *f, rai_number n){
	fprintf(f, "%f %f*i\n", n.real, n.imag);
	fflush(f);
}

/*
 * @brief Converts string to rai_number (value with real and imaginary floats)
 * 
 * @details Lets us place any number of space-chars as we want => still will be a valid
 *				rai_number.
 *			if only real part is in the string => parse it as rai_number, and set imaginary part to 0
 *			If both real and imaginary parts are in string, then 
 *				they have to end with "*i\n"
 *			If any other non-numeric-relevant chars are detected => invalid input => 
 *				program termination
 *
 * @param String to be parsed
 * @param Length of string to be parsed.
 * 
 * @return rai_number parsed from str.
 */
static rai_number str2rai_number(char *str, size_t strSize){
	//fprintf(stderr, "String is:%s\n", str);
	rai_number out = {.real = 0, .imag = 0};
	char *alo = NULL, *endPtrReal = NULL, *endPtrImag = NULL;
	float real = strtof(str, &endPtrReal);
	if(errno == ERANGE){
		handle_error(ONERR_LAZYEXIT, "Input too big/small");
		return out;
	}
	if(str == endPtrReal){
		handle_error(ONERR_LAZYEXIT, "Invalid character in real part of a input");
		return out;
	}
	out.real = real;
	if(endPtrReal == alo + strSize - 1){
		//fprintf(stderr, "REAL IS NOW:%f\n", real);
		
		return out;
	}
	float imag = strtof(endPtrReal, &endPtrImag);
	if(errno == ERANGE){
		handle_error(ONERR_LAZYEXIT, "Input too big/small");
		return out;
	}
	if(endPtrReal == endPtrImag){
		int i = 0;
		while(1){
			char currChar = endPtrImag[0 + i++];
			if(currChar == ' '){
				continue;
			}
			if(currChar == '\n'){
				break;
			}
			handle_error(ONERR_LAZYEXIT, "Invalid char between real and imaginary parts: -%c-", currChar);
			return out;
		}
	
		return out;
	}
	if(endPtrImag <= str + strSize - 3 && strncmp(endPtrImag, "*i", 2) == 0){
		int i = 0;
		while(1){
			char currChar = endPtrImag[2 + i++];
			if(currChar == ' '){
				continue;
			}
			if(currChar == '\n'){
				break;
			}		
			handle_error(ONERR_LAZYEXIT, "Invalid char after imaginary part: -%c-", currChar);
			return out;
		}
		out.imag = imag;
	} else {
		handle_error(ONERR_LAZYEXIT, "Invalid chars in imaginary part: %s", endPtrImag);
		return out;
	}

	return out; 
}

/*
 * @brief Parses inputs from stdin via getline() and sends even/odd inputs to
 *		  	corresponding children on input arrival.
 *
 * @details Forks only if input length > 1
 * 			If there was only 1 input => output it.
 *			Terminates self and children if input length is not even.
 * 			
 */
static void parse_stdin_inputs(void){
	char *currentRaiStr = NULL;
	size_t currentRaiStrAllocedLen = 0, gotChars;
	rai_number currentRai, firstRai;
	
	while((gotChars = getline(&currentRaiStr, &currentRaiStrAllocedLen, stdin)) != -1){
		currentRai = str2rai_number(currentRaiStr, gotChars);	
		if(critical_error == 1){
			free(currentRaiStr);
			exit(EXIT_FAILURE);
		}
		totalInputs++;
		switch(totalInputs){
			case 1:
				firstRai = currentRai;
				break;
			case 2:
				setup_children_and_pipes();
				if(critical_error == 1){
					free(currentRaiStr);
					exit(EXIT_FAILURE);
				}
				write_rai_number(lchild_fwrite, firstRai);
			default:
				if(totalInputs % 2 == 0){
					write_rai_number(rchild_fwrite, currentRai);
				} else {
					write_rai_number(lchild_fwrite, currentRai);
				}
				break;
		}
	}
	if(errno == EINVAL || errno == ENOMEM){
		free(currentRaiStr);
		handle_error(ONERR_TERM | ONERR_PRINTRSN, "Could not get next line");
	}
	fclose(stdin);
	if(totalInputs == 0){
		free(currentRaiStr);
		handle_error(ONERR_TERM, "No input was specified");
	}
	if(totalInputs == 1){
		printf("%f\n", currentRai.real);
		exit(EXIT_SUCCESS);
	}
	fclose_and_null(lchild_fwrite);
	close(lchild_writefd);
	
	fclose_and_null(rchild_fwrite);
	close(rchild_writefd);

	if((totalInputs & 1) != 0){
		free(currentRaiStr);
		handle_error(ONERR_TERM, "Input is of length %d. Did you specify 2^n inputs?", totalInputs);
	}

	free(currentRaiStr);
}

/*
 * @brief Reads childrens' outputs, calculates output values
 *        	and sends them to parent.
 *
 * @details If any child terminated with EXIT_FAILURE code =>
 *				program terminates as well with EXIT_FAILURE.
 * 			Saves half of read outputs to previous_rai_numbers array.
 * 
 */
static void read_children_outputs(void){
	size_t currentRaiStrAllocedLen = 0;
	rai_number leftRai, rightRai;
	char *localCurrentRaiStr = NULL;
	int childStatus;

	if(wait(&childStatus) == -1){
		handle_error(ONERR_TERM | ONERR_PRINTRSN, "Could not wait for a child");
	}
	if(WEXITSTATUS(childStatus) == EXIT_FAILURE){
		exit(EXIT_FAILURE);
	}

	if(wait(&childStatus) == -1){
		handle_error(ONERR_TERM | ONERR_PRINTRSN, "Could not wait for a child");
	}
	if(WEXITSTATUS(childStatus) == EXIT_FAILURE){
		exit(EXIT_FAILURE);
	}
	
	rai_number *previous_rai_numbers = malloc(totalInputs / 2 * sizeof(rai_number));
	if(previous_rai_numbers == NULL){
		free(previous_rai_numbers);
		handle_error(ONERR_TERM | ONERR_PRINTRSN, "Could not allocate memory for k+n/2 values");
	}
	for(int k = 0; k < totalInputs / 2; k++){	
		size_t leftRaiLen = 0;
		if((leftRaiLen = getline(&localCurrentRaiStr, &currentRaiStrAllocedLen, lchild_fread)) == -1){
			free(previous_rai_numbers);
			handle_error(ONERR_TERM | ONERR_PRINTRSN, "Could not read input from left child");
		}
		//fprintf(stderr, "local:%s\n", localCurrentRaiStr);
		leftRai = str2rai_number(localCurrentRaiStr, leftRaiLen);
		if(critical_error == 1){
			free(previous_rai_numbers);
			exit(EXIT_FAILURE);
		}
		size_t rightRaiLen = 0;
		if((rightRaiLen = getline(&localCurrentRaiStr, &currentRaiStrAllocedLen, rchild_fread)) == -1){
			free(previous_rai_numbers);
			handle_error(ONERR_TERM | ONERR_PRINTRSN, "Could not read input from right child");
		}
		//fprintf(stderr, "local:%s\n", localCurrentRaiStr);
		rightRai = str2rai_number(localCurrentRaiStr, rightRaiLen);
		if(critical_error == 1){
			free(previous_rai_numbers);
			exit(EXIT_FAILURE);
		}

		write_rai_number(stdout, R_k(leftRai, rightRai, k));
		//fprintf(stderr, "Wrote:%f %f*i\n", R_k(leftRai, rightRai, k).real, R_k(leftRai, rightRai, k).imag);
		previous_rai_numbers[k] = R_k_n2(leftRai,rightRai, k);
		//fprintf(stderr, "Wrote:%f %f*i\n", R_k_n2(leftRai, rightRai, k).real, R_k_n2(leftRai, rightRai, k).imag);
	}

	for(int k = 0; k < totalInputs / 2; k++){
		write_rai_number(stdout, previous_rai_numbers[k]);
	}

	free(previous_rai_numbers);
		
}

int main(int argc, char **argv){
	parse_args(argc, argv);

	setup_signals();

	set_atexit();
	
	parse_stdin_inputs();
	
	read_children_outputs();


	exit(EXIT_SUCCESS);
}
