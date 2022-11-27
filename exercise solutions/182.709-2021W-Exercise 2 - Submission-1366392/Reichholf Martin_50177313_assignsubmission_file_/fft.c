/**
 * @file common.h
 * @author Martin Reichholf <e12002225@student.tuwien.ac.at>
 * @date 07.11.2021 
 *
 * @brief program computing the fft of a sequence trough recursion
 **/

#include <stdlib.h>
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <math.h>
#include <stdio.h>

/** useful renames **/
#define u0 void 

#define s8 char
#define s16 short
#define s32 int
#define s64 long long

#define u8 unsigned s8
#define u16 unsigned s16
#define u32 unsigned s32
#define u64 unsigned s64

#define f32 float
#define f64 double

#define INTERNAL static

#define PI 3.1415926

/** Macro for Error messages **/
#define ERROR(X) write(STDERR_FILENO,"Error "X"\n",strlen("Error "X"\n")); 

#define LINE_LENGTH 4096
#define PRGM "./fft"

/**
 * @brief struct to store an imaginary via to floats
**/
typedef struct cf32 {
   /** real part **/
   f32 r;
   /** imaginary part **/
   f32 i;
} cf32;

/**
 * @brief struct to store resources
**/
typedef struct res {
   /** input buffer for even values **/
   s8 *bufferE;
   /** input buffer for odd values **/
   s8 *bufferO;
   /** complex buffer for result of fft **/
   cf32 *result;
   /** pipe for even process **/
   s32 pipeE[2];
   /** pipe for odd process **/
   s32 pipeO[2];
} res;


/**
 * @brief Function printing how to use the program.
**/
inline INTERNAL u0 usage() {
   ERROR("Usage: " PRGM);
}

/**
 * @brief Function responsible for cleaning up resources.
 * @param r The state of the resources.
 * @return -1 if an error occured and 0 if not. 
**/
INTERNAL s32 cleanup(res r) {
   s32 result = 0;

   free(r.bufferE);
   free(r.bufferO);
   free(r.result);

   if(r.pipeE[0] != 0) {
      if(close(r.pipeE[0]) == -1) {
         result = -1;
      }
   }

   if(r.pipeE[1] != 0) {
      if(close(r.pipeE[1]) == -1) {
         result = -1;
      }
   }

   if(r.pipeO[0] != 0) {
      if(close(r.pipeO[0]) == -1) {
         result = -1;
      }
   }

   if(r.pipeO[1] != 0) {
      if(close(r.pipeO[1]) == -1) {
         result = -1;
      }
   }

   return result;
}

/**
 * @brief Function responsible for exiting correctly on error.
 * @param r The state of the resources.
 * @param msg Error message.
**/
INTERNAL u0 error_exit(res r, const s8 msg[]) {
   if(strlen(msg) != 0) {
      write(STDERR_FILENO,"Error: ", strlen("Error: "));
      write(STDERR_FILENO, msg, strlen(msg));
      write(STDERR_FILENO, "\n", 1);
   }
   cleanup(r);
   exit(EXIT_FAILURE);   
}

/**
 * @brief Function to parse an imagary represented trough two values on a 
 *        line.
 * @param c pointer to imaginary to store result in.
 * @param buffer Buffer to read from.
 * @param size Size of buffer.
 * @return -1 if an error occured and 0 if not. 
**/
INTERNAL s32 parse_line(cf32 *c, s8 *buffer, u32 size) {
   s8 *end1 = 0, *end2 = 0;
   f32 a, b;

   a = strtof(buffer, &end1);

   if(end1 == buffer || errno != 0 || (*end1 != '\n' && *end1 != ' ')) {
      ERROR("parsing real");
      return -1;
   }

   b = strtof(end1, &end2);

   if(errno != 0 || (*end2 != '\n' && *end2 != ' ' && *end2 != '*')) {
      ERROR("parsing imaginary");
      return -1;
   }

   c->r = a;
   c->i = b;

   return 0;
}

/**
 * @brief Function responsible for creating pipes and a child process via 
 *        fork.
 * @param rpipes pointer to storage of pipes for parent process.
 * @return -1 if an error occured and 0 if not. 
**/
INTERNAL s32 create_child(s32 *rpipes) {
   s32 pipes[2][2];

   if(pipe(pipes[0]) == -1 || pipe(pipes[1]) == -1) {
      ERROR("couldn't open pipe");
      return -1;
   } 

   s32 cpid = fork();

   if(cpid == -1) {
      ERROR("fork");
      return -1;

   } else if(cpid == 0) { // child

      if(dup2(pipes[0][0], STDIN_FILENO) == -1) {
         ERROR("redirecting stdin failed");
         return -1;
      }

      if(dup2(pipes[1][1], STDOUT_FILENO) == -1) {
         ERROR("redirecting stdout failed");
         return -1;
      }

      if(close(pipes[0][1]) == -1 || close(pipes[1][0]) == -1 || 
         close(pipes[0][0]) == -1 || close(pipes[1][1]) == -1) {
         ERROR("closing pipes");
         return -1;
      }

      execlp(PRGM, PRGM, 0);

      ERROR("in execlp");
      return -1;

   } else { // parent
      if(close(pipes[0][0]) == -1 || close(pipes[1][1]) == -1) {
         ERROR("closing pipes");
         return -1;
      }

      rpipes[0] = pipes[1][0];
      rpipes[1] = pipes[0][1];

      return cpid; 
   }

}

/**
 * @brief Function that read a line unbuffered from the given file 
 *         descriptor.
 * @param fd file descriptor to read from.
 * @param buffer Buffer to write to.
 * @param size Size of buffer.
 * @return -1 if an error occured and 0 if not. 
**/
INTERNAL s32 read_line(s32 fd, s8 *buffer, u32 size) {
   s32 i;

   for(i = 0; i < size; ++i) {

      if(read(fd, buffer + i, 1) != 1) {
         return -1;
      }
      
      if(buffer[i] == '\n') {
         return i+1;
      }
   } 

   return size;
}

/**
 * @brief Function that waits for forked child.
 * @param cpid PID od child.
 * @return -1 if an error occured and 0 if not. 
**/
INTERNAL s32 wait_child(s32 cpid) {
   s32 code;

   waitpid(cpid,&code,0);

   if(WIFEXITED(code)){

      if(WEXITSTATUS(code) == EXIT_FAILURE){
         return -1;
      }

      return 0;

   } else {

      return 0;
   }
}

/**
 * @brief main function representing the core of the program
 * @param argc number of cmd line arguments  
 * @param argv array holding cmd line argument as 0 terminated strings
 * @return EXIT_FAILURE if an error occured and EXIT_SUCCESS if not. 
**/
s32 main(s32 argc, s8* argv[]) {
   res r = {0};
   s32 rsize1, rsize2, cpid1, cpid2, N, k;
   cf32 c1;

   // ----------------- deal with inputs --------------

   if(argc != 1) {
      usage();
      error_exit(r, "");
   }

   r.bufferE = malloc(LINE_LENGTH);
      
   if(r.bufferE == 0) {
      error_exit(r, "allocation falied!");
   }

   r.bufferO = malloc(LINE_LENGTH);

   if(r.bufferO == 0) {
      error_exit(r, "allocation falied!");
   }

   rsize1 = read_line(STDIN_FILENO, r.bufferE, LINE_LENGTH);

   if(rsize1 <= 1) {
      usage();
      error_exit(r, "");
   }

   rsize2 = read_line(STDIN_FILENO, r.bufferO, LINE_LENGTH);

   if(rsize2 <= 1) {
      if(parse_line(&c1, r.bufferE, rsize1) != -1) {

         if(write(STDOUT_FILENO, r.bufferE, rsize1) == -1) {
            error_exit(r, "write");
         }
         if(cleanup(r) == -1) {
            error_exit(r, "cleanup");
         }
         return EXIT_SUCCESS;

      } else {
         usage();
         error_exit(r, "");
      }
   }

   // -------------- setup children ----------------


   cpid1 = create_child(r.pipeE); // even

   if(cpid1 == -1) {
      error_exit(r, "");
   }

   cpid2 = create_child(r.pipeO); // odd

   if(cpid2 == -1) { 
      error_exit(r, "");
   }

   // ----------- write to children -----------------

   if(write(r.pipeE[1], r.bufferE, rsize1) == -1) {
      error_exit(r, "write");
   }

   if(write(r.pipeO[1], r.bufferO, rsize2) == -1) {
      error_exit(r, "write");
   }

   for(N = 2;;N+=2) {
      rsize1 = read_line(STDIN_FILENO, r.bufferE, LINE_LENGTH);

      if(rsize1 == -1) {
         break; 
      }

      rsize2 = read_line(STDIN_FILENO, r.bufferO, LINE_LENGTH);

      if(rsize2 == -1) {
         error_exit(r, "odd number of inputs");
      }

      if(write(r.pipeE[1], r.bufferE, rsize1) == -1) {
         error_exit(r, "write");
      }

      if(write(r.pipeO[1], r.bufferO, rsize2) == -1) {
         error_exit(r, "write");
      }
   }   
   
   if(close(r.pipeE[1]) == -1) {
      error_exit(r, "close");
   }

   if(close(r.pipeO[1]) == -1) {
      error_exit(r, "close");
   }

   r.pipeE[1] = 0;
   r.pipeO[1] = 0;

   // ----------- wait for children ---------------------

   if(wait_child(cpid1) == -1) {
      error_exit(r, "child didn't exit correctly");
   }

   if(wait_child(cpid2) == -1) {
      error_exit(r, "child didn't exit correctly");
   }

   // ----------- calculate fft ------------------------- 

   r.result = malloc(N/2 * sizeof(cf32));

   if(r.result == 0) {
      error_exit(r, "allocation falied!");
   }

   f32 c, s;
   cf32 c2, c3;

   for(k = 0; k < N/2; ++k) {
      rsize1 = read_line(r.pipeE[0], r.bufferE, LINE_LENGTH);

      if(rsize1 == -1) {
         error_exit(r, "read");
      }

      rsize2 = read_line(r.pipeO[0], r.bufferO, LINE_LENGTH);

      if(rsize2 == -1) {
         error_exit(r, "read");
      }

      if(parse_line(&c1, r.bufferE, rsize1) == -1) {
         error_exit(r, "");
      }

      if(parse_line(&c2, r.bufferO, rsize2) == -1) {
         error_exit(r, "");
      }

      c = cos(-2*PI/N*k);
      s = sin(-2*PI/N*k);

      c3.r = c1.r + (c * c2.r - s * c2.i);
      c3.i = c1.i + (c * c2.i + s * c2.r);

      rsize1 = snprintf(r.bufferE, LINE_LENGTH, "%f %f*i\n", c3.r, c3.i);
      write(STDOUT_FILENO, r.bufferE, rsize1);

      c3.r = c1.r - (c * c2.r - s * c2.i);
      c3.i = c1.i - (c * c2.i + s * c2.r);
      
      r.result[k] = c3;
   }    

   for(k = 0; k < N/2; ++k) {
      c3 = r.result[k];

      rsize1 = snprintf(r.bufferE, LINE_LENGTH, "%f %f*i\n", c3.r, c3.i);
      write(STDOUT_FILENO, r.bufferE, rsize1);
   }

   if(cleanup(r) == -1) {
      error_exit(r, "cleanup");
   }

   return EXIT_SUCCESS;
}
