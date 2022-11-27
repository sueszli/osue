/**
 * @author Firat Berk Cakar, e0828668
 * @brief input headers for the mycompress assigment
 * @date 3.11.2021
 */
#ifndef MYCOMPRESS_INPUT_H
#define MYCOMPRESS_INPUT_H

typedef struct input_ctx * input_ctx_ptr;

void initialize_file_input_ctx(input_ctx_ptr input_ptr, char * filename);
input_ctx_ptr create_input_ctx();
void get_input(input_ctx_ptr input_ptr) ;
char * get_input_buffer(input_ctx_ptr input_ptr );
void free_input_ctx(input_ctx_ptr input_ptr);

#endif //MYCOMPRESS_INPUT_H
