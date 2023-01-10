#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define usage(msg)                                                       \
  do {                                                                   \
    fprintf(stderr,                                                      \
            "Wrong usage: %s\nSYNOPSIS:\n\tserver [-p PORT] [-i INDEX] " \
            "DOC_ROOT\nEXAMPLE\n\tserver -p 1280 -i index.html "         \
            "~/Documents/my_website/\n",                                 \
            msg);                                                        \
    exit(EXIT_FAILURE);                                                  \
  } while (0);

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

typedef struct {
  char* port;
  char* defaultFileName;
  char* rootPath;
} Arguments;

typedef struct {
  int httpStatusCode;
  FILE* resourceStream;
} Response;

static volatile sig_atomic_t quit = false;
static void onSignal(int sig, siginfo_t* si, void* unused) { quit = true; }
static void initSignalListener(void) {
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO;
  sigemptyset(&sa.sa_mask);
  sa.sa_sigaction = onSignal;

  if (sigaction(SIGINT, &sa, NULL) == -1) {
    error("sigaction");
  }
  if (sigaction(SIGTERM, &sa, NULL) == -1) {
    error("sigaction");
  }
  if (sigaction(SIGQUIT, &sa, NULL) == -1) {
    error("sigaction");
  }
}

static void validateArguments(Arguments args) {
  if (args.port != NULL) {
    if (strspn(args.port, "0123456789") != strlen(args.port)) {
      usage("port contains non digit characters");
    }
    errno = 0;
    long portLong = strtol(args.port, NULL, 10);
    if (errno != 0) {
      error("strtoul");
    }
    if ((portLong < 0) || (portLong > 65535)) {
      usage("port not in legal range");
    }
  }

  const char* illegalFileChars = "/\\:*?\"<>|";  // unix is less strict
  if (args.defaultFileName != NULL) {
    if (strspn(args.defaultFileName, illegalFileChars) != 0) {
      usage("default file name contains illegal characters");
    }
    if (strlen(args.defaultFileName) > 255) {
      usage("default file name too long");
    }
  }

  const char* illegalPathChars = "\\:*?\"<>|";
  if (args.rootPath != NULL) {
    if (strspn(args.rootPath, illegalPathChars) != 0) {
      usage("root path name contains illegal characters");
    }
  }
}

static Arguments parseArguments(int argc, char* argv[]) {
  bool optP = false;
  bool optI = false;

  char* port = (char*)"8080";
  char* defaultFileName = (char*)"index.html";
  char* rootPath = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "p:i:")) != -1) {
    switch (opt) {
      case 'p':
        if (optP) {
          usage("option p used multiple times");
        }
        optP = true;
        if (optarg[0] == '-') {
          usage("missing option p argument or negative argument");
        }
        port = optarg;
        break;

      case 'i':
        if (optI) {
          usage("option i used multiple times");
        }
        optI = true;
        if (optarg[0] == '-') {
          usage("missing option i argument");
        }
        defaultFileName = optarg;
        break;

      default:
        error("illegal option");
    }
  }

  if ((argc - optind) != 1) {
    usage("illegal number of positional arguments");
  }
  rootPath = argv[optind];

  Arguments args = {
      .port = port, .defaultFileName = defaultFileName, .rootPath = rootPath};

  validateArguments(args);

  return args;
}

static Response generateResponse(Arguments args, FILE* socketStream) {
  Response resp = {.httpStatusCode = 200, .resourceStream = NULL};

  // read first line: "GET /<name> HTTP/1.1"
  char* line = NULL;
  size_t len = 0;
  if ((getline(&line, &len, socketStream) == -1) && (errno != EINTR)) {
    fclose(socketStream);
    error("getline");
  }
  char str[strlen(line) + 1];
  memcpy(str, line, strlen(line) + 1);
  free(line);

  char* reqMethod = strtok(str, " ");
  char* reqPath = strtok(NULL, " ");
  char* reqProtocol = strtok(NULL, " ");
  char* empty = strtok(NULL, "\r\n");

  // get status code
  if ((reqMethod == NULL) || (reqPath == NULL) || (reqProtocol == NULL) ||
      (empty != NULL) || (reqPath[0] != '/') ||
      (strcmp(reqProtocol, "HTTP/1.1\r\n") != 0)) {
    resp.httpStatusCode = 400;
  } else if (strcmp(reqMethod, "GET") != 0) {
    resp.httpStatusCode = 501;
  }

  // get full path
  char fullPath[strlen(args.rootPath) + strlen(reqPath) + 1];
  strcpy(fullPath, args.rootPath);
  strcat(fullPath, reqPath);
  if (reqPath[strlen(reqPath) - 1] == '/') {
    strcat(fullPath, args.defaultFileName);
  }

  printf("> Full resource path: %s\n", fullPath);

  // open stream for full path
  FILE* resourceStream = fopen(fullPath, "r+");
  if (resourceStream == NULL) {
    if (errno == ENOENT) {
      resp.httpStatusCode = 404;
    } else {
      fclose(socketStream);
      error("fopen");
    }
  }
  resp.resourceStream = resourceStream;

  // read and discard the rest of request
  while (fgetc(socketStream) != EOF) {
  }

  return resp;
}

static void sendResponse(Response resp, FILE* socketStream) {
  // write status
  char* httpStatus = NULL;
  switch (resp.httpStatusCode) {
    case -1:
      httpStatus = "Internal Server Error";
      break;

    case 200:
      httpStatus = "OK";
      break;

    case 400:
      httpStatus = "Bad Request";
      break;

    case 404:
      httpStatus = "Not Found";
      break;

    case 501:
      httpStatus = "Not Implemented";
      break;

    default:
      error("illegal state");
  }
  fprintf(socketStream, "HTTP/1.1 %d %s\r\n", resp.httpStatusCode, httpStatus);

  // write date
  char* date = malloc(100 * sizeof(char));
  time_t rtime;
  time(&rtime);
  struct tm* info = gmtime(&rtime);
  strftime(date, sizeof(date), "Date: %a, %d %b %y %H:%M:%S GMT\r\n", info);
  fprintf(socketStream, "%s", date);
  free(date);

  // write content type

  // write content-length
  fseek(resp.resourceStream, 0L, SEEK_END);
  int len = ftell(resp.resourceStream);
  rewind(resp.resourceStream);
  fprintf(socketStream, "Content-Length: %d\r\n", len);

  // write last modified

  // write connection: close
  fprintf(socketStream, "%s", "Connection: close\r\n\r\n");

  // write body
  int c;
  while ((c = fgetc(resp.resourceStream)) != EOF) {
    fputc(c, socketStream);
  }
}

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

  printf("> Received arguments:\n %s\n %s\n %s\n", args.port,
         args.defaultFileName, args.rootPath);

  initSignalListener();

  // get socket list and store in &result
  struct addrinfo* result;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  int s;
  if ((s = getaddrinfo(NULL, args.port, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    error("getaddrinfo");
  }

  // iterate through results until a connection succeeds
  int sockfd;
  struct addrinfo* rp;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) {
      continue;
    }
    if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
      break;  // success
    }
    close(sockfd);
  }
  freeaddrinfo(result);
  if (rp == NULL) {
    error("no connection could be established");
  }

  // set socket to be immediately reusable
  const int optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

  const int queueLen = 1 << 4;
  if (listen(sockfd, queueLen) == -1) {
    error("listen");
  }

  // listen to requests
  while (!quit) {
    int reqfd = accept(sockfd, NULL, NULL);
    if ((reqfd == -1) && (errno != EINTR)) {
      perror("accept");
      continue;
    }

    FILE* socketStream = fdopen(reqfd, "w+");
    if (socketStream == NULL) {
      perror("fdopen");
      continue;
    }

    Response resp = generateResponse(args, socketStream);
    sendResponse(resp, socketStream);

    fclose(socketStream);
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}