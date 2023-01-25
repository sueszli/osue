## About this exam

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
You can run a script to update your man pages based on a tutorial in `manpages.txt`, because the old version does not contain all the helpful examples that you can use.

<br><br><br>

## Theoretical questions

- What is the difference between UDP and TCP sockets and when should they be used? (connection oriented vs. connectionless, ... reliable vs. unreliable, ...)
- What are strings in C? (end with null characters)
- What is the initial value of local and global variables?
- How do preprocessor macros work and which macros exist? (#ifdef, #define, #import, ...)
- Are arrays and pointers the same?
- What are modules used for in C? (and c files vs. h files)
- How do options work based on the unix conventions?
- How do you synchronize a shared resource?
- How does mutual exclusion work? (1, 2, 3 semaphores vs. local variables)
- How do POSIX semaphores work? (create, open, close, wait, post, unlink, ...)
- Understand static C code and determine the output of a function with nested loops.
- How is the development of unix kernel libraries different from the development of user space libraries? (you can't access extrenal libraries in the kernel)
- What things are considered a file in unix?
- What are unix drivers for? (hardware)
- What kind of unix devices are there? (block devices, network devices, ...)
- What kind of floating point number types are there in C? (float, double, long double - single precision, double precision, extended precision - they are always signed)
- Which ways of inter process communication exist? (pipes, sockets, shared memory, ...)
- What is a `FILE` in C?
- What is the internal represenation of arrays in C? How do arrays get stored in C?
- What is the difference between passing a variable by reference / passing by value to a function?

<br><br><br>

## Coding exercise [WORK IN PROGRESS]

### Task 1: create a socket as a server

Create a passive socket of domain `AF_INET` and type `SOCK_STREAM`.
Listen for connections on the port given by the argument `port_str`.

Return the file descriptor of the created socket so it can be used in the next step!

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
  if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
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

int setup_connection(const char *port_str) {
  // parse port
  errno = 0;
  int port = strtoul(port_str, NULL, 10);
  if (errno != 0) {
    error_exit("strtoul");
  }

  // create socket
  int sockfd = listen_socket(port);
  return sockfd; // will be used as the argument of task2() in main
}
```


### Task 2: accept connections from the created socket as a server

Wait for connections on the received socket file descriptor and accept them.

Read the arguments transmitted by the client from the connection and save them in a buffer that can hold a C-string with `MAX_ARGUMENT_LEN` characters (excluding `\0`).

Then you should call `execute_command()` with the content of the clients request and read the `FILE*` returned by this function.

The execution of the command should be done in a forked child process.

Then you should read the content from the file stream and send it to the client waiting on the accepted connection.

```c
#define MAX_ARGUMENT_LEN 100

/**
 * TODO!!
 */

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

### Task 3: send around files with a forked child

(You can use anything for executing the command, e.g. `system()` or `execvp()`)

```c
/**
 * TODO!!
 */

FILE* execute_command(char* arg) {
  ...
}
```
