/**
 * @file forkFFT.c
 * @author Dunja Kreckovic (11928274)
 * @date 8.12.2021
 *
 * @brief Contains the functionality of the Fork Fourier Transformation.
 *
 * @details Program computes the Fourier Transform of its input values recursively, by calling itself. The input values are floating point values, which should be read from stdin.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>

static char *prog_name; 

/**
 * @brief Prints usage message and terminates the program.
 *
 * @details prog_name - Global variable used in this function. It contains the program name specified in argv[0].
 **/
static void usage(void){
  fprintf(stderr,"[%s] Usage: %s\n",prog_name,prog_name);
  exit(EXIT_FAILURE);
}

/**
 * @brief Prints error message and terminates the program.
 *
 * @details prog_name - Global variable used in this function. It contains the program name specified in argv[0].
 *
 * @param message - Message that should be printed on stderr.
 **/
static void print_error_and_exit(char* message){
  fprintf(stderr,"[%s] ERROR: %s: %s\n",prog_name,message,strerror(errno));
  exit(EXIT_FAILURE);
}


/**
 * @brief The starting point of the forkFFT program.
 *
 * @details This function performs all the functionalities of this program. If the input consists of only one number, this number will be written to stdout and the program will terminate. Otherwise the input values will be splittet into two parts, one for every child. Recursively will be created two child processes. The communication will be performed with two unnamed pipes per child. The stdin and stdout of children will be redirected. Wenn both children finish their job, the result will be computed and printted to stdout. In the end all resources will be cleaned up.
 *
 * @param argc - Number of argument values. It must be equal to 1.
 * @param argv[] - Argument values. It should only contain the name of the program.
 *
 * @return Returns EXIT_SUCESS upon success or EXIT_FAILURE upon failure.
 **/
int main(int argc,char *argv[]) {

  const float PI = 3.141592654;

  prog_name = argv[0];

  if (argc != 1){
    usage();
  }

// getting first number form stdin and saving it into line_for_child1
  char *line_for_child1 = NULL;
  size_t length1 = 0;
  if(getline(&line_for_child1,&length1,stdin) == -1){
    if(feof(stdin)){ // no input
      fprintf(stderr,"[%s] No numbers inputted.\n", prog_name);
      free(line_for_child1);
      exit(EXIT_FAILURE);
    }else{ 
      free(line_for_child1);
      print_error_and_exit("getline failed");
    }
  }


// getting second number from stdin and saving it into line_for_child2
  char *line_for_child2 = NULL;
  size_t length2 = 0;
   if(getline(&line_for_child2,&length2,stdin) == -1){
    if(feof(stdin)){ // only one number inputted
      char *end_pointer;
      float number = strtof(line_for_child1,&end_pointer);
      if(errno != 0){
        free(line_for_child2);
        free(line_for_child1);
        print_error_and_exit("strtof failed");
      }
      if(*end_pointer == 10){ // ASCII decimal value for enter
        fprintf(stdout,"%f %d\n",number,0);
        free(line_for_child2);
        free(line_for_child1);       
        exit(EXIT_SUCCESS);
      } else{
        fprintf(stderr,"[%s] Given not floating number\n",prog_name);
        free(line_for_child2);
        free(line_for_child1);
        exit(EXIT_FAILURE);
      }
    }else{ 
      free(line_for_child1);
      free(line_for_child2);
      print_error_and_exit("getline failed");
    } 
  }

  
  // creating pipes for communication block
  int pipefd_parent_to_child1[2];
  int pipefd_parent_to_child2[2];
  int pipefd_child1_to_parent[2];
  int pipefd_child2_to_parent[2];

  if(pipe(pipefd_child1_to_parent) == -1){
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("pipe failed");
  }
  if(pipe(pipefd_child2_to_parent) == -1){
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("pipe failed");
  }
  if(pipe(pipefd_parent_to_child1) == -1){
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("pipe failed");
  }
  if(pipe(pipefd_parent_to_child2) == -1){
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("pipe failed");
  }
  // end block


  // creating first child process
  pid_t child1_pid = fork();

  switch(child1_pid){

    case -1: //error
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("fork failed");

    case 0: //child1
    // redirecting of stdin and stdout of child block
    if(dup2(pipefd_parent_to_child1[0],STDIN_FILENO) == -1){
      free(line_for_child1);
      free(line_for_child2);
      print_error_and_exit("dup2 failed");
    }
    if(dup2(pipefd_child1_to_parent[1],STDOUT_FILENO) == -1){
      free(line_for_child1);
      free(line_for_child2);
      print_error_and_exit("dup2 failed");
    }
    // end block

    // closing unused fds block
    if(close(pipefd_parent_to_child1[0]) == -1 || close(pipefd_parent_to_child1[1]) == -1 || close(pipefd_child1_to_parent[0]) == -1 || close(pipefd_child1_to_parent[1]) == -1 || close(pipefd_child2_to_parent[0]) == -1 || close(pipefd_child2_to_parent[1]) == -1 || close(pipefd_parent_to_child2[0]) == -1 ||
    close(pipefd_parent_to_child2[1]) == -1){
      free(line_for_child1);
      free(line_for_child2);
      print_error_and_exit("close failed");
    }
    // end block

    // recursion: executing the same program as a child
    execl(prog_name, prog_name, NULL);

    // this should not be reached 
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("execl failed");

    default: // parent
    break;
  }

  // this line can only be reached by parent

  // creating second child process
  pid_t child2_pid = fork();

  switch(child2_pid){

    case -1: // error
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("fork failed");

    case 0: // child2
    // redirecting of stdin and stdout of child block
    if(dup2(pipefd_parent_to_child2[0],STDIN_FILENO) == -1){
      free(line_for_child1);
      free(line_for_child2);
      print_error_and_exit("dup2 failed");
    }
    if(dup2(pipefd_child2_to_parent[1],STDOUT_FILENO) == -1){
      free(line_for_child1);
      free(line_for_child2);
      print_error_and_exit("dup2 failed");
    }
    // end block
 
 // closing unused fds block
    if(close(pipefd_parent_to_child1[0]) == -1 || close(pipefd_parent_to_child1[1]) == -1 || close(pipefd_child1_to_parent[0]) == -1 || close(pipefd_child1_to_parent[1]) == -1 || close(pipefd_child2_to_parent[0]) == -1 || close(pipefd_child2_to_parent[1]) == -1 || close(pipefd_parent_to_child2[0]) == -1 ||
      close(pipefd_parent_to_child2[1]) == -1){
      free(line_for_child1);
      free(line_for_child1);
      print_error_and_exit("close failed");
    }
    // end block

    // recursion: executing the same program as a child
    execl(prog_name, prog_name, NULL);

    // this should not be reached
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("execl failed");
    default: //parent
    break;
  }

  // this line can only be reached by parent

 // closing unused fds block
  if (close(pipefd_parent_to_child2[0]) == -1 || close(pipefd_child2_to_parent[1]) == -1 || close(pipefd_child1_to_parent[1]) == -1 || close(pipefd_parent_to_child1[0]) == -1) {
    free(line_for_child1);
    free(line_for_child2);
    print_error_and_exit("close failed");
  }
  // end block


 // opening files for communication with children block
  FILE *write_to_child1 = fdopen(pipefd_parent_to_child1[1],"w");
  FILE *write_to_child2 = fdopen(pipefd_parent_to_child2[1],"w");

  if(write_to_child1 == NULL || write_to_child2 == NULL){
    free(line_for_child1);
    free(line_for_child2);
    fclose(write_to_child1);
    fclose(write_to_child2);
    print_error_and_exit("fdopen failed");
  }
  // end block


  // writing two first numbers to children block
  int num_of_lines = 2;
  if(fprintf(write_to_child1,"%s",line_for_child1) < 0){
    free(line_for_child1);
    free(line_for_child2);
    fclose(write_to_child1);
    fclose(write_to_child2);
    print_error_and_exit("fprintf failed");
  }
  if(fprintf(write_to_child2,"%s",line_for_child2) < 0){
    free(line_for_child1);
    free(line_for_child2);
    fclose(write_to_child1);
    fclose(write_to_child2);
    print_error_and_exit("fprintf failed");
  }
  fflush(write_to_child1);
  fflush(write_to_child2);
  free(line_for_child1);
  free(line_for_child2);
  // end block

  
  // writing the rest numbers to children block
  char *line = NULL;
  size_t length = 0;
  while(getline(&line,&length,stdin) != -1){
    num_of_lines++;
    if(num_of_lines % 2 != 0){
      if(fprintf(write_to_child1,"%s",line) < 0){
        free(line);
        fclose(write_to_child1);
        fclose(write_to_child2);
        print_error_and_exit("fprintf failed");
      }
    }
    else{
      if(fprintf(write_to_child2,"%s",line) < 0){
        free(line);
        fclose(write_to_child1);
        fclose(write_to_child2);
        print_error_and_exit("fprintf failed");
      }
    }
  }
  if(feof(stdin) == 0){
    free(line);
    fclose(write_to_child1);
    fclose(write_to_child2);
    print_error_and_exit("getline failed");
  }
  if(num_of_lines % 2 != 0){ // odd number of inputted numbers
    fprintf(stderr,"[%s] ERROR: invalid number of floating numbers",prog_name);
    free(line);
    fclose(write_to_child1);
    fclose(write_to_child2);
    exit(EXIT_FAILURE);
  }
 // end block


 // closing files for writing
  if (fclose(write_to_child1) == EOF || fclose(write_to_child2) == EOF){
    free(line);
    print_error_and_exit("fclose failed");
  } 

 
 // waiting for children block
  int status1;
  int status2;
  if(waitpid(child1_pid, &status1, 0) == -1){
    free(line);
    print_error_and_exit("waitpit failed");
  }
  if(waitpid(child2_pid, &status2, 0) == -1){
    free(line);
    print_error_and_exit("waitpit failed");
  } 

  if (WEXITSTATUS(status1) != EXIT_SUCCESS){
    fprintf(stderr, "[%s] ERROR: child1 exited with error\n", prog_name);
    free(line);
    exit(EXIT_FAILURE);
  }
  if (WEXITSTATUS(status2) != EXIT_SUCCESS){
    fprintf(stderr, "[%s] ERROR: child2 exited with error\n", prog_name);
    free(line);
    exit(EXIT_FAILURE);
  }
// end block

 
  // opening files for reading from children block
  FILE *read_from_child1 = fdopen(pipefd_child1_to_parent[0], "r");
  FILE *read_from_child2 = fdopen(pipefd_child2_to_parent[0], "r");
  if(read_from_child1 == NULL || read_from_child2 == NULL){
    free(line);
    print_error_and_exit("fdopen failed");
  }
  // end block
  
  
  float *real_parts = malloc(sizeof(float)*num_of_lines); // space for real parts of result
  float *imaginary_parts = malloc(sizeof(float)*num_of_lines); // space for imaginary parts of result
  if(real_parts == NULL || imaginary_parts == NULL){
    free(line);
    fclose(read_from_child1);
    fclose(read_from_child2);
    print_error_and_exit("malloc failed");
  }

  float real_part_child1, real_part_child2, imaginary_part_child1, imaginary_part_child2;


  char *end_pointer;
  int counter = 0;

  // reading the numbers form children and calculating the result block
  while(1){

    // reading number from child1
    if(getline(&line,&length,read_from_child1) == -1){ 
      if (feof(read_from_child1)) break;
      else{
        free(line);
        fclose(read_from_child1);
        fclose(read_from_child2);
        free(real_parts);
        free(imaginary_parts);
        print_error_and_exit("getline failed");
      }
    }
    real_part_child1 = strtof(line,&end_pointer);
    imaginary_part_child1 = strtof(end_pointer+1,&end_pointer);
    if(errno != 0){
      free(line);
      fclose(read_from_child1);
      fclose(read_from_child2);
      free(real_parts);
      free(imaginary_parts);
      print_error_and_exit("strtof failed");
    }
    
    // reading number form child2
    if(getline(&line,&length,read_from_child2) == -1){
        free(line);
        fclose(read_from_child1);
        fclose(read_from_child2);
        free(real_parts);
        free(imaginary_parts);
        print_error_and_exit("getline failed"); 
    }
    real_part_child2 = strtof(line,&end_pointer);
    imaginary_part_child2 = strtof(end_pointer+1,&end_pointer);
     if(errno != 0){
      free(line);
      fclose(read_from_child1);
      fclose(read_from_child2);
      free(real_parts);
      free(imaginary_parts);
      print_error_and_exit("strtof failed");
    }

  
    // calculating the result
    real_parts[counter] = real_part_child1 + real_part_child2*cos(-(2*PI)/num_of_lines * counter) -imaginary_part_child2*sin(-(2*PI)/num_of_lines * counter);
    imaginary_parts[counter] = imaginary_part_child1 + sin(-(2*PI)/num_of_lines * counter)*real_part_child2 + cos(-(2*PI)/num_of_lines * counter)*imaginary_part_child2;

    real_parts[counter + num_of_lines/2]= real_part_child1 - real_part_child2*cos(-(2*PI)/num_of_lines * counter) +imaginary_part_child2*sin(-(2*PI)/num_of_lines * counter);
    imaginary_parts[counter +num_of_lines/2] = imaginary_part_child1 - sin(-(2*PI)/num_of_lines * counter)*real_part_child2 - cos(-(2*PI)/num_of_lines * counter)*imaginary_part_child2;

    counter++;
  }
  // end block

  
  // printing the result
  for(int i = 0;i < num_of_lines;i++){
    fprintf(stdout,"%f %f*i\n",real_parts[i],imaginary_parts[i]);
  }
  
  // cleaning up the resourcen block
  free(line);
  free(real_parts);
  free(imaginary_parts);
  if(fclose(read_from_child1) == EOF || fclose(read_from_child2) == EOF){
    print_error_and_exit("fclose failed");
  }
  // end block

  return EXIT_SUCCESS;
}