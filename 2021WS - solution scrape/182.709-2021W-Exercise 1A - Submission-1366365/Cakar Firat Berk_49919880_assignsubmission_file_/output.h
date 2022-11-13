/**
 * @author Firat Berk Cakar, e0828668
 * @brief output headers for the mycompress assigment
 * @date 3.11.2021
 */
#ifndef MYCOMPRESS_OUTPUT_H
#define MYCOMPRESS_OUTPUT_H

typedef struct output_ctx * output_ctx_ptr;
output_ctx_ptr create_output_ctx();
void initialize_file_output_ctx(output_ctx_ptr output_ptr, char *filename);
void write_to_output(output_ctx_ptr output_ptr);
void free_output_ctx(output_ctx_ptr output_ptr);
void create_output_buffer(output_ctx_ptr output_ptr, size_t output_size);
size_t get_output_buffer_len(output_ctx_ptr output_ptr);
char * get_output_buffer(output_ctx_ptr output_ptr);
void write_statistics_to_sdterr(size_t input_len, size_t compressed_output_len);

#endif //MYCOMPRESS_OUTPUT_H
