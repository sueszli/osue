/**
 * @brief This is the main module for forkFFT.
 * It contains everything needed for the program to run.
 * @author Florian Weinzerl (11701313)
 * @date 2021-12-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include <complex.h>
#include <string.h>

/**
 * @brief Parses a complex number from a string.
 * @details The string str may be of the form "%f[ %f[*i]]" and will be converted to
 * a complex float stored in result, if possible. If not, false is returned.
 * @param str the input string
 * @param result a pointer to where the result should be stored
 * @return true if parsing was successful
 */
bool parse_complex(char *str, complex float *result);
/**
 * @brief Reconnects pipes from a child processes point of view to its std in- and outputs.
 * @details A helper function that handles the given pipes for a child process.
 * @param pipefd_to_child the file descriptor of the pipe that the parent uses to speak to the child
 * @param pipefd_to_parent the file descriptor of the pipe that the child uses to speak to the parent
 */
void lay_children_pipes(int pipefd_to_child[2], int pipefd_to_parent[2]);

int main(){
	// --- input ---
	char *inbuf = NULL; // input buffer
	size_t inbuf_len;
	size_t inbuf_strlen; // inbuf content length
	size_t n = 0; // length of input series (same as length of fourier'd output)
	
	if((inbuf_strlen = getline(&inbuf, &inbuf_len, stdin)) == -1){ // receive EOF instantly -> failure
		free(inbuf);
		exit(EXIT_FAILURE);
	}
	
	// declare pipe and pid vars for the n>1 case
	int pipefds_to_children[2][2];
	int pipefds_to_parent[2][2];
	pid_t cpids[2] = {1, 1}; // so the checks against 0 will fail
	
	{ // temp vars not needed afterwards
		char *tempbuf = NULL; // temporary second input buffer
		size_t tempbuf_len;
		size_t tempbuf_strlen; // tempbuf content length
		
		if((tempbuf_strlen = getline(&tempbuf, &tempbuf_len, stdin)) == -1){ // only 1 input -> straight to stdout
			free(tempbuf);
			complex float num;
			bool parse_success = parse_complex(inbuf, &num);
			free(inbuf);
			if(!parse_success)
				exit(EXIT_FAILURE);
			printf("%f %f*i\n", crealf(num), cimagf(num));
			exit(EXIT_SUCCESS);
		} else{ // more than 1 input -> quickly prepare pipes, fork and empty buffers into pipes
			// - build pipes -
			for(int i = 0; i < 2; i++){
				if (pipe(pipefds_to_children[i]) == -1
				|| pipe(pipefds_to_parent[i]) == -1){
					free(tempbuf);
					free(inbuf);
					exit(EXIT_FAILURE);
				}
			}

			// - fork twice -
			for(int i = 0; i < 2; i++){
				cpids[i] = fork();
				if(cpids[i] == -1){
					free(tempbuf);
					free(inbuf);
					exit(EXIT_FAILURE);
				}
				if(cpids[i] == 0) break; // we're in the child proc, no more forking
			}
			
			// - lay pipes -
			for(int i = 0; i < 2; i++){
				if(cpids[i] == 0){
					// close pipes used by other child
					for(int j = 0; j < 2; j++){
						close(pipefds_to_children[(i+1) % 2][j%2]);
						close(pipefds_to_parent[(i+1) % 2][j%2]);
					}
					
					// lay own pipes
					lay_children_pipes(pipefds_to_children[i], pipefds_to_parent[i]);
					break;
				}
			}
			
			if(cpids[0] != 0
			&& cpids[1] != 0){ // in parent process
				for(int i = 0; i < 2; i++){
					close(pipefds_to_children[i][0]); // close unused read end
					close(pipefds_to_parent[i][1]); // close unused write end
				}
			}
			
			// - start child processes -
			if(cpids[0] == 0
			|| cpids[1] == 0){
				execlp("./forkFFT", "forkFFT", NULL);
				free(tempbuf);
				free(inbuf);
				exit(EXIT_SUCCESS);
			}
			
			// - write buffer content to pipes -
			write(pipefds_to_children[0][1], inbuf, inbuf_strlen);
			write(pipefds_to_children[1][1], tempbuf, tempbuf_strlen);
		}
		
		free(tempbuf);
	}
	
	// --- recursion ---
	n = 2;
	while((inbuf_strlen = getline(&inbuf, &inbuf_len, stdin)) != -1){
		write(pipefds_to_children[n%2][1], inbuf, inbuf_strlen);
		n++;
	}
	
	close(pipefds_to_children[0][1]);
	close(pipefds_to_children[1][1]);
	if(n%2 != 0)
		exit(EXIT_FAILURE);
	
	// --- calculation & first half of output ---
	complex float *series;
	series = malloc(sizeof(complex float) * n/2);
	
	/*
	 * R[k]     = R_E[k] + e^{-2*pi*1i/n * k} * R_O[k]
	 * R[k+n/2] = R_E[k] - e^{-2*pi*1i/n * k} * R_O[k]
	 * R ... result in this proc
	 * R_E ... result of child proc with even indexed inputs
	 * R_O ... result of child proc with odd indexed inputs
	 */
	{
		FILE *pfile_evn = fdopen(pipefds_to_parent[0][0], "r");
		FILE *pfile_odd = fdopen(pipefds_to_parent[1][0], "r");
		
		complex float h, rek, rok; // helper, R_E[k], R_O[k]
		
		// repurpose inbuf as input buffer for child proc communication
		for(int k = 0; k < n/2; k++){
			// parse R_E[k]
			if((inbuf_strlen = getline(&inbuf, &inbuf_len, pfile_evn)) == -1
			|| !parse_complex(inbuf, &rek)){ // because left-to-right eval is guaranteed
			free(series);
				free(inbuf);
				exit(EXIT_FAILURE); // should never happen if child processes are handled correctly
			}
			// parse R_O[k]
			if((inbuf_strlen = getline(&inbuf, &inbuf_len, pfile_odd)) == -1
			|| !parse_complex(inbuf, &rok)){ // because left-to-right eval is guaranteed
			free(series);
				free(inbuf);
				exit(EXIT_FAILURE); // should never happen if child processes are handled correctly
			}
			
			// calc e^{-2*pi*1i/n * k} * R_O[k]
			h = cexpf(-2 * 3.141592654f * I / n * k) * rok;
			// store R[k+n/2] to series[k]
			series[k] = rek - h;
			// calc R[k]
			h += rek;
			// print R[k]
			printf("%f %f*i\n", crealf(h), cimagf(h));
		}
		
		fclose(pfile_evn);
		fclose(pfile_odd);
	}
	
	int status;
	for(int i = 0; i < 2; i++){
		waitpid(cpids[i], &status, 0); // wait for child i
		if(WEXITSTATUS(status) == EXIT_FAILURE){
			free(series);
			free(inbuf);
			exit(EXIT_FAILURE);
		}
	}
	
	// --- second half of output ---
	for(int k = 0; k < n/2; k++)
		// print R[k+n/2]
		printf("%f %f*i\n", crealf(series[k]), cimagf(series[k]));
	
	free(series);
	free(inbuf);
}

bool parse_complex(char *str, complex float *result){ // expect str of form "%f[ %f[*i]]"
	char *endptr;
	
	*result = strtof(str, &endptr);
	if(str == endptr) // no float found
		return false;
	if(strlen(endptr) <= 1) // only real part given
		return true;
	
	str = endptr+1; // skip " " and get remaining "%f[*i]"
	*result += strtof(str, &endptr) * I;
	if(str == endptr) // no float found
		return false;
	return true;
}

void lay_children_pipes(int pipefd_to_child[2], int pipefd_to_parent[2]){
	close(pipefd_to_child[1]); // close unused write end
	dup2(pipefd_to_child[0], STDIN_FILENO);
	close(pipefd_to_child[0]);
	
	close(pipefd_to_parent[0]); // close unused read end
	dup2(pipefd_to_parent[1], STDOUT_FILENO);
	close(pipefd_to_parent[1]);
}
