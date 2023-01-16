/**
 * @file server.c
 * @brief A simple HTTP server.
 * @author Yahya Jabary <yahya.jabary@tuwien.ac.at>
 * @date 15.01.2023
 */

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

typedef struct {
  char* port;
  char* defaultFileName;
  char* rootPath;
} Arguments; /** < Struct to hold arguments sent from parseArguments() */

typedef struct {
  int httpStatusCode;
  char* mime;
  FILE* resourceStream;
} Response; /** < Response from generateResponse() to be used in
               sendResponse()*/

static volatile sig_atomic_t quit = false;

/**
 * @brief Flips quit variable to true.
 * @param sig ignored.
 * @param si ignored.
 * @param unused ignored.
 */
static void onSignal(int sig, siginfo_t* si, void* unused) { quit = true; }

/**
 * @brief Quits main() loop on SIGINT, SIGTERM, SIGQUIT through global variable
 * quit, called by initSignalListener().
 * @details Sets up signal handler through sigaction().
 */
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

/**
 * @brief Validates the arguments parsed by parseArguments().
 * @param args Arguments struct to be validated.
 */
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

/**
 * @brief Parses the arguments passed to main() but sends them to
 * validateArguments() for validation before returning them.
 * @param argc number of arguments.
 * @param argv array of arguments.
 * @return Arguments struct containing the parsed arguments.
 */
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

/**
 * @brief Generates a Response struct from the arguments and socket stream
 * @details Because a test-case contains raw bytes / non-UTF-8 characters,
 * fgetws() is used to read in the first line, determine if it is a valid UTF-8
 * string and then convert it to a char* string. All other FILE* streams would
 * break with non UTF-8 chars which is why we have this unusual approach.
 * @param args Arguments struct containing the arguments.
 * @param socketStream FILE* stream set from the request file descriptor
 * (actually not the socket).
 * @return Response struct containing the response which will be sent to the
 * sendResponse() function.
 * @post The socketStream is closed if an error occurs.
 * @post Returned resourceStream must be closed by the caller if != NULL.
 */
static Response generateResponse(Arguments args, FILE* socketStream) {
  Response resp = {.httpStatusCode = 200, .mime = NULL, .resourceStream = NULL};

  // read first line: "GET /file HTTP/1.1"
  // use fgetws to detect non UTF-8 chars (there is no other option)
  size_t bufSize = 1 << 10;
  wchar_t wcline[bufSize];
  wcline[0] = '\0';
  if (fgetws(wcline, (int)sizeof(wcline), socketStream) == NULL) {
    if (errno == EINTR) {
      // retry if interrupted mid read
      return generateResponse(args, socketStream);
    }
    if (errno == EILSEQ) {
      // non UTF-8 character
      resp.httpStatusCode = 400;
      return resp;
    }
    fclose(socketStream);
    error("fgetws");
  }

  // convert back to UTF-8
  char line[bufSize];
  if (wcstombs(line, wcline, sizeof(line)) == (size_t)-1) {
    fclose(socketStream);
    error("wcstombs");
  }

  // validate line
  if (strlen(line) < 16) {
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
      fclose(socketStream);
      error("fopen");
    }
  }
  resp.resourceStream = resourceStream;

  return resp;
}

/**
 * @brief Sends the response received from generateResponse() to the client
 * through the request file descriptor.
 * @param resp Response struct containing the response to be sent.
 * @param reqfd Request file descriptor.
 * @post The resourceStream is closed if != NULL.
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
    error("write");
  }

  // (stop here if error)
  const char* conStr = "Connection: close\r\n\r\n";
  if (resp.httpStatusCode != 200) {
    if (write(reqfd, conStr, strlen(conStr)) == -1) {
      if (resp.resourceStream != NULL) {
        fclose(resp.resourceStream);
      }
      error("write");
    }
    return;
  }

  // send RFC-822 time
  char date[100];
  time_t rtime;
  time(&rtime);
  strftime(date, sizeof(date), "Date: %a, %d %b %y %H:%M:%S\r\n",
           gmtime(&rtime));
  if (write(reqfd, date, strlen(date)) == -1) {
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
    error("write");
  }

  // send content type
  if (resp.mime != NULL) {
    sprintf(line, "Content-Type: %s\r\n", resp.mime);
    if (write(reqfd, line, strlen(line)) == -1) {
      if (resp.resourceStream != NULL) {
        fclose(resp.resourceStream);
      }
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
    error("write");
  }

  // send connection: close
  if (write(reqfd, conStr, strlen(conStr)) == -1) {
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
    error("write");
  }

  // send body
  int c;
  while ((c = fgetc(resp.resourceStream)) != EOF) {
    write(reqfd, &c, 1);
  }
}

/**
 * @brief Parses the arguments passed to the program.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return status code, 0 if successful.
 */
int main(int argc, char* argv[]) {
  initSignalListener();

  Arguments args = parseArguments(argc, argv);

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

  while (!quit) {
    int reqfd = accept(sockfd, NULL, NULL);
    if (reqfd == -1) {
      if (errno != EINTR) {
        // not a signal interrupt
        perror("accept");
      }
      break;
    }

    FILE* socketStream = fdopen(reqfd, "w+");
    if (socketStream == NULL) {
      close(reqfd);
      perror("fdopen");
    }
    setvbuf(socketStream, NULL, _IONBF, 0);

    Response resp = generateResponse(args, socketStream);
    shutdown(reqfd, SHUT_RD);

    sendResponse(resp, reqfd);
    shutdown(reqfd, SHUT_WR);

    fclose(socketStream);
    if (resp.resourceStream != NULL) {
      fclose(resp.resourceStream);
    }
  }

  exit(EXIT_SUCCESS);
}
