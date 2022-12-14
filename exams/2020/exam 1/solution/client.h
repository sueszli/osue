#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"

typedef struct {
  char *password;
} args_t;

void DEMO_parse_arguments(int argc, char *argv[], args_t *args);
void DEMO_allocate_resources(void);
void DEMO_process_password(const char *password, char *hash);

void free_resources(void);
void usage(const char *msg);
void print_message(const char *msg);
void error_exit(const char *msg);

#endif
