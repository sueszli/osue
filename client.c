#define _GNU_SOURCE
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

#define protocolError()                   \
  do {                                    \
    fprintf(stderr, "Protocol error!\n"); \
    exit(2);                              \
  } while (0);

typedef struct {
  char* hostname;
  char* suffix;
  char* port;
  FILE* outputStream;
} Arguments;

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

  const char* illegalChars = "/\\:*?\"<>|";  // unix is less strict
  if (outputFile != NULL) {
    if (strcspn(outputFile, illegalChars) != 0) {
      usage("file name contains illegal characters");
    }
    if (strlen(outputFile) > 255) {
      usage("file name too long");
    }
  }

  if (outputDirectory != NULL) {
    if (strcspn(outputDirectory, illegalChars) != 0) {
      usage("file name contains illegal characters");
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

  char* port = "80";
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

  printf("port: %s\n", port);
  printf("outputFile: %s\n", outputFile);
  printf("outputDirectory: %s\n", outputDirectory);
  printf("url: %s\n", url);
  printf("\n");

  validateArguments(port, outputFile, outputDirectory, url);

  Arguments args = {
      .hostname = NULL,
      .suffix = NULL,
      .port = port,
      .outputStream = NULL,
  };

  // get suffix (everything after first ";/?:@=&")
  char* suffix = strpbrk(url + 7, ";/?:@=&");
  if (suffix == NULL) {
    suffix = "/";  // never empty
  }
  asprintf(&args.suffix, "%s", suffix);
  printf("suffix: %s\n", args.suffix);

  // get hostname (everything between "http://" and suffix)
  asprintf(&args.hostname, "%.*s", (int)(suffix - (url + 7)), url + 7);
  printf("hostname: %s\n", args.hostname);

  // outputStream -> get resourceName (set to index.html if empty)
  char resourceName[strlen(suffix) + 1];
  char* rnStart = rindex(suffix, '/');
  if (strlen(rnStart) == 1) {
    strncpy(resourceName, "index.html", 11);
  } else {
    rnStart++;
    char* rnEnd = strpbrk(rnStart, ";?:@=&");
    if (rnEnd == NULL) {
      rnEnd = suffix + strlen(suffix);
    }
    const ptrdiff_t rnLen = rnEnd - rnStart;
    strncpy(resourceName, rnStart, (size_t)rnLen);
    resourceName[rnLen] = '\0';
  }
  printf("resourceName: %s\n", resourceName);

  // outputStream -> get outputPath
  if (!optO) {
    outputFile = resourceName;
  }
  if (!optD) {
    outputDirectory = "";
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
  printf("outputPath: %s\n", outputPath);
  printf("\n");

  // make directory
  if (optD) {
    if ((mkdir(outputDirectory, 0777) == -1) && (errno != EEXIST)) {
      error("mkdir");
    }
  }

  // get outputStream
  FILE* outputStream = stdout;
  if (optO | optD) {
    outputStream = fopen(outputPath, "w+");
    if (outputStream == NULL) {
      error("fopen");
    }
  }
  args.outputStream = outputStream;

  return args;
}

static void sendRequest(Arguments args, FILE* socketStream) {
  fprintf(socketStream,
          "GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "Connection: close\r\n\r\n",
          args.suffix, args.hostname);

  if (fflush(socketStream) == EOF) {
    error("fflush");
  }
}

static void readResponse(FILE* socketStream, FILE* outputStream) {
  char* line = NULL;
  size_t len = 0;
  if (getline(&line, &len, socketStream) == -1) {
    free(line);
    protocolError();
  }

  char* part = strtok(line, " ");
  if (part == NULL || strcmp(part, "HTTP/1.1") != 0) {
    free(line);
    protocolError();
  }

  char* status_code = strtok(NULL, " ");
  char* status_text = strtok(NULL, "\r\n");
  if (status_code == NULL || status_text == NULL) {
    free(line);
    protocolError();
  }

  char* endptr;
  strtol(status_code, &endptr, 10);
  if (endptr != status_code + strlen(status_code)) {
    // The statuscode is not a number
    free(line);
    protocolError();
  }

  if (strncmp(status_code, "200", 3) != 0) {
    // Statuscode is not 200
    fprintf(stderr, "STATUS: %s %s\n", status_code, status_text);
    free(line);
    return 3;
  }

  // Read the rest of the headers line by line
  bool is_compressed = false;
  bool is_chunked = false;
  while (true) {
    if (getline(&line, &len, socketStream) == -1) {
      error("No response content");
      fprintf(stderr, "ERROR: No content.\n");
      free(line);
      return 1;
    };

    if (strcmp(line, "\r\n") == 0) {
      break;
    }
  }
  free(line);

  // copy content
  unsigned long BUFFER_SIZE = 1024;
  uint8_t buf[BUFFER_SIZE];
  while (!feof(socketStream)) {
    size_t read = fread(buf, sizeof(uint8_t), BUFFER_SIZE, socketStream);
    fwrite(buf, sizeof(uint8_t), read, outputStream);
  }
}

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

  // get socket list and store in &result
  struct addrinfo* result;
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(args.hostname, args.port, &hints, &result) != 0) {
    error("getaddrinfo");
  }

  // iterate through result list until a connection was successful
  struct addrinfo* rp;
  int sockfd;
  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) {
      continue;
    }

    if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) != -1) {
      break;  // success
    } else {
      close(sockfd);
    }
  }
  freeaddrinfo(result);
  if (rp == NULL) {
    error("no address could connect");
  }

  // open unbuffered stream
  FILE* socketStream = fdopen(sockfd, "w+");
  if (socketStream == NULL) {
    error("fdopen");
  }
  setvbuf(socketStream, NULL, _IONBF, 0);

  sendRequest(args, socketStream);

  readResponse(socketStream, args.outputStream);

  free(args.hostname);
  free(args.suffix);
  if (fclose(args.outputStream) == -1) {
    error("fclose");
  }
  exit(EXIT_SUCCESS);
}