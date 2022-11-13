/**
 * @file cpair.c
 * @author Lucia Lechner <e0701149@student.tuwien.ac.at>
 * @date 06.12.2021
 *
 * @brief a recursive program which searches for the closest pair of points in a set of 2D-points
 *
 **/

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h> // wait() 
#include <unistd.h>
#include <math.h>

#define BUF_SIZE (100)

/** Structure for the points. */
typedef struct {
    float x;
    float y;
} point;


/** Name of the executable (for printing messages). */
char *program_name = "<not yet set>";

// struct of points
point *points = NULL;
point A[2]={{0}};
int A_set=0;
point B[2]={{0}};
int B_set=0;
point C[2]={{0}};
int C_set=0;
point result[2]={{0}};
int result_set=0;

//process id's used for fork
pid_t pid_lower;
pid_t pid_upper;

//use two unnamed pipes per child to redirect stdin and stdout
int fd_pipe_lower_in[2]; //STD_IN from parent to lower child
int fd_pipe_lower_out[2]; //STD_OUT from lower child to parent

int fd_pipe_upper_in[2]; //STD_IN from parent to upper child
int fd_pipe_upper_out[2]; //STD_OUT from upper child to parent

/**
 * @brief print usage message and terminate program with EXIT_FAILURE
 */
void usage(char *msg){
	fprintf(stderr, "Usage: %s < 1.txt\n%s\n", program_name, msg);
	exit(EXIT_FAILURE);
}

/**
 * @brief Print error message and terminate program with EXIT_FAILURE
 * @param msg Message to print to stderr
 */
void error_exit(char* msg) {
	fprintf(stderr, "%s: %s\n", program_name, msg);
	exit(EXIT_FAILURE);
}

void free_resources(void){
	if(points != NULL) free(points);
	
	
	//close all pipes
	close(fd_pipe_lower_out[0]);
	close(fd_pipe_lower_in[0]);
	close(fd_pipe_upper_out[0]);
	close(fd_pipe_upper_in[0]);

	close(fd_pipe_lower_out[1]);
	close(fd_pipe_lower_in[1]);
	close(fd_pipe_upper_out[1]);
	close(fd_pipe_upper_in[1]);
	
}

/**
 * @brief Parse arguments
 *
 * Synopsis:
 *   ./cpair
 * Example:
 *   $ cat 1.txt
 *   4.0 4.0
 *   -1.0 1.0
 *   1.0 -1.0
 *   -4.0 -4.0
 *   $ ./cpair < 1.txt
 *   -1.000000 1.000000
 *   1.000000 -1.000000
 *
 * @param argc Number of elements in argv
 * @param argv Array of command line arguments
 * @param points Array of struct with the parsed arguments
 * @return size_t number of points
 **/
size_t parse_points(int argc, char *argv[]) {
	int abort = 0;
	size_t N=0; //size of array
	char* line;
	size_t nbytes; // the size of the allocated buffer
	
	if(argc > 1) usage("wrong number of commandline arguments");
	
	nbytes = BUF_SIZE*sizeof(char); // set line buffer size
    	line = (char *) malloc (nbytes);
    	if(line == NULL) error_exit("malloc failed");
	memset(line, 0, nbytes);

	//ssize_t nread;
	//while ((nread = read(STDIN_FILENO, line, BUF_SIZE)) > 0 && (strcmp(line, "\n") !=0)) {

	//fprintf(stderr,"stderr: bytes: %ld %s\n", nread, line);

//	    nwrite = write(STDOUT_FILENO, buf, nread);
	    /* Error handling and partial writes left as exercise. */
//	}
		//strcmp(line,"\n") != 0
	
	//read values from stdin until EOF is encountered
	while(fgets(line, BUF_SIZE, stdin) != NULL && strcmp(line, "\n") != 0){
	
		//split on space
		//TODO verify that there is only one space
		char *second;
		second = strchr(line, ' ');
		if(second != NULL){	// replace space with '\0'
			*second++ = '\0';
		} else { // if there is no space break and quit (because it means that there are not two points in one row)
			abort=1;
			break; 
		}
		
		// increase size of array
    		N+=1;
		point *tmp;
		tmp = (point *) calloc((N), sizeof(point));
		for (int i=0;i<(N-1);i++) tmp[i]=points[i];
		free(points);
		points = tmp;
		tmp = NULL;
		
		// convert line to float
		float real;
	        char* pt_real=NULL;
		real = strtof(line, &pt_real);
//		if(scanf("%f",&real) == 1){
			points[N-1].x = real;
//		} else {
//			abort=1;
//			break;
//		}
		real = strtof(second, &pt_real);	
//		if(scanf("%f",&real) == 1){
			points[N-1].y = real;
//		} else {
//			abort=1;
//			break;
//		}

//		for(int i=0;i<N;i++)fprintf(stderr,"stderr: point[%d](%f,%f)\n", i, points[i].x, points[i].y);
	
	}
	free(line);

	if(abort) usage("wrong number of coordinates");
	
	return N;
	
}

float calculate_distance(point A, point B){

	return sqrt( 
		(B.x - A.x) * (B.x - A.x)  +
		(B.y - A.y) * (B.y - A.y) 
		);
}

int main(int argc, char *argv[]) {

	program_name = argv[0];

	size_t n = 0; //number of points in array
	int quit = 0;

	// parse the input	
	n = parse_points(argc, argv);
	
//	for(int i=0;i<n;i++) fprintf(stderr,"stderr: (%f,%f)\n", points[i].x, points[i].y);
	
	switch(n){ //switch on number of points in array
	
		case 0: fprintf(stderr, "zero?\n");
			quit=1;
			break;
		case 1: 
			quit=1; //exit without any output
			break;
		case 2:
			fprintf(stdout, "%f %f\n%f %f\n", points[0].x, points[0].y, points[1].x, points[1].y);
			fflush(stdout);
//			fprintf(stderr, "(stderr):\n%f %f\n%f %f\n", points[0].x, points[0].y, points[1].x, points[1].y);
			quit=1; 
			break;
	}

	if(!(n>2)) quit=1; //verify that there are more than 2 points
	
	if(quit){
		free_resources();
		exit(EXIT_SUCCESS);
	}
	
	//calculate mean
	float sum_x=0, mean_x=0;
	for(int i=0;i<n;i++){
		sum_x += points[i].x;
	}
	mean_x = sum_x/n;

/*	point *upper_points;
	upper_points = (point *) calloc((N), sizeof(point));
	point *lower_points;
	lower_points = (point *) calloc((N), sizeof(point));
	for (int i=0;i<n;i++){
		if(points[i].x <= mean_x){
			upper_points[i]=points[i];
		}
		if(points[i].x > mean_x){
			lower_points[i]=points[i];
		}
	}
*/
//	fprintf(stderr, "[%d] mean_x: %f\n", getpid(), mean_x);
	

	//setup pipes
	if( pipe(fd_pipe_lower_in) == -1 ) error_exit("pipe setup failed.");
	if( pipe(fd_pipe_lower_out) == -1 ) error_exit("pipe setup failed.");
	if( pipe(fd_pipe_upper_in) == -1 ) error_exit("pipe setup failed.");
	if( pipe(fd_pipe_upper_out) == -1 ) error_exit("pipe setup failed.");


	pid_lower = fork(); // fork the lower child and connect the pipes
	if( pid_lower == -1 ) {
		error_exit("fork of lower child failed.");
	} else if( pid_lower > 0 ) { //parent starting lower child
		
		//close read end at parent
		if(close(fd_pipe_lower_in[0]) == -1) error_exit("error closing read end of lower child at parent process");
		
		//write points to write end of lower child pipe
		for(int i=0; i<n; i++){
	
			if(points[i].x <= mean_x){
				//point goes to the lower child
//				fprintf(stderr, "[%d] goes to lower child: %f %f\n", getpid(), points[i].x, points[i].y);
				dprintf(fd_pipe_lower_in[1], "%f %f\n", points[i].x, points[i].y);
			}
		}

		//close the pipe we just wrote to
		if(close(fd_pipe_lower_in[1]) == -1) 
			error_exit("error closing write end of lower child at parent process");		
		
		/*
		[x] close(fd_pipe_lower_in[0]);
		[x] close(fd_pipe_lower_in[1]);
		close(fd_pipe_lower_out[0]);
		close(fd_pipe_lower_out[1]);

		close(fd_pipe_upper_in[0]);
		close(fd_pipe_upper_in[1]);
		close(fd_pipe_upper_out[0]);
		close(fd_pipe_upper_out[1]);
		*/
		
		pid_upper = fork(); // fork the upper child and connect the pipes
		if( pid_upper == -1 ) {
   			error_exit("fork of upper child failed.");
		} else if ( pid_upper > 0 ) { //parent starting upper child
		
			//close read end at parent
			if(close(fd_pipe_upper_in[0]) == -1) error_exit("error closing read end of upper child at parent process");		
			
			//forward points to upper child process
			for(int i=0; i<n; i++){
	
				if(points[i].x > mean_x){
					//point goes to the upper child
//					fprintf(stderr, "[%d] goes to upper child: %f %f\n", getpid(), points[i].x, points[i].y);			
   					dprintf(fd_pipe_upper_in[1], "%f %f\n", points[i].x, points[i].y);
   				}

			}
			//close the pipe we just wrote to
			if(close(fd_pipe_upper_in[1]) == -1) 
				error_exit("error closing write end of upper child at parent process");


		/*
		[x] close(fd_pipe_lower_in[0]);
		[x] close(fd_pipe_lower_in[1]);
		close(fd_pipe_lower_out[0]);
		close(fd_pipe_lower_out[1]);

		[x] close(fd_pipe_upper_in[0]);
		[x] close(fd_pipe_upper_in[1]);
		close(fd_pipe_upper_out[0]);
		close(fd_pipe_upper_out[1]);
		*/
		
		
						//read the result from upper child's stdout
	close(fd_pipe_upper_out[1]);
	
	int pos=0;
	char* line;
	size_t nbytes; // the size of the allocated buffer	
	nbytes = 100*sizeof(char); // set line buffer size
	line = (char *) malloc (nbytes);
    	if(line == NULL) error_exit("malloc failed");
	memset(line, 0, nbytes);
	
	while(read(fd_pipe_upper_out[0],line, BUF_SIZE) > 0 ){
	}
	//close the pipe we just read from
	close(fd_pipe_upper_out[0]);
//	fprintf(stderr, "[%d] read from upper_out: %s\n", getpid(), line);

	char *left = strchr(line, '\n');
	if(left != NULL){
		char *right = line;

		//split on space
		char *second;
		second = strchr(left, ' ');
		if(second != NULL){	// replace space with '\0'
			*second++ = '\0';
		}
		// convert line to float
		float real;
		char* pt_real=NULL;
		real = strtof(left, &pt_real);
		B[pos].x = real;
		real = strtof(second, &pt_real);		
		B[pos].y = real;
		pos++;

		//split on space
		second = strchr(right, ' ');
		if(second != NULL){	// replace space with '\0'
			*second++ = '\0';
		} 
		// convert line to float
		pt_real=NULL;
		real = strtof(right, &pt_real);
		B[pos].x = real;
		real = strtof(second, &pt_real);		
		B[pos].y = real;

		B_set=1;
	}
	free(line);
		
		} else { //if( pid_upper == 0 ) upper child
		
			//connect this child's write end to stdout
			if(dup2(fd_pipe_upper_out[1], STDOUT_FILENO)==-1){
				error_exit("error duplicating STDOUT for write end of the upper child");
			}
			close(fd_pipe_upper_out[1]); //close write end				
   			close(fd_pipe_upper_out[0]); //close read end

			//connect this child's read end to stdin
			if(dup2(fd_pipe_upper_in[0], STDIN_FILENO)==-1){
				error_exit("error duplicating STDIN for read end of the upper child");
			}
   			close(fd_pipe_upper_in[0]); //close read end
			close(fd_pipe_upper_in[1]); //close write end


			execlp("./cpair", "cpair", (char *) NULL);
	   		error_exit("exec failed."); //this line is only reached if execlp was unsuccessful		

		close(fd_pipe_lower_out[0]);
		close(fd_pipe_lower_out[1]);
						
		/*
		[x] close(fd_pipe_lower_in[0]);
		[x] close(fd_pipe_lower_in[1]);
		[x] --> close(fd_pipe_lower_out[0]);
		[x] --> close(fd_pipe_lower_out[1]);

		[x] close(fd_pipe_upper_in[0]);
		[x] close(fd_pipe_upper_in[1]);
		[x] close(fd_pipe_upper_out[0]);
		[x] close(fd_pipe_upper_out[1]);
		*/
			
			
		}
		
		
			//read the result from lower child's stdout
	close(fd_pipe_lower_out[1]);
	
	int pos = 0;
	char* line;
	size_t nbytes; // the size of the allocated buffer	
	nbytes = 100*sizeof(char); // set line buffer size
	line = (char *) malloc (nbytes);
    	if(line == NULL) error_exit("malloc failed");
	memset(line, 0, nbytes);
	
	while(read(fd_pipe_lower_out[0],line, BUF_SIZE) > 0 ){
	}
	//close the pipe we just read from
	close(fd_pipe_lower_out[0]);
//	fprintf(stderr, "[%d] read form lower_out: %s\n", getpid(), line);

	char *left = strchr(line, '\n');
	if(left != NULL){
		char *right = line;
		if(right == NULL)
			fprintf(stderr, "right is null. \n");

		//split on space
		char *second;
		second = strchr(left, ' ');
		if(second != NULL){	// replace space with '\0'
			*second++ = '\0';
		}
		// convert line to float
		float real;
		char* pt_real=NULL;
		real = strtof(left, &pt_real);
		A[pos].x = real;
		real = strtof(second, &pt_real);		
		A[pos].y = real;
		pos++;

		//split on space
		second = strchr(right, ' ');
		if(second != NULL){	// replace space with '\0'
			*second++ = '\0';
		} 
		// convert line to float
		pt_real=NULL;
		real = strtof(right, &pt_real);
		A[pos].x = real;
		real = strtof(second, &pt_real);		
		A[pos].y = real;

		A_set=1;
	}
	
	free(line);
			
	} else { //if( pid_lower == 0 ) lower child
					
		//connect this child's write end to STDOUT
		if(dup2(fd_pipe_lower_out[1], STDOUT_FILENO)==-1){
			error_exit("error duplicating STDOUT for write end of the lower child");
		}
		close(fd_pipe_lower_out[1]); //close write end				
   		close(fd_pipe_lower_out[0]); //close read end

		//connect this child's read end to STDIN
		if(dup2(fd_pipe_lower_in[0], STDIN_FILENO)==-1){
			error_exit("error duplicating STDIN for read end of the lower child");
		}
   		close(fd_pipe_lower_in[0]); //close read end
		close(fd_pipe_lower_in[1]); //close write end
			

		execlp("./cpair", "cpair", (char *) NULL);
   		error_exit("exec failed."); //this line is only reached if execlp was unsuccessful

		/*
		[x] close(fd_pipe_lower_in[0]);
		[x] close(fd_pipe_lower_in[1]);
		[x] close(fd_pipe_lower_out[0]);
		[x] close(fd_pipe_lower_out[1]);

		close(fd_pipe_upper_in[0]);
		close(fd_pipe_upper_in[1]);
		close(fd_pipe_upper_out[0]);
		close(fd_pipe_upper_out[1]);
		*/

	}
	
	//wait for all children to end
	//terminate the program with EXIT_FAILURE if the exit status of any child is not exit_success
	int wait_status_lower;
	int wait_status_upper;
	do{
		if(waitpid(pid_lower, &wait_status_lower, 0) == -1) error_exit("exit failure occured at lower child.");
		if(WEXITSTATUS(wait_status_lower) == EXIT_FAILURE) error_exit("exit failure occured at lower child.");
	} while(!WIFEXITED(wait_status_lower));
	
	do{
		if(waitpid(pid_upper, &wait_status_upper, 0) == -1) error_exit("exit failure occured at upper child.");
		if(WEXITSTATUS(wait_status_upper) == EXIT_FAILURE) error_exit("exit failure occured at upper child.");
	} while(!WIFEXITED(wait_status_upper));


//	for(int i=0;i<2;i++) fprintf(stdout,"A(%f,%f)\n", A[i].x, A[i].y);
//	for(int i=0;i<2;i++) fprintf(stdout,"B(%f,%f)\n", B[i].x, B[i].y);
//	for(int i=0;i<2;i++) fprintf(stdout,"C(%f,%f)\n", C[i].x, C[i].y);
//	for(int i=0;i<2;i++) fprintf(stdout,"result (%f,%f)\n", result[i].x, result[i].y);

	if(A_set && B_set){
		float smallest_distance = calculate_distance(A[0],B[0]);
		C[0].x = A[0].x;
		C[0].y = A[0].y;
		C[1].x = B[0].x;
		C[1].y = B[0].y;

		for(int i=0;i<2;i++){
			for(int j=0; j<2; j++){
				float d = calculate_distance(A[i],B[j]);
				//fprintf(stderr, "distance A[%d] B[%d] = %f\n", i, j, d);
				if(d<smallest_distance){
					C[0].x = A[i].x;
					C[0].y = A[i].y;
					C[1].x = B[j].x;
					C[1].y = B[j].y;
					smallest_distance=d;
		
//					C_set=1;
//					fprintf(stderr, "[%d] smallest distance A(%f,%f) B(%f,%f): C[0]=(%f,%f) C[1]=(%f,%f)\n", getpid(), A[i].x, A[i].y, B[j].x, B[j].y, C[0].x, C[0].y, C[1].x, C[1].y);
				}
			}
		}
		result[0].x = C[0].x;
		result[0].y = C[0].y;
		result[1].x = C[1].x;
		result[1].y = C[1].y;
	
//		if(C_set){
			float closest = calculate_distance(A[0],A[1]);
			result[0].x = A[0].x;
			result[0].y = A[0].y;
			result[1].x = A[1].x;
			result[1].y = A[1].y;

			float d = calculate_distance(A[0],A[1]);
			if(d<closest){
				result[0].x = A[0].x;
				result[0].y = A[0].y;
				result[1].x = A[1].x;
				result[1].y = A[1].y;
				closest=d;
			}
		
			d = calculate_distance(B[0],B[1]);
			if(d<closest){
				result[0].x = B[0].x;
				result[0].y = B[0].y;
				result[1].x = B[1].x;
				result[1].y = B[1].y;
				closest=d;
			}
		
			d = calculate_distance(C[0],C[1]);
			if(d<closest){
				result[0].x = C[0].x;
				result[0].y = C[0].y;
				result[1].x = C[1].x;
				result[1].y = C[1].y;
				closest=d;
			}
			fprintf(stderr, "[%d] A (%f,%f) (%f,%f), B(%f,%f) (%f,%f), C: (%f,%f) (%f,%f)\n", getpid(), A[0].x, A[0].y, A[1].x, A[1].y, B[0].x, B[0].y, B[1].x, B[1].y, C[0].x, C[0].y, C[1].x, C[1].y);
			fprintf(stderr, "[%d] closest: (%f,%f) (%f,%f)\n", getpid(), result[0].x, result[0].y, result[1].x, result[1].y);
//		}
	}

	if(A_set && !B_set){
		result[0].x = A[0].x;
		result[0].y = A[0].y;
		result[1].x = A[1].x;
		result[1].y = A[1].y;	
	}

	if(B_set && !A_set){
		result[0].x = B[0].x;
		result[0].y = B[0].y;
		result[1].x = B[1].x;
		result[1].y = B[1].y;	
	}

	//fprintf(stderr,"[%d] result: %f %f\n%f %f\n", getpid(), result[0].x, result[0].y, result[1].x, result[1].y);
	fprintf(stdout,"%f %f\n%f %f\n", result[0].x, result[0].y, result[1].x, result[1].y);

    free_resources();
    return EXIT_SUCCESS;
    
}
