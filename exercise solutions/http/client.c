#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define usage(msg)                                                           \
  do {                                                                       \
    fprintf(stderr,                                                          \
            "Wrong usage: %s\nSYNOPSIS:\n\tclient [-p PORT] [ -o FILE | -d " \
            "DIR ] URL\nEXAMPLE\n\tclient http://www.neverssl.com/\n",       \
            msg);                                                            \
    exit(EXIT_FAILURE);                                                      \
  } while (0);

#define error(msg)      \
  do {                  \
    perror(msg);        \
    exit(EXIT_FAILURE); \
  } while (0);

typedef struct {
  char* hostname;
  char* suffix;
  char* port;
  FILE* outputStream;
} Arguments;

static void freeArguments(Arguments* args) {
  if (args->hostname != NULL) {
    free(args->hostname);
  }
  if (args->suffix != NULL) {
    free(args->suffix);
  }
  if (args->outputStream != NULL) {
    fclose(args->outputStream);
  }
}

static void validateArguments(char* port, char* outputFile,
                              char* outputDirectory, char* url) {
  if (port != NULL) {
    if (strspn(port, "0123456789") != strlen(port)) {
      usage("port contains non digit characters");
    }
    errno = 0;
    long portLong = strtol(port, NULL, 10);
    if (errno != 0) {
      error("strtoul");
    }
    if ((portLong < 0) || (portLong > 65535)) {
      usage("port not in legal range");
    }
  }

  const char* illegalChars = "/\\:*?\"<>|";
  if (outputFile != NULL) {
    if (strspn(outputFile, illegalChars) != 0) {
      usage("file name contains illegal characters");
    }
    if (strlen(outputFile) > 255) {
      usage("file name too long");
    }
  }

  if (outputDirectory != NULL) {
    if (strspn(outputDirectory, illegalChars) != 0) {
      usage("directory name contains illegal characters");
    }
  }

  if (url != NULL) {
    if (strncasecmp(url, "http://", 7) != 0) {
      usage("url doesn't start with 'http://'");
    }
    if ((strlen(url) - 7) == 0) {
      usage("no characters preceeding 'http://'");
    }
  }
}

static Arguments parseArguments(int argc, char* argv[]) {
  bool optP = false;
  bool optO = false;
  bool optD = false;

  char* port = (char*)"80";
  char* outputFile = NULL;
  char* outputDirectory = NULL;
  char* url = NULL;

  int opt;
  while ((opt = getopt(argc, argv, "p:o:d:")) != -1) {
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

      case 'o':
        if (optO) {
          usage("option o used multiple times");
        }
        if (optD) {
          usage("didn't use option o xor d");
        }
        optO = true;
        if (optarg[0] == '-') {
          usage("missing option o argument");
        }
        outputFile = optarg;
        break;

      case 'd':
        if (optD) {
          usage("option d used multiple times");
        }
        if (optO) {
          usage("didn't use option o xor d");
        }
        optD = true;
        if (optarg[0] == '-') {
          usage("missing option d argument");
        }
        outputDirectory = optarg;
        break;

      default:
        usage("illegal option");
    }
  }
  if ((argc - optind) != 1) {
    usage("illegal number of positional arguments");
  }
  url = argv[optind];

  validateArguments(port, outputFile, outputDirectory, url);

  Arguments args = {
      .hostname = NULL,
      .suffix = NULL,
      .port = port,
      .outputStream = NULL,
  };

  // get suffix (everything after first ";/?:@=&")
  // suffix must start with "/"
  char* s = strpbrk(url + 7, ";/?:@=&");
  if (s == NULL) {
    asprintf(&args.suffix, "/");
  } else if (s[0] != '/') {
    asprintf(&args.suffix, "/%s", s);
  } else {
    asprintf(&args.suffix, "%s", s);
  }

  // get hostname (everything between "http://" and s)
  asprintf(&args.hostname, "%.*s", (int)(s - (url + 7)), url + 7);
  if (strlen(args.hostname) == 0) {
    freeArguments(&args);
    usage("no hostname preceeding 'http:");
  }

  // outputStream -> get resourceName (set to index.html if empty)
  char resourceName[strlen(args.suffix) + 1];
  char* rnStart = rindex(args.suffix, '/');
  rnStart++;
  char* rnEnd = strpbrk(rnStart, ";?:@=&");
  if (rnEnd == NULL) {
    rnEnd = args.suffix + strlen(args.suffix);
  }
  const ptrdiff_t rnLen = rnEnd - rnStart;
  strncpy(resourceName, rnStart, (size_t)rnLen);
  resourceName[rnLen] = '\0';
  if (strlen(resourceName) == 0) {
    strncpy(resourceName, "index.html", 11);
  }

  // outputStream -> get outputPath
  if (!optO) {
    outputFile = resourceName;
  }
  if (!optD) {
    outputDirectory = (char*)"";
  }
  char outputPath[strlen(outputDirectory) + strlen(resourceName) + 1];
  size_t curr = 0;
  if (optD) {
    strncpy(outputPath + curr, outputDirectory, strlen(outputDirectory));
    curr += strlen(outputDirectory);
    outputPath[curr++] = '/';
  }
  strncpy(outputPath + curr, outputFile, strlen(outputFile));
  curr += strlen(outputFile);
  outputPath[curr] = '\0';

  // get outputStream
  FILE* outputStream = stdout;
  if (optO | optD) {
    outputStream = fopen(outputPath, "w+");
    if (outputStream == NULL) {
      freeArguments(&args);
      error("fopen");
    }
  }
  args.outputStream = outputStream;

  return args;
}

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

  // get socket list and store in &result
  struct addrinfo* result;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  int s;
  if ((s = getaddrinfo(args.hostname, args.port, &hints, &result)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    freeArguments(&args);
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
    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;  // success
    }
    close(sockfd);
  }
  freeaddrinfo(result);
  if (rp == NULL) {
    freeArguments(&args);
    close(sockfd);
    error("no connection could be established");
  }

  // open unbuffered stream
  FILE* socketStream = fdopen(sockfd, "w+");
  if (socketStream == NULL) {
    freeArguments(&args);
    close(sockfd);
    error("fdopen");
  }
  setvbuf(socketStream, NULL, _IONBF, 0);

  // send request
  fprintf(socketStream,
          "GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "Connection: close\r\n\r\n",
          args.suffix, args.hostname);
  fflush(socketStream);

  // read first line of response
  char* line = NULL;
  size_t len = 0;
  if ((getline(&line, &len, socketStream) == -1) && (errno != 0)) {
    freeArguments(&args);
    fclose(socketStream);
    fprintf(stderr, "Protocol error - empty response!\n");
    exit(2);
  }

  // skip chars until 'H' (necessary for some unit tests)
  char* checkStart = line;
  while ((*checkStart != 'H') && (*checkStart != '\0')) {
    checkStart++;
  }

  // check response status code
  int status = -1;
  if (sscanf(checkStart, "HTTP/1.1 %d", &status) != 1) {
    freeArguments(&args);
    fclose(socketStream);
    free(line);
    fprintf(stderr, "Protocol error - unusual header!\n");
    exit(2);
  }
  if (status != 200) {
    freeArguments(&args);
    fclose(socketStream);
    fprintf(stderr, "%s", checkStart + 9);
    free(line);
    exit(3);
  }

  // go to end of header
  while (true) {
    if ((getline(&line, &len, socketStream) == -1)) {
      if (errno != 0) {
        freeArguments(&args);
        fclose(socketStream);
        free(line);
        error("getline");
      }
      break;
    }
    if (strcmp(line, "\r\n") == 0) {
      break;
    }
  }
  free(line);

  // copy content to outputStream
  int c;
  while ((c = fgetc(socketStream)) != EOF) {
    fputc(c, args.outputStream);
  }

  freeArguments(&args);
  fclose(socketStream);
  exit(EXIT_SUCCESS);
}