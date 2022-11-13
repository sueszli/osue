/**
 * @author Firat Berk Cakar, e0828668
 * @brief input implementation for the mycompress assigment
 * @date 3.11.2021
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "input.h"

typedef struct input_ctx {
    bool input_file_present;
    FILE *input_stream;
    size_t input_size;
    char *input_buffer;

} input_ctx;

/**
 * creates an input context object, has to be called before anything else
 * @return  the pointer to the created input_ctx object
 */
input_ctx_ptr create_input_ctx() {
    input_ctx_ptr input_ptr = malloc(sizeof(input_ctx));
    input_ptr->input_file_present = false;
    input_ptr->input_stream = stdin;
    input_ptr->input_size = 0;
    input_ptr->input_buffer = NULL;
    return input_ptr;
}

/**
 * Configures the input context to read from a file
 * @param input_ptr pointer to the input context obj created before
 * @param filename
 */
void initialize_file_input_ctx(input_ctx_ptr input_ptr, char *filename) {
    input_ptr->input_file_present = true;
    input_ptr->input_stream = fopen(filename, "r");
    if (input_ptr->input_stream == NULL) {
        fprintf(stderr, "Couldnt open input file %s for writing!\n", filename);
        exit(EXIT_FAILURE);
    }
    fseek(input_ptr->input_stream, 0, SEEK_END); // seek to end of file
    input_ptr->input_size = ftell(input_ptr->input_stream); // get current file pointer
    fseek(input_ptr->input_stream, 0, SEEK_SET);

}
/**
 * gets input, either from stdin or from a file, depending on how is it configured before
 * @param input_ptr pointer to the input context obj created before
 */
void get_input(input_ctx_ptr input_ptr) {
    if (input_ptr->input_file_present) {
        input_ptr->input_buffer = malloc(input_ptr->input_size);
        if (input_ptr->input_buffer == NULL) {
            fprintf(stderr,"Malloc error from stdin\n");
            exit(EXIT_FAILURE);
        }
        fread(input_ptr->input_buffer, 1, input_ptr->input_size, input_ptr->input_stream);
    } else {
        getline(&(input_ptr->input_buffer), &(input_ptr->input_size), input_ptr->input_stream);

    }
}
/**
 *
 * @param input_ptr pointer to the input context obj created before
 * @return input buffer
 */
char *get_input_buffer(input_ctx_ptr input_ptr) {
    return input_ptr->input_buffer;
}
/**
 * Frees all resources being used by the input context, must be called after everything is done.
 * @param input_ptr pointer to the input context obj created before
 */
void free_input_ctx(input_ctx_ptr input_ptr) {
    free(input_ptr->input_buffer);
    if (input_ptr->input_file_present) {
        fclose(input_ptr->input_stream);
    }
    free(input_ptr);
}