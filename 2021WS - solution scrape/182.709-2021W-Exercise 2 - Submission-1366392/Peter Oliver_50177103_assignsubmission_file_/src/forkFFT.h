#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <unistd.h> //for fork()
#include <sys/wait.h>
#include <math.h>

#define EVEN 0
#define ODD 1
#define READ 0
#define WRITE 1
#define RE 0
#define IM 1
#define _GNU_SOURCE
