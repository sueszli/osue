## About this exam

80 points are acheivable in total consisting of:

- 30 points for the theoretical questions
- 50 points for the coding exercise (10 + 20 + 20 for each question)

Point deductions for wrong answers on theoretical questions:

- the theoretical questions are asked in a multiple choice format (each question can have one or more correct answers)
- for each wrong answer on an individual question, points are deducted from the theoretical part only but not from the coding exercise
- you can't get less than 0 points for the theoretical questions in total

Total number of theoretical questions: 15

<br><br><br>

## Theoretical questions

- What is the difference between UDP and TCP? 
- Strings in C end with null characters?
- Initial value of local and global variables?
- How preprocessing works?
- Are arrays and pointers the same?

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

int setup_connection(const char *port_str) {
  int port = strtoul(port_str);
  // ... check for strtoul errors
  int sockfd = listen_socket(port);
  return sockfd;
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

function() {
  // accept

  int sockfd = setup_connection(...);

  char buffer[MAX_ARGUMENT_LEN];
  int fd = execute_command(buffer);
  
  FILE* responseStream = fopen(fd, "r");
  fprintf(sockfd, ...);
  fclose(responseStream);
}
```




### 3. send around files with a forked child

```c
int execute_command(char* arg) {
  ...
}
```
