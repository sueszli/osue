/**
 * @author Firat Berk Cakar, e0828668
 * @brief Main for the mycompress program
 * @date 3.11.2021
 */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mycompress.h"
#include "input.h"
#include "output.h"


int main(int argc, char **argv) {
    int output_file_flag = 0;
    char *cvalue = NULL;
    int c;

    opterr = 0;
    input_ctx_ptr inputCtx = create_input_ctx();
    output_ctx_ptr outputCtx = create_output_ctx();

    while ((c = getopt(argc, argv, "o:")) != -1)
        switch (c) {
            case 'o': //output file
                output_file_flag = 1;
                cvalue = optarg;
                break;
            case '?':
                if (optopt == 'o')
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint(optopt))
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf(stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return EXIT_FAILURE;
            default:
                fprintf(stderr,"no option given...");
                break; //no option given

        }

    if(!output_file_flag && argc > 2){
        fprintf(stderr,"Usage Error! \tProper input: %s [-o output_file] [input_file]\n", argv[0]);
        return 1;
    } 
    if (output_file_flag) { // there is an output file
        initialize_file_output_ctx(outputCtx, cvalue);

    }
    if ((output_file_flag && argc == 4) || (!output_file_flag && argc == 2)) {// there is an input file
        initialize_file_input_ctx(inputCtx, argv[argc - 1]);
    }


    get_input(inputCtx);
    size_t output_buffer_len = strlen(get_input_buffer(inputCtx)) *2+1;
    create_output_buffer(outputCtx, output_buffer_len);

    compress(get_input_buffer(inputCtx), get_output_buffer(outputCtx), get_output_buffer_len(outputCtx));

    write_to_output(outputCtx);
    write_statistics_to_sdterr(strlen(get_input_buffer(inputCtx)), strlen(get_output_buffer(outputCtx)));


    free_output_ctx(outputCtx);
    free_input_ctx(inputCtx);
    return EXIT_SUCCESS;

}



