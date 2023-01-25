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

(tip: there is a huge pool of questions and it is very unlikely that you will have any of these)

<br><br><br>

## Coding exercise

### Task 1: create a socket as a server

Create a passive socket of domain `AF_INET` and type `SOCK_STREAM`.
Listen for connections on the port given by the argument `port_str`.

Return the file descriptor of the created socket so it can be used in the next step!

```c
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
  // parse port (see: `man strtol`)
  errno = 0;
  int port = strtoul(port_str, NULL, 10);
  if (errno != 0) {
    error_exit("strtoul");
  }

  // create socket (see: `man select_tut`)
  int sockfd = listen_socket(port);
  if (sockfd == -1) {
    error_exit("listen_socket");
  }
  return sockfd;
}
```


### Task 2: accept connections from the created socket as a server

Wait for connections on the received socket file descriptor and accept them.

Read the arguments transmitted by the client from the connection and save them in a buffer that can hold a C-string with `MAX_ARGUMENT_LEN` characters (excluding `\0`).

Then you should call `execute_command()` with the content of the clients request and read the `FILE*` returned by this function.

The execution of the command should be done in a forked child process.

Then you should read the content from the file stream and send it to the client waiting on the accepted connection.

(tip: use `fileno(FILE* f)` to read and write using the underlying file descriptors, when things don't work as expected. Another advantage is that you won't have to call `fflush`.)

```c
#define MAX_ARGUMENT_LEN 100

void task2(int sockfd) {
  // accept (see: `man unix`)
  int fd = accept(sockfd, NULL, NULL);

  // read request
  FILE *clientStream = fdopen(fd, "r+");
  char buf[MAX_ARGUMENT_LEN + 1];
  memset(buf, 0, sizeof(buf));
  fgets(buf, MAX_ARGUMENT_LEN + 1, clientStream);

  // run child to generate response
  FILE *childResult = execute_command(buf);

  if(childResult == NULL) {
    // send error to client
    fprintf(clientStream, "ERROR_MESSAGE");
    fflush(clientStream);
    error_exit("");

  } else {
    // send response to client (see: `man pipe`)
    char c;
    while (read(fileno(childResult), &c, 1) > 0) {
        write(fileno(clientStream), &c, 1);
    }
  }

  // clean up
  fclose(childResult);
  fclose(clientStream);
}
```

### Task 3: send around files with a forked child

Create a child process through forking.

Then redirect the childs `fileno(stdout)` to the parents `fileno(stdin)` by using pipes.

Then wait for the child to exit in the parent process with exit status `0`.

Then read the childs output written into the pipe and use that to create a `FILE*` to return from this function.

Don't forket to close unused file descriptors.

(You are free to use anything to execute the command, e.g. `system()` or `execvp()`)

```c
/**
 * TODO!!
 */

FILE* execute_command(char* command, char* argument) {

  // fork

  // pipe

  /** @see `man system`*/
  execl(command, argument, (char *) NULL);
  

  // create FILE pointer
}
```
