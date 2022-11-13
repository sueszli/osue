/**
 * @author Firat Berk Cakar, e0828668
 * @brief ouput implementation for the mycompress assigment
 * @date 3.11.2021
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "output.h"
typedef struct output_ctx {
    bool output_file_present;
    FILE *output_stream;
    size_t output_size;
    char *output_buffer;

} output_ctx;

/**
 * creates an output context object, has to be called before anything else
 * @return pointer to the output context object
 */
output_ctx_ptr create_output_ctx() {
    output_ctx_ptr output_ptr = malloc(sizeof(output_ctx));
    output_ptr->output_stream = stdout;
    output_ptr->output_size = 0;
    output_ptr->output_buffer = NULL;
    output_ptr->output_file_present = false;
    return output_ptr;
}

/**
 * configure output context to write to a file, should be called right after create_output_ctx
 * @param output_ptr pointer to the output context obj created before
 * @param filename
 */
void initialize_file_output_ctx(output_ctx_ptr output_ptr, char *filename) {
    output_ptr->output_file_present = true;
    output_ptr->output_stream =  fopen(filename, "w+");
    if (output_ptr->output_stream == NULL) {
        fprintf(stderr, "Couldnt open input file %s for writing!\n", filename);
        exit(EXIT_FAILURE);
    }

}

/**
 * writes every buffered output to the dedicated (stdout or File) output stream
 * create_output_buffer must be called before
 * @param output_ptr pointer to the output context obj created before
 */
void write_to_output(output_ctx_ptr output_ptr){
    fwrite(output_ptr->output_buffer, 1, strlen(output_ptr->output_buffer), output_ptr->output_stream);

}
/**
 *
 * @param output_ptr pointer to the output context obj created before
 * @return output buffer
 */
char * get_output_buffer(output_ctx_ptr output_ptr){
    return output_ptr->output_buffer;
}

/**
 *
 * @param output_ptr pointer to the output context obj created before
 * @return output buffer length
 */
size_t get_output_buffer_len(output_ctx_ptr output_ptr){
    return output_ptr->output_size;
}

/**
 * creates an output buffer, must be called before calling write_to_output
 * @param output_ptr pointer to the output context obj created before
 * @param output_size size of the output buffer
 */
void create_output_buffer(output_ctx_ptr output_ptr, size_t output_size){
    output_ptr->output_size = output_size;
    output_ptr->output_buffer = malloc(output_size);
    if (output_ptr->output_buffer == NULL) {
        fprintf(stderr,"Malloc error from input ctx\n");
        exit(EXIT_FAILURE);
    }

}

/**
 * frees all allocated resources, must be called in the end
 * @param output_ptr pointer to the output context obj created before
 */
void free_output_ctx(output_ctx_ptr output_ptr) {
    free(output_ptr->output_buffer);
    if(output_ptr->output_file_present) {
        fclose(output_ptr->output_stream);
    }
    free(output_ptr);
}

/**
 *
 * @param input_len
 * @param compressed_output_len
 */
void write_statistics_to_sdterr(size_t input_len, size_t compressed_output_len){
    double ratio = (double )compressed_output_len/(double )input_len;
    fprintf(stderr,"Read:\t%zu characters\nWritten:\t%zu characters\nCompression ratio:\t%2lf percent\n",input_len,compressed_output_len,ratio);
}