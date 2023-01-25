```c
//TASK 1

struct addrinfo hints, *ai;
int scokfd;

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET;    /* Allow IPv4 or IPv6 */
hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */

int s = getaddrinfo(NULL, port_str, &hints, &ai);
if (s != 0) {
   fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
   exit(EXIT_FAILURE);
}

sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
if(sockfd < 0)
	error_exit("");

if(bind(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
	error_exit("");

if (listen(sockfd, 1) == -1)
	error_exit("");

//TASK 2

int fd = accept(sockfd, NULL, NULL);

FILE *f = fdopen(fd, "r+");

buf[MAX_SIZE+1];
memset(buf, 0, maxsize+1);
fgets(buf, maxsize+1, f);

FILE *f1 = execute_command(buf);

if(f1 == NULL) {
	fprintf(f, "ERROR_MESSAGE");
	fflush(f);
	error_exit("");
} else {
	while(!feof(f1)) {
		buf1[MAX_SIZE+1];
		memset(buf1, 0, maxsize+1);
		fgets(buf1, maxsize+1, f1);

		fprintf(f, "%s", buf1);
		fflush(f);
	}
}
fclose(f);

//TASK 3

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
	close(pipefd[0]);
	dup2(pipefd[1], STDOUT_FILENO);
    close(pipefd[1]);
	execlp(arg1, arg1, arg2, NULL);
	error_exit("");
} else {
	close(pipefd[1]);
	wait(NULL);
	FILE *f = fdopen(pipefd[0], "r");
	return f;
}
return NULL;
```