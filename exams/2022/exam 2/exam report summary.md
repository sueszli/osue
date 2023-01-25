## About this exam [CONFIRMED]

80 points are acheivable in total consisting of:

- 30 points for the theoretical questions
- 50 points for the coding exercise (10 + 20 + 20 for each question)

Point deductions for wrong answers on theoretical questions:

- the theoretical questions are asked in a multiple choice format (each question can have one or more correct answers)
- questions have 3-6 possible answers
- for each wrong answer on an individual question, points are deducted for that individual question only but not from the coding exercise
- you can't get less than 0 points for the theoretical questions in total

Total number of theoretical questions: 15 (from a huge pool of questions - so they aren't the same for every slot / group)

You can fetch the exam content with `fetch` and run the unit tests with `delivery`.
The last run of the unit tests determines the total number of points you get for the coding exercise.

<br><br><br>

## Theoretical questions

- What is the difference between UDP and TCP sockets and when should they be used? (connection oriented vs. connectionless, ... reliable vs. unreliable, ...)
- What are strings in C? (end with null characters)
- What is the initial value of local and global variables?
- How do preprocessor macros work and which macros exist? (#ifdef, #define, #import, ...)
- Are arrays and pointers the same?
- What are modules used for in C? (and c files vs. h files)
- How do options work based on the unix conventions?
- How does mutual exclusion work? (1, 2, 3 semaphores vs. local variables)
- How do POSIX semaphores work? (create, open, close, wait, post, unlink, ...)
- Understand static C code and determine the output of a function with nested loops.
- How is the development of unix kernel libraries different from the development of user space libraries? (you can't access extrenal libraries in the kernel)
- What things are considered a file in unix?
- What are unix drivers for? (hardware)

<br><br><br>

## Coding exercise

### 1. create a socket as a server

Create a passive socket of domain `AF_INET` and type `SOCK_STREAM`.
Listen for connections on the port given by the argument `port_str`.
Return the file descriptor of the communication socket `int setup_connection(const char *port_str)`.

The file descriptor will be used in the next step!

```c
/** @see `man select_tut`*/
static int listen_socket(int listen_port) {
  struct sockaddr_in addr;
  int lfd;
  int yes;

  lfd = socket(AF_INET, SOCK_STREAM, 0);
  if (lfd == -1) {
      perror("socket");
      return -1;
  }

  yes = 1;
  if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
          &yes, sizeof(yes)) == -1) {
      perror("setsockopt");
      close(lfd);
      return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_port = htons(listen_port);
  addr.sin_family = AF_INET;
  if (bind(lfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
      perror("bind");
      close(lfd);
      return -1;
  }

  printf("accepting connections on port %d\n", listen_port);
  listen(lfd, 10);
  return lfd;
}

int task1(const char *port_str) { // maybe called "setup_connection()" ?
  int port = strtoul(port_str);
  // TODO: check for strtoul errors: error_exit(...);
  int sockfd = listen_socket(port);
  return sockfd; // will be used as the argument of task2() in main
}
```


### 2. accept connections from the created socket as a server

Wait for connections on the received socket file descriptor and accept them.

Read the arguments transmitted by the client from the connection and save them in a buffer that can hold a C-string with the size `MAX_ARGUMENT_LEN`.

Then you should call `execute_command()` with the argument received by the client and read the file descriptor returned by this function.

The execution of the command should be done in a forked child process.

Then you should read the content from the file that the received file descriptor points to and send it to the client waiting on the accepted connection.

```c
#define MAX_ARGUMENT_LEN 100

void task2(int sockfd, char* address) {
  // accept connection and open stream
  int connectionFd = connect_socket(sockfd, address);
  FILE* connectionStream = fopen(connectionFd, "r+");  

  // read arguments from connection into buffer
  char buffer[MAX_ARGUMENT_LEN + 1]; // +1 for '\0'
  memset(buffer, 0, sizeof(buffer));

  int fd = execute_command(buffer);
  FILE* responseStream = fopen(fd, "r");


  fprintf(sockfd, ...);
  fclose(responseStream);
}
```

### 3. send around files with a forked child

(You can use anything for executing the command, e.g. `system()` or `execvp()`)

```c
int execute_command(char* arg) {
  ...
}
```
