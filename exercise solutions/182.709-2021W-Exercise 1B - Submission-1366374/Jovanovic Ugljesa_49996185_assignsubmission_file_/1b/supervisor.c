/**
 * @brief supervisor program
 * @detail supervises generator programs on generating solutions for 3-coloring problem
 *
 * @author Ugljesa Jovanovic 11807861
 * @date 2021-11-14
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "circ_buf.h"

struct circ_buf cbuf;

/**
 * @brief SIGTERM and SIGINT handler
 * 
 * @param signal type
 */
static void signal_handler(int signal)
{
    circbuf_deactivate(&cbuf);
}

/**
 * @brief prints the given solution
 *
 * @param the solution to be printed
 */
static void print_solution(int solution[EDGE_SIZE * EDGE_SET_SIZE])
{
    int a = solution[0];
    int b = solution[1];

    if (a < 0 || b < 0) {
        printf("no edges\n");
        return;
    }

    int i = 0;
    while (a >= 0 && b >= 0) {
        printf("%d-%d ", a, b);
        i += 1;
        a = solution[i*2];
        b = solution[i*2+1];
    }
    printf("\n");
}

/**
 * @brief main function
 *
 * @param argument count
 * @param argument vector
 * @return exit code
 */
int main(int argc, char **argv)
{
    int best_len = EDGE_SET_SIZE, tmp_len = 0;
    int best[EDGE_SIZE * EDGE_SET_SIZE];
    int tmp[EDGE_SIZE * EDGE_SET_SIZE];
    int r;

    if (argc > 1) {
        fprintf(stderr, "SYNOPSYS:\n    %s\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_int;
    sa_int.sa_handler = signal_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    r = sigaction(SIGINT, &sa_int, NULL);
    if (r == -1) {
        fprintf(stderr, "%s: could not assign signal handler to SIGINT: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    struct sigaction sa_term;
    sa_term.sa_handler = signal_handler;
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;
    r = sigaction(SIGTERM, &sa_term, NULL);
    if (r == -1) {
        fprintf(stderr, "%s: could not assign signal handler to SIGTERM: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    cbuf.program = argv[0];
    circbuf_create(&cbuf);

    while (circbuf_isactive(&cbuf)) {
        r = circbuf_read(&cbuf, tmp);
        if (r == -2) { // sem_wait interrupted
            fprintf(stderr, "%s: circbuf read interrupted\n", argv[0]);
            continue;
        }
        if (r == -1) { // error
            fprintf(stderr, "%s: error reading circbuf: %s\n", argv[0], strerror(errno));
            break;
        }

        // evaluate length of read edge set
        {
            tmp_len = 0;
            int a = tmp[0];
            int b = tmp[1];
            while (a >= 0 && b >= 0) {
                tmp_len += 1;
                a = tmp[tmp_len*2];
                b = tmp[tmp_len*2+1];
            }
        }

        // check if better solution was found
        if (tmp_len < best_len) {
            best_len = tmp_len;
            memcpy(best, tmp, sizeof(int) * EDGE_SIZE * EDGE_SET_SIZE);

            printf("new best solution: ");
            print_solution(best);
        }

        // abort if solution with 0 edges was found
        if (best_len == 0) {
            circbuf_deactivate(&cbuf);
            break;
        }
    }

    fprintf(stderr, "%s: initiating shutdown\n", argv[0]);
    circbuf_destroy(&cbuf);
}
