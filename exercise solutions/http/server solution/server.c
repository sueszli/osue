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
#include <wchar.h>

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

#ifdef DEBUG
#define log(fmt, ...) (void)fprintf(stderr, fmt, ##__VA_ARGS__)
#elif
#define log(fmt, ...) (void)0
#endif

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

  const char* illegalDirectoryChars = "/\\:*?\"<>|";
  if (args.defaultFileName != NULL) {
    if (strcspn(args.defaultFileName, illegalDirectoryChars) !=
        strlen(args.defaultFileName)) {
      usage("default file name contains illegal characters");
    }
    if (strlen(args.defaultFileName) > 255) {
      usage("default file name too long");
    }
  }

  const char* illegalFileChars = "\\:*?\"<>|";
  if (args.rootPath != NULL) {
    if (strcspn(args.rootPath, illegalFileChars) != strlen(args.rootPath)) {
      usage("root path name contains illegal characters");
    }
    struct stat sb;
    memset(&sb, 0, sizeof(sb));
    if (stat(args.rootPath, &sb) == -1) {
      error("stat");
    }
    if (!S_ISDIR(sb.st_mode)) {
      usage("root path directory doesn't exist");
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

  // read first line (can be wide character): "GET /file HTTP/1.1"
  size_t bufSize = 1 << 10;
  wchar_t wcline[bufSize];
  wcline[0] = '\0';
  if (fgetws(wcline, (int)sizeof(wcline), socketStream) == NULL) {
    if (errno == EINTR) {
      log("%s", "> interrupted -> retry\n");
      return generateResponse(args, socketStream);
    }
    if (errno == EILSEQ) {
      log("%s", "> non ascii chars\n");
      resp.httpStatusCode = 400;
      return resp;
    }
    fclose(socketStream);
    error("fgetws");
  }
  char line[bufSize];
  if (wcstombs(line, wcline, sizeof(line)) == (size_t)-1) {
    error("wcstombs");
  }
  log("> wc line: %ls", wcline);
  log("> line: %s", line);

  // validate line
  if (strlen(line) < 16) {
    log("%s", "> line too short\n");
    resp.httpStatusCode = 400;
    return resp;
  }
  char* reqMethod = strtok(line, " ");
  char* reqPath = strtok(NULL, " ");
  char* httpVersion = strtok(NULL, "\r\n");
  log("> reqPath: %s\n", reqPath);
  if ((reqMethod == NULL) || (reqPath == NULL) || (httpVersion == NULL) ||
      (strcmp(httpVersion, "HTTP/1.1") != 0)) {
    resp.httpStatusCode = 400;
    return resp;
  }
  if (strcmp(reqMethod, "GET") != 0) {
    resp.httpStatusCode = 501;
    return resp;
  }

  // get full path
  char fullPath[strlen(args.rootPath) + strlen(reqPath) + 4];
  strcpy(fullPath, args.rootPath);
  if ((args.rootPath[strlen(args.rootPath) - 1] != '/') &&
      (reqPath[0] != '/')) {
    strcat(fullPath, "/");
  }
  strcat(fullPath, reqPath);
  if (reqPath[strlen(reqPath) - 1] == '/') {
    strcat(fullPath, args.defaultFileName);
  }
  log("> full path: %s\n", fullPath);

  // get mime type
  char* mime = rindex(fullPath, '.');
  if (mime != NULL) {
    if ((strcmp(mime, ".html") == 0) || (strcmp(mime, ".htm") == 0)) {
      resp.mime = (char*)"text/html";
    } else if (strcmp(mime, ".css") == 0) {
      resp.mime = (char*)"text/css";
    } else if (strcmp(mime, ".js") == 0) {
      resp.mime = (char*)"application/javascript";
    }
  }

  // open file stream
  FILE* resourceStream = fopen(fullPath, "r+");
  if (resourceStream == NULL) {
    if (errno == ENOENT) {
      log("> %s\n", "file not found");
      resp.httpStatusCode = 404;
      return resp;
    } else {
      fclose(socketStream);
      error("fopen");
    }
  }
  resp.resourceStream = resourceStream;

  return resp;
}

static void sendResponse(Response resp, FILE* socketStream) {
  // send status code
  char* httpStatusWord = NULL;
  switch (resp.httpStatusCode) {
    case -1:
      httpStatusWord = (char*)"(Internal Server Error)";
      resp.httpStatusCode = 500;
      break;

    case 200:
      httpStatusWord = (char*)"OK";
      break;

    case 400:
      httpStatusWord = (char*)"(Bad Request)";
      break;

    case 404:
      httpStatusWord = (char*)"(Not Found)";
      break;

    case 501:
      httpStatusWord = (char*)"(Not Implemented)";
      break;

    default:
      error("illegal state: received unkonwn http status code");
  }
  log("HTTP/1.1 %d %s\n", resp.httpStatusCode, httpStatusWord);

  fprintf(socketStream, "HTTP/1.1 %d %s\r\n", resp.httpStatusCode,
          httpStatusWord);

  // (stop here if error)
  if (resp.httpStatusCode != 200) {
    log("%s", "Connection: close\n");
    fprintf(socketStream, "Connection: close\r\n\r\n");
    return;
  }

  // send RFC-822 time
  char date[100];
  time_t rtime;
  time(&rtime);
  strftime(date, sizeof(date), "Date: %a, %d %b %y %H:%M:%S\r\n",
           gmtime(&rtime));
  log("%s", date);
  fprintf(socketStream, "%s", date);

  // send content type
  if (resp.mime != NULL) {
    log("Content-Type: %s\n", resp.mime);
    fprintf(socketStream, "Content-Type: %s\r\n", resp.mime);
  }

  // send content-length
  fseek(resp.resourceStream, 0L, SEEK_END);
  long int len = ftell(resp.resourceStream);
  rewind(resp.resourceStream);
  log("Content-Length: %ld\n", len);
  fprintf(socketStream, "Content-Length: %lu\r\n", len);

  // send connection: close
  log("%s", "Connection: close\n");
  fprintf(socketStream, "Connection: close\r\n");

  // send body
  fprintf(socketStream, "\r\n");
  int c;
  while ((c = fgetc(resp.resourceStream)) != EOF) {
    log("%c", c);
    fputc(c, socketStream);
  }
}

int main(int argc, char* argv[]) {
  initSignalListener();

  Arguments args = parseArguments(argc, argv);

  log("> port: %s\n> default file name: %s\n> root path: %s\n\n", args.port,
      args.defaultFileName, args.rootPath);

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
    if (reqfd == -1) {
      if (errno != EINTR) {  // not a signal
        perror("accept");
      }
      break;
    }

    /*
    char buffer[1024];
    int bytes_received;
    while ((bytes_received = recv(reqfd, buffer, sizeof(buffer), 0)) > 0) {
      printf("%.*s", bytes_received, buffer);
    }
    if (bytes_received < 0) {
      perror("recv");
      break;
    }
    */

    log("%s", "----------------------------------------------\n");

    FILE* socketStream = fdopen(reqfd, "w+");
    if (socketStream == NULL) {
      close(reqfd);
      perror("fdopen");
      continue;
    }
    setvbuf(socketStream, NULL, _IONBF, 0);

    Response resp = generateResponse(args, socketStream);
    shutdown(reqfd, SHUT_RD);

    sendResponse(resp, socketStream);
    shutdown(reqfd, SHUT_WR);

    log("%s", "----------------------------------------------\n\n");

    fclose(socketStream);
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
  }

  exit(EXIT_SUCCESS);
}
