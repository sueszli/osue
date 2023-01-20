/**
 * Here I removed getaddrinfo but this solution has not been tested with the
 * unit test service from the course yet.
 */

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>
#include <sys/select.h>
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
  uint16_t port;
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

  uint16_t port = 8080;
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
        if (strspn(optarg, "0123456789") != strlen(optarg)) {
          usage("port contains non digit characters");
        }
        errno = 0;
        unsigned long portLong = strtoul(optarg, NULL, 10);
        if (errno != 0) {
          error("strtoul");
        }
        if (portLong > 65535) {
          usage("port not in legal range");
        }
        if (portLong > INT16_MAX) {
          usage("port out of range for uint16_t on this machine");
        }
        port = (uint16_t)portLong;
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

/**
 * Don't refactor by using `getline()` instead of `read()` because it will not
 * detect `errno == EILSEQ` and will get stuck in an infinite loop.
 */
static Response generateResponse(Arguments args, int reqfd) {
  Response resp = {.httpStatusCode = 200, .mime = NULL, .resourceStream = NULL};

  // read first line: "GET /file HTTP/1.1"
  size_t bufSize = 1 << 10;
  char line[bufSize];
  memset(line, '\0', bufSize);
  if (read(reqfd, line, bufSize) == -1) {
    if (errno == EINTR) {
      // retry if interrupted mid read
      return generateResponse(args, reqfd);
    }
    if (errno == EILSEQ) {
      // non UTF-8 character
      resp.httpStatusCode = 400;
      return resp;
    }
    error("read");
  }

  // validate line
  if (strlen(line) < 14) {
    resp.httpStatusCode = 400;
    return resp;
  }
  char* reqMethod = strtok(line, " ");
  char* reqPath = strtok(NULL, " ");
  char* httpVersion = strtok(NULL, "\r\n");
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
      // file not found
      resp.httpStatusCode = 404;
      return resp;
    } else {
      error("fopen");
    }
  }
  resp.resourceStream = resourceStream;

  return resp;
}

/**
 * Don't refactor by using `fprintf()` instead of `write()` because many
 * test-cases use `wget` which only works with the functions `write()` and
 * `send()` which use the underlying file descriptor instead of `FILE*`.
 */
static void sendResponse(Response resp, int reqfd) {
  char line[1 << 10];

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
  sprintf(line, "HTTP/1.1 %d %s\r\n", resp.httpStatusCode, httpStatusWord);
  if (write(reqfd, line, strlen(line)) == -1) {
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
    close(reqfd);
    error("write");
  }

  // (stop here if error)
  const char* conStr = "Connection: close\r\n\r\n";
  if (resp.httpStatusCode != 200) {
    if (write(reqfd, conStr, strlen(conStr)) == -1) {
      if (resp.resourceStream != NULL) {
        fclose(resp.resourceStream);
      }
      close(reqfd);
      error("write");
    }
    return;
  }

  // send RFC-822 time
  char date[100];
  time_t rtime;
  time(&rtime);
  strftime(date, sizeof(date), "Date: %a, %d %b %y %H:%M:%S GMT\r\n",
           gmtime(&rtime));
  if (write(reqfd, date, strlen(date)) == -1) {
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
    close(reqfd);
    error("write");
  }

  // send content type
  if (resp.mime != NULL) {
    sprintf(line, "Content-Type: %s\r\n", resp.mime);
    if (write(reqfd, line, strlen(line)) == -1) {
      if (resp.resourceStream != NULL) {
        fclose(resp.resourceStream);
      }
      close(reqfd);
      error("write");
    }
  }

  // send content-length
  fseek(resp.resourceStream, 0L, SEEK_END);
  long int len = ftell(resp.resourceStream);
  rewind(resp.resourceStream);
  sprintf(line, "Content-Length: %lu\r\n", len);
  if (write(reqfd, line, strlen(line)) == -1) {
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
    close(reqfd);
    error("write");
  }

  // send connection: close
  if (write(reqfd, conStr, strlen(conStr)) == -1) {
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
    close(reqfd);
    error("write");
  }

  // send body
  int c;
  while ((c = fgetc(resp.resourceStream)) != EOF) {
    write(reqfd, &c, 1);
  }
}

int main(int argc, char* argv[]) {
  initSignalListener();

  Arguments args = parseArguments(argc, argv);

  struct sockaddr_in addr;

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    error("socket");
  }

  // makes port reusable before 1min passes
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
    close(sockfd);
    error("setsockopt");
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_port = htons(args.port);
  addr.sin_family = AF_INET;
  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    close(sockfd);
    error("bind");
  }

  const int queueLen = 1 << 4;
  if (listen(sockfd, queueLen) == -1) {
    close(sockfd);
    error("listen");
  }

  while (!quit) {
    int reqfd = accept(sockfd, NULL, NULL);
    if (reqfd == -1) {
      if (errno != EINTR) {
        close(sockfd);
        error("accept");
      }
      break;
    }

    // read
    Response resp = generateResponse(args, reqfd);
    shutdown(reqfd, SHUT_RD);

    // write
    sendResponse(resp, reqfd);
    shutdown(reqfd, SHUT_WR);

    // clean
    close(reqfd);
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
  }

  close(sockfd);
  exit(EXIT_SUCCESS);
}
