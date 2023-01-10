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
  char* mime;
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
    if (access(args.rootPath, R_OK) == -1) {
      usage("root path not accessible");
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
  Response resp = {.httpStatusCode = 200, .mime = NULL, .resourceStream = NULL};

  // read first line: "GET /<name> HTTP/1.1"
  char* line = NULL;
  size_t len = 0;
  if ((getline(&line, &len, socketStream) == -1) && (errno != EINTR)) {
    fclose(socketStream);
    error("getline");
  }

  char reqMethod[strlen(line) + 1];
  char reqPath[strlen(line) + 1];
  if (sscanf(line, "%[^ ] /%[^ ] HTTP/1.1\r\n", reqMethod, reqPath) != 2) {
    resp.httpStatusCode = 400;
    return resp;
  }
  free(line);
  if (strlen(reqPath) < 1) {
    resp.httpStatusCode = 400;
    return resp;
  }
  if (strcmp(reqMethod, "GET") != 0) {
    resp.httpStatusCode = 501;
    return resp;
  }

  // join to full path
  char fullPath[strlen(args.rootPath) + strlen(reqPath) + 1];
  strcpy(fullPath, args.rootPath);
  strcat(fullPath, reqPath);
  if (reqPath[strlen(reqPath) - 1] == '/') {
    strcat(fullPath, args.defaultFileName);
  }

  // get mime type (don't use substring from fullPath to avoid free())
  char* mime = rindex(fullPath, '.');
  if (mime == NULL) {
    resp.httpStatusCode = 501;
    return resp;
  } else if ((strcmp(mime, ".html") == 0) || (strcmp(mime, ".htm") == 0)) {
    resp.mime = "text/html";
  } else if (strcmp(mime, ".css") == 0) {
    resp.mime = "text/css";
  } else if (strcmp(mime, ".js") == 0) {
    resp.mime = "application/javascript";
  } else {
    resp.httpStatusCode = 501;
    return resp;
  }

  // open stream of full path
  FILE* resourceStream = fopen(fullPath, "r+");
  if (resourceStream == NULL) {
    if (errno == ENOENT) {
      resp.httpStatusCode = 404;
      return resp;
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
  // status code
  char* httpStatusWord = NULL;
  switch (resp.httpStatusCode) {
    case -1:
      httpStatusWord = "Internal Server Error";
      resp.httpStatusCode = 500;
      break;

    case 200:
      httpStatusWord = "OK";
      break;

    case 400:
      httpStatusWord = "Bad Request";
      break;

    case 404:
      httpStatusWord = "Not Found";
      break;

    case 501:
      httpStatusWord = "Not Implemented";
      break;

    default:
      error("illegal state: received unkonwn http status code");
  }
  fprintf(socketStream, "HTTP/1.1 %d %s\r\n", resp.httpStatusCode,
          httpStatusWord);

  // connection: close
  fprintf(socketStream, "Connection: close\r\n");

  // stop here if error
  if (resp.httpStatusCode != 200) {
    fprintf(socketStream, "\r\n");
    return;
  }

  // write RFC-822 time
  char date[100];
  time_t rtime;
  time(&rtime);
  strftime(date, sizeof(date), "Date: %a, %d %b %y %H:%M:%S GMT\r\n",
           gmtime(&rtime));
  fprintf(socketStream, "%s", date);

  // content type
  fprintf(socketStream, "Content-Type: %s\r\n", resp.mime);

  // content-length
  fseek(resp.resourceStream, 0L, SEEK_END);
  size_t len = ftell(resp.resourceStream);
  rewind(resp.resourceStream);
  fprintf(socketStream, "Content-Length: %lu\r\n", len);

  // body
  fprintf(socketStream, "\r\n");
  int c;
  while ((c = fgetc(resp.resourceStream)) != EOF) {
    fputc(c, socketStream);
  }
}

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

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
      break;
    }

    FILE* socketStream = fdopen(reqfd, "w+");
    if (socketStream == NULL) {
      close(reqfd);
      perror("fdopen");
      continue;
    }
    setvbuf(socketStream, NULL, _IONBF, 0);

    Response resp = generateResponse(args, socketStream);
    sendResponse(resp, socketStream);

    fclose(socketStream);
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
  }

  exit(EXIT_SUCCESS);
}