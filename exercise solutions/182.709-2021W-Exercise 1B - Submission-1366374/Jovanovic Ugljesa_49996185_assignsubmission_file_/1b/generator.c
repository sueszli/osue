/**
 * @brief generator program
 * @detail generates solutions for the supervisor program
 *
 * @author Ugljesa Jovanovic 11807861
 * @date 2021-11-14
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "circ_buf.h"

/**
 * @brief usage function
 *
 * @param program name
 */
static void usage(const char *program)
{
    fprintf(stderr, "SYNOPSYS:\n"
    "    %s EDGE1...\n"
    "EXAMPLE:\n"
    "    %s 0-1 0-2 0-3 1-2 1-3 2-3\n", program, program);
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
    int r;
    const char* program = argv[0];

    if (argc <= 1) {
        usage(program);
        exit(EXIT_FAILURE);
    }

    // init seed for randomness
    srandom(time(NULL));

    // allocate memory for edges
    // memory size can be determined via argc
    int edge_count = argc - 1;
    int *edges = malloc(sizeof(int) * EDGE_SIZE * edge_count);
    if (edges == NULL) {
        fprintf(stderr, "%s: could not allocate memory for edges: %s\n", argv[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    // load edges from varg into memory
    for (int i = 1; i < argc; i++) {
        int *a = &edges[(i-1)*2];
        int *b = &edges[(i-1)*2+1];

        r = sscanf(argv[i], "%d-%d", a, b);
        if (r < 2) {
            fprintf(stderr, "%s: invalid format for positional argument: %s\n", argv[0], argv[i]);
            usage(program);
            exit(EXIT_FAILURE);
        }
    }

    // calculate the vertex count
    // via a 'find maximim' algorithm
    // max vert number + 1 is vert count
    int vert_count = 0;
    for(int i = 0; i < (edge_count * 2); i++) {
        if (edges[i] > vert_count) {
            vert_count = edges[i];
        }
    }
    vert_count += 1;

    // allocate memory for vert colors
    int* colors = malloc(sizeof(int) * vert_count);

    // declare memory for removed edges
    int removed_edges_size = 0;
    int removed_edges[EDGE_SIZE * EDGE_SET_SIZE];

    // set circular buffer for submitting solutions
    struct circ_buf cbuf;
    cbuf.program = argv[0];
    circbuf_init(&cbuf);

    while (circbuf_isactive(&cbuf)) {
        // color vertices randomly
        // NOTE: a 2- or 1-coloring is also possible here
        // but this is ok, since a 2- or 1-coloring is also a valid 3-coloring
        for (int i = 0; i < vert_count; i++) {
            colors[i] = random() % 3;
        }

        // reset removed_edges to empty
        removed_edges_size = 0;
        for (int i = 0; i < (EDGE_SIZE * EDGE_SET_SIZE); i++) {
            removed_edges[i] = -1;
        }

        // remove edges until 3-coloring is valid
        for (int i = 0; i < edge_count; i++) {
            int a = edges[i*2];
            int b = edges[i*2+1];

            int color_a = colors[a];
            int color_b = colors[b];

            if (color_a == color_b) {
                // solutions may be larger then the circular buffer supports
                // in this case we abandon the current coloring
                // and attempt a new coloring
                if (removed_edges_size >= EDGE_SET_SIZE) {
                    continue;
                }
                removed_edges[removed_edges_size*2] = a;
                removed_edges[removed_edges_size*2+1] = b;
                removed_edges_size += 1;
            }
        }

        // before the generator writes, it ought to check
        // if the circular buffer is active
        if (!circbuf_isactive(&cbuf)) {
            break;
        }

        r = circbuf_write(&cbuf, removed_edges);
        if (r == -1) {
            fprintf(stderr, "%s: error while writing solution to circbuf\n", argv[0]);
            break;
        }
    }

    fprintf(stderr, "%s: initiating shutdown\n", argv[0]);

    circbuf_close(&cbuf);
    free(edges);
    free(colors);
}
