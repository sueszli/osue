#define _GNU_SOURCE
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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

static Arguments parseArguments(int argc, char* argv[]) {
  bool optP = false;
  bool optO = false;
  bool optD = false;

  char* port = "80";
  char* outputFile = "index.html";
  char* outputDirectory = "";
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

  // ----------------- validate -----------------
  if (optP) {
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
  if (optO) {
    const char* illegalChars = "/\\:*?\"<>|";  // unix is less strict
    if (strpbrk(outputFile, illegalChars) != NULL) {
      usage("file name contains illegal characters");
    }
    if (strlen(outputFile) > 255) {
      usage("file name too long");
    }
  }
  if (optD) {
    const char* illegalChars = "/\\:*?\"<>|";
    if (strpbrk(outputDirectory, illegalChars) != NULL) {
      usage("file name contains illegal characters");
    }
  }
  {
    if (strncasecmp(url, "http://", 7) != 0) {
      usage("url doesn't start with 'http://'");
    }
    if ((strlen(url) - 7) == 0) {
      usage("no characters preceeding 'http://'");
    }
  }

  // ----------------- parse arguments -----------------
  Arguments args = {
      .hostname = NULL,
      .suffix = NULL,
      .port = port,
      .outputStream = NULL,
  };

  // get suffix from url
  char* suffix = strpbrk(url + 7, ";/?:@=&");
  if (suffix == NULL) {
    asprintf(&args.suffix, "");
  } else {
    asprintf(&args.suffix, "%s", suffix);
  }
  printf("suffix: %s\n", args.suffix);

  // get hostname from url
  const ptrdiff_t hnLen = suffix - (url + 7);
  char hostname[hnLen + 1];
  strncpy(hostname, url + 7, (size_t)hnLen);
  hostname[hnLen] = '\0';
  asprintf(&args.hostname, "%s", hostname);
  printf("hostname: %s\n", hostname);

  // outputStream: get resourceName from url
  char resourceName[strlen(suffix) + 1];
  char* rnStart = rindex(suffix, '/');
  if (rnStart == NULL) {
    resourceName[0] = '\0';
  } else {
    rnStart++;
    char* rnEnd = strpbrk(rnStart, ";?:@=&");
    if (rnEnd == NULL) {
      rnEnd = url + strlen(url);
    }
    const ptrdiff_t rnLen = rnEnd - rnStart;
    strncpy(resourceName, rnStart, (size_t)rnLen);
    resourceName[rnLen] = '\0';
  }
  printf("resourceName: %s\n", resourceName);

  // outputStream: get outputPath
  char outputPath[strlen(outputDirectory) + strlen(outputFile) +
                  strlen(resourceName) + 3];
  size_t curr = 0;
  outputPath[curr++] = '.';
  outputPath[curr++] = '/';
  if (optD) {
    strncpy(outputPath + curr, outputDirectory, strlen(outputDirectory));
    curr += strlen(outputDirectory);
    outputPath[curr++] = '/';
    outputFile = resourceName;  // if !optD -> index.html or outputFile
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

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

  // print
  printf("hostname: %s\n", args.hostname);
  printf("suffix: %s\n", args.suffix);
  printf("port: %s\n", args.port);
  printf("\n");

  // write
  fprintf(args.outputStream, "HELLO WORLD");

  // free
  free(args.hostname);
  free(args.suffix);
  if (fclose(args.outputStream) == -1) {
    error("fclose");
  }
  exit(EXIT_SUCCESS);
}