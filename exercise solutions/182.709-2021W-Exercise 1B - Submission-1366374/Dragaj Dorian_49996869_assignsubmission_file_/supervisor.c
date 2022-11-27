#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include "shared_mem.h"
#include "circ_buff.h" 

volatile sig_atomic_t quit = 0;
void handle_signal(int signal) { quit = 1; }

int main(int argc, char *argv[])
{
    struct sigaction sa = { .sa_handler = handle_signal };
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    struct circ_buff *buffer = create_buff(true);
    while(quit == 0)
    {
        char *s = read_circ_buff(buffer);
        printf("Reading input: %s\n", s);    
        free(s);
    }
    close_buff(true, buffer);
    return 0;
}