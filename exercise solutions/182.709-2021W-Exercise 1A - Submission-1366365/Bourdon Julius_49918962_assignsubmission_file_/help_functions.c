/**
 * @file help_functions.c
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief Implementation of the functions declarated in help_functions.h. For documentation see help_functions.h.
 * 
 *      
 **/

#include "help_functions.h"

/** See help_functions.h for documentation of this function **/ 
int compress_string(char *buffer, int* buffer_pos, FILE* file_to_read_from){ 

    char act_char; 
    char prev_char;

    int run_counter = 0;
    int char_counter = 1;

    /* NOTE: managing offset of the buffer-pointer (not to be done, if it's the first invocation of compress_string -> then start at index 0)*/
    if((*buffer_pos) != 0){
        buffer += (*buffer_pos);
    }

    while(feof(file_to_read_from) == 0){

        /* Diesen Teil auskommentiert drinnen gelassen für Frage beim Abgabegespräch 
        if( (file_to_read_from == stdin) && (run_counter != 0)){

            buffer -= *buffer_pos; //set buffer back to its starting char for realloc

            if( (buffer = realloc(buffer, (*buffer_pos)*sizeof(char)+10*sizeof(char))) == NULL){
                fprintf(stderr, "help_function - memory allocation failed: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            
            buffer += *buffer_pos; //set buffer to the index where the last writing left off (+1)
        }
        */
        

        if( (act_char = fgetc(file_to_read_from)) != EOF){ /* NOTE: fgetc returns 'EOF' also on error (not just when reaching the end of the file) */
           
            if(run_counter == 0){
                *buffer = act_char;
                buffer++;
                (*buffer_pos)++;

                prev_char = act_char;
                run_counter++;

                continue;
            }

            if(prev_char == act_char){
                char_counter++;
            } else {
                sprintf(buffer, "%d", char_counter);
                buffer++;
                (*buffer_pos)++;
                
                if(char_counter > 9){
                    sprintf(buffer, "%d", char_counter%10);
                    buffer++;
                    (*buffer_pos)++;
                }

                *buffer = act_char;
                buffer++;
                (*buffer_pos)++;

                char_counter = 1;
            }

            prev_char = act_char;
            
        }
        run_counter++;

    }
    sprintf(buffer, "%d", char_counter); /* NOTE: inserting the number of the last char */

    (*buffer_pos)++;

    return run_counter;

}

/** See help_functions.h for documentation of this function **/
long size_of_file(FILE *file){
    if(file == stdin || file == stdout || file == stderr){
        fprintf(stderr, "stdin, stdout and stderr are not allowed as argument of 'size_of_file'");
        exit(EXIT_FAILURE);
    }

    long size;

    fseek(file, 0L, SEEK_END);
    size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    return size;
}
