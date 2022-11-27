/**
 * @file mycompress.c
 * @author Julius Bourdon ID:11904658 <e11904658@student.tuwien.ac.at>
 * @date 14.11.2022
 *
 * @brief Implementation of the program 'mycompress', which compresses the input and writes it into the provided output file or to stdout
 * 
 * @details 'mycompresss' parses the CLI arguments via getopt(3). It reads the input from arbitrary many input files or from stdin, if
 *          no input files are provided. The read input is written to an intermediate buffer. 
 *          Afterwards it calls the function 'compressstring' and writes the compressed result into an output file, if one was provided
 *          via CLI, or otherwise to stdout.  
 *      
 **/


#include "help_functions.h"

/**
 * @brief This static function prints information about the correct use of the mycompress program.
**/
void static usage_message(void){
    fprintf(stderr, "Usage: mycompress [-o outfile] [file...]\n");
}

/**
 * @brief This static function frees the allocated resources.
 *
 * @param read_write_buffer  The char-Pointer pointing to the intermediate buffer, where the input is written to.
 * @param success            The boolean indicating a successful or an erroneous termination.
 */
void static free_resources_and_exit(char *read_write_buffer, bool success){

    if(read_write_buffer != NULL){
        free(read_write_buffer);
    }

    fprintf(stdout, "\nResources freed!\n");

    if(success == true){
        exit(EXIT_SUCCESS);
    } else {
        exit(EXIT_FAILURE);
    }

}

/**
 * @brief   The main function of the mycompress program reading in all the input chars and printing them to the given output-stream.
 * @details The main function is responsible for the execution of the inherent functionality of the program.
 *          It sets up the 'read_write_buffer', which serves as an intermediate buffer between input and output. It reads from 
 *          the provided input and writes to the provided output. 
 *          To achieve this it uses the following functions: fopen(3), fclose(3), malloc(3), realloc(3)
 *          If any of the called functions fails then this function frees the intermediate buffer and terminates with EXIT_FAILURE.
 *
 * @param argc The argument counter. It can be 1, if no input files as well as no output file is provided. It's not restricted to the 
 *             upside, as there can be arbitrarily many input files. 
 * @param argv The array/pointer to the argument values. It contains the program name, the input files, if there are any and the 
 *             output file, if there is one.
 *
 * @return Returns EXIT_SUCCESS upon success or EXIT_FAILURE upon failure.
 */
int main(int argc, char *argv[]){

int curr_opt;
int opt_o = 0;
char *o_arg = NULL;

char *output_file_name = NULL;
char *input_file_name = NULL;

char *read_write_buffer = malloc(100*sizeof(char)); 
if(read_write_buffer == NULL){
    fprintf(stderr, "prog. 'mycompress' - memory allocation failed: %s\n", strerror(errno));
    free_resources_and_exit(read_write_buffer, false); 
}

while( (curr_opt = getopt(argc, argv, "o:")) != -1){
    switch(curr_opt){
        case 'o':
            opt_o++;
            o_arg = optarg;
            break; 
        case '?':
            fprintf(stderr, "prog. 'mycompress' - An invalid option was entered or an option argument is missing!\n");
            usage_message();
            free_resources_and_exit(read_write_buffer, false);            
            break;
        default:
            assert(0);
    }
}


int number_chars_read = 0;

FILE *input_stream;
FILE *output_stream = stdout;

int read_write_buffer_pos_init = 0;
int *read_write_buffer_pos = &read_write_buffer_pos_init;

int number_pos_arguments;

if(opt_o == 0){
    number_pos_arguments = argc - 1;

} else if(opt_o >= 2){
    fprintf(stderr, "prog. 'mycompress' - Option o is not allowed to be entered more than once!\n");
    usage_message();
    free_resources_and_exit(read_write_buffer, false); 

} else {
    output_file_name = o_arg;
    number_pos_arguments = argc - 3;

    /* NOTE: just to open the output-stream (this block is reached if there is an output-file)*/
    output_stream = fopen(output_file_name, "w+");
    if(output_stream == NULL){
        fprintf(stderr, "prog. 'mycompress' - fopen of output file failed: %s\n", strerror(errno));
        free_resources_and_exit(read_write_buffer, false); 
    }
}


/* NOTE: if there are no input-files */
if(number_pos_arguments == 0){
    fprintf(stdout, "Enter the input via CLI and finish with 'Ctrl D':");

    number_chars_read += compress_string(read_write_buffer, read_write_buffer_pos, stdin);
}

/* NOTE: for-loop to read from all input-files */
int i;
for(i = 0; i < number_pos_arguments; i++){

    input_file_name = argv[optind + i];

    input_stream = fopen(input_file_name, "r");
    if(input_stream == NULL){
        fprintf(stderr, "prog. 'mycompress' - fopen of an input file failed: %s\n", strerror(errno));
        free_resources_and_exit(read_write_buffer, false); 
    }

    if( (read_write_buffer = realloc(read_write_buffer, ((*read_write_buffer_pos)+1)*sizeof(char) + (int)(size_of_file(input_stream)*2))) == NULL){
        fprintf(stderr, "prog. 'mycompress' - memory allocation failed: %s\n", strerror(errno));
        free_resources_and_exit(read_write_buffer, false); 
    };
    
    number_chars_read += compress_string(read_write_buffer, read_write_buffer_pos, input_stream);

    if(fclose(input_stream) != 0){
        fprintf(stderr, "prog. 'mycompress' - Closing of an input-file failed: %s\n", strerror(errno));
        free_resources_and_exit(read_write_buffer, false); 
    }

}

/* NOTE: Re-Allocation to the buffer size actually needed */
if( (read_write_buffer = realloc(read_write_buffer, ((*read_write_buffer_pos)+1)*sizeof(char))) == NULL){
    fprintf(stderr, "prog. 'mycompress' - memory allocation failed: %s\n", strerror(errno));
    free_resources_and_exit(read_write_buffer, false);    
}


/* NOTE: This is the writing into the output-file, if there is one, or otherwise to stdout (default value of 'output_stream' is stdout) */
fprintf(output_stream, "%s", read_write_buffer);

fprintf(stdout, "\n----------------------\n\n");

int number_chars_written = (int)strlen(read_write_buffer);

fprintf(stderr, "Read: %d characters\nWritten: %d characters\nCompression ratio: %.2f %%\n", 
    number_chars_read, number_chars_written, ((double) number_chars_written)/number_chars_read*100);


if(output_stream != stdout){
    if( fclose(output_stream) != 0){
        fprintf(stderr, "prog. 'mycompress' - Closing of the output-file failed: %s\n", strerror(errno));
        free_resources_and_exit(read_write_buffer, false); 
    }
}

free_resources_and_exit(read_write_buffer, true); 
return EXIT_SUCCESS;
}

