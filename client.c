#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
  char* url;
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
  char* url = argv[optind];

  printf("-p %s\n", port);
  printf("-o %s\n", outputFile);
  printf("-d %s\n", outputDirectory);
  printf("url: %s\n", url);

  // get hostname from url (between "http://" and ";/?:@=&")
  if (strncasecmp(url, "http://", 7) != 0) {
    usage("url doesn't start with 'http://'");
  }
  if ((strlen(url) - 7) == 0) {
    usage("no characters preceeding 'http://'");
  }
  char* start = url + 7;
  char* end = strpbrk(start, ";/?:@=&");
  if (end == NULL) {
    end = url + strlen(url);
  }
  size_t hostnameLen = end - start;
  char hostname[hostnameLen + 1];
  strncpy(hostname, start, hostnameLen);
  hostname[hostnameLen] = '\0';
  printf("hostname: %s\n", hostname);

  // get suffix from url (after ";/?:@=&")
  char* suffix = end;
  printf("suffix: %s\n", suffix);

  // get resource location from suffix (after first "/" before ";?:@=&")

  /*
  // validate
  if (optP) {
    if (strspn(port, "0123456789") != strlen(port)) {
      usage("port contains non digit characters");
    }
    errno = 0;
    long portLong = strtoul(port, NULL, 10);
    if (errno != 0) {
      error("strtoul");
    }
    if ((portLong < 0) || (portLong > 65535)) {
      usage("port not in legal range");
    }
  }

  char* illegalChars = "/\\:*?\"<>|";  // unix is less strict
  if (optO) {
    if (strpbrk(outputFile, illegalChars) != NULL) {
      usage("file name contains illegal characters");
    }
    if (strlen(outputFile) > 255) {
      usage("file name too long");
    }
  }

  if (optD) {
    if (strpbrk(outputDirectory, illegalChars) != NULL) {
      usage("file name contains illegal characters");
    }
    // set outputfile through url
    // check if directory name valid
    // set directory
  }
  */

  // get outputPath and outputStream from given arguments
  /*
    get output stream (if both o,d not given: write to stdout)
      -o: store output in "./<filename>"
      -d: store output in "./<newdir>/<path in url | index.html>"
  */

  FILE* outputStream = stdout;

  return (Arguments){url : NULL, port : port, outputStream : NULL};
}

int main(int argc, char* argv[]) {
  Arguments args = parseArguments(argc, argv);

  // fclose(args.outputStream);
  exit(EXIT_SUCCESS);
}