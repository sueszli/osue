#ifndef _CLIENT_H_
#define _CLIENT_H_

/** Structure for the arguments. */
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

#endif  // _CLIENT_H_
