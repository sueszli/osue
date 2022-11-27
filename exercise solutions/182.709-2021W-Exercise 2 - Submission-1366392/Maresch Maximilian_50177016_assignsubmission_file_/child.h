/**
 * @author Maximilian Maresch
 */ 

/**
 * @brief Reads the result of the calculation executed by a child process
 * @detail This function is used to read the result of the calculation executed by a child process running intmul. 
 * This result is leveraged by the parent as part of a bigger calculation.
 * @param stdoutPipe - the pipe used to communicate between the parent and the child for stdout (from this pipe will be read)
 * @param argv - argv from main
 * @return the result calculated by the child process
 */ 
char* readResultFromChild(int stdoutPipe[], char* argv[]);

/**
 * @brief Sets up a new child process
 * @detail This function is used to set up a new child process executing intmul and feeding that process a partial 
 * calculation to carry out. The result of this calculation is leveraged by the parent as part of a bigger calculation.
 * @param stdinPipe - the pipe used to communicate between the parent and the child for stdin
 * @param stdoutPipe - the pipe used to communicate between the parent and the child for stdout
 * @param line1 - the first line to provide to the new child
 * @param line2 - the second line to provide to the new child
 * @param argv - argv from main
 * @return the PID of the child which was set up
 */ 
pid_t setupChild(int stdinPipe[], int stdoutPipe[], char* line1, char* line2, char* argv[]);