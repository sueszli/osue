## Task 1: create a socket as a server

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
  // parse port
  errno = 0;
  int port = strtoul(port_str, NULL, 10);
  if (errno != 0) {
    error_exit("");
  }

  // create socket (see: `man select_tut`)
  int sockfd = listen_socket(port);
  if (sockfd == -1) {
    error_exit("");
  }
  return sockfd;
}
```

<br><br>

## Task 2: accept connections from the created socket as a server

```c
#define COMMAND ("./doStuff")
#define MAX_ARGUMENT_LEN (100)

void task2(int sockfd) {
  // accept (see: `man unix`)
  int fd = accept(sockfd, NULL, NULL);

  // read request
  FILE *clientStream = fdopen(fd, "r+");
  char buf[MAX_ARGUMENT_LEN + 1];
  memset(buf, 0, sizeof(buf));
  fgets(buf, MAX_ARGUMENT_LEN + 1, clientStream);

  // run child to generate response
  FILE *childResult = execute_command(COMMAND, buf);
  if(childResult == NULL) {
    fprintf(clientStream, "ERROR_MESSAGE");
    fflush(clientStream);
    error_exit("");
  }

  // send response to client (see: `man pipe`)
  char c;
  while (read(fileno(childResult), &c, 1) > 0) {
    write(fileno(clientStream), &c, 1);
  }

  // clean up
  fclose(childResult);
  fclose(clientStream);
}
```

<br><br>

## Task 3: send around files with a forked child

```c
#define READ  (0)
#define WRITE (1)

FILE* execute_command(char* command, char* argument) {

  // fork (see: `man pipe`)
  int pipefd[2];
  pid_t cpid;

  if (pipe(pipefd) == -1) {
    error_exit("");
  }
  cpid = fork();
  if (cpid == -1) {
    error_exit("");
  }
  
  if (cpid == 0) {
    // redirect (see: `man dup2`)
    close(pipefd[READ]);
    dup2(pipefd[WRITE], fileno(stdout));
    close(pipefd[WRITE]);
    
    // run command (see: `man system`)
    execl(command, command, argument, (char *) NULL);
    error_exit("");
  }

  close(pipefd[WRITE]);
  wait(NULL);
  FILE *f = fdopen(pipefd[READ], "r");
  return f;
}
```
