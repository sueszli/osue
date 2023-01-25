```
Relevant man pages:

- select_tut / getaddrinfo
- unix
- pipe
- system
```


```c

#define COMMAND ("./doStuff")
#define MAX_ARGUMENT_LEN (100)

#define READ  (0)
#define WRITE (1)



/*
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
*/

int task1(const char *port_str) {
  errno = 0;
  int port = strtoul(port_str, NULL, 10);
  if (errno != 0) {
    error_exit("");
  }

  int sockfd = listen_socket(port); // ---> select_tut
  if (sockfd == -1) {
    error_exit("");
  }
  return sockfd;
}



void task2(int sockfd) {
  int fd = accept(sockfd, NULL, NULL); // ---> unix

  FILE *clientStream = fdopen(fd, "r+");
  char buf[MAX_ARGUMENT_LEN + 1];
  memset(buf, 0, sizeof(buf));
  fgets(buf, MAX_ARGUMENT_LEN + 1, clientStream);

  FILE *childResult = task3(COMMAND, buf);
  if(childResult == NULL) {
    fprintf(clientStream, "ERROR_MESSAGE");
    fflush(clientStream);
    error_exit("");
  }

  char c; // ---> pipe
  while (read(fileno(childResult), &c, 1) > 0) {
    write(fileno(clientStream), &c, 1);
  }

  fclose(childResult);
  fclose(clientStream);
}



FILE* task3(char* command, char* argument) {
  int pipefd[2];
  pid_t cpid;

  if (pipe(pipefd) == -1) {
    error_exit("");
  }
  cpid = fork(); // ---> pipe
  if (cpid == -1) {
    error_exit("");
  }
  
  if (cpid == 0) {
    close(pipefd[READ]);
    dup2(pipefd[WRITE], fileno(stdout));
    close(pipefd[WRITE]);
    
    execl(command, command, argument, (char *) NULL); // ---> system
    error_exit("");
  }

  close(pipefd[WRITE]);
  wait(NULL);
  FILE *f = fdopen(pipefd[READ], "r");
  return f;
}
```
