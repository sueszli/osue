/**
 * @file forkFFT.c
 * @author Atheer Elobadi (e01049225@student.tuwien.ac.at)
 * @brief This program is an implementation of the Cooley-Tukey Fast Fourier Transform algorithm. 
 * @date 11.12.2021
 * 
 * @details The program applies the algorith by recursivly forking and calling itself. The generated 
 *          solutions are then sent up to the parents which in their turn calculate their solutions 
 *          and send them up... 
 */

#include "forkFFT.h"

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        usage(argv[0]);
    }

    char *line_check_one_input = NULL;
    char *line_check_eof = NULL;
    size_t len;

    int count = 0;

    child_t child[2];
    FILE *fd_down[2];
    FILE *fd_up[2];

    //Check if one line is given
    if (getline(&line_check_one_input, &len, stdin) == -1)
    {
        exit(EXIT_FAILURE);
    }

    complex_t first_complex_number;
    if (getline(&line_check_eof, &len, stdin) == -1)
    {
        strtoc(&first_complex_number, line_check_one_input);  
        fprintf(stdout, "%f %f*i\n", first_complex_number.real, first_complex_number.img);
        fflush(stdout);
        exit(EXIT_SUCCESS);
    }

    complex_t second_complex_number;
    strtoc(&second_complex_number, line_check_one_input);
    //Init pipes, fork and open 
    for (int i = 0; i < 2; i++)
    {
        if (pipe(child[i].pipefd_down) == -1 || pipe(child[i].pipefd_up) == -1)
        {
            fprintf(stderr, "Cannot pipe!\n%s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        switch (child[i].pid = fork())
        {
        case -1:
            fprintf(stderr, "Cannot fork!\n");
            exit(EXIT_FAILURE);
        case 0: // child
            if (init_pipes(child[i].pipefd_down, child[i].pipefd_up)==PIPE_FAILURE){
                fprintf(stderr, "Error initializing pipes: %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            execlp(argv[0], argv[0], NULL);
            fprintf(stderr, "Cannot exec!\n");
            exit(EXIT_FAILURE);
        default: // parent
            if (close(child[i].pipefd_down[PIPE_READ_END])==-1){
                fprintf(stderr, "Error closing pipefd_down\n%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if (close(child[i].pipefd_up[PIPE_WRITE_END])==-1){
                fprintf(stderr, "Error closing pipefd_up\n%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            if ((fd_down[i] = fdopen(child[i].pipefd_down[PIPE_WRITE_END], "w"))==NULL){
                fprintf(stderr, "Failed to open fd_down\n%s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }
            break;
        }
    }

    //Send data down
    fprintf(fd_down[CHILD_ODD], "%s", line_check_one_input);
    fflush(fd_down[CHILD_ODD]);
    
    fprintf(fd_down[CHILD_EVEN], "%s", line_check_eof);
    fflush(fd_down[CHILD_EVEN]);
    
    free(line_check_one_input);
    free(line_check_eof);
    
    count = 2;

    char *line = NULL;

    while (getline(&line, &len, stdin) != EOF)
    {
        count++;
        //     fprintf(stderr, "%d:  %s\n", getpid(), line);

        if ((count % 2) == 0)
        {
            fprintf(fd_down[CHILD_EVEN], "%s", line);
            fflush(fd_down[CHILD_EVEN]);
        }
        else
        {
            fprintf(fd_down[CHILD_ODD], "%s", line);
            fflush(fd_down[CHILD_ODD]);
        }
    }
    free(line);
    if ((count % 2) != 0)
    {
        fprintf(stderr, "Error: the count of input numbers must a power of 2.\n");
        exit(EXIT_FAILURE);
    }

    //Finished with the down pipes - Closing them
    if (close(child[CHILD_EVEN].pipefd_down[PIPE_WRITE_END])==-1) {
        fprintf(stderr, "Failed to open fd_down\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (close(child[CHILD_ODD].pipefd_down[PIPE_WRITE_END])==-1){
        fprintf(stderr, "Failed to open fd_down\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    //Wait for children to finish
    int status_o;
    waitpid(child[CHILD_EVEN].pid, &status_o, 0);
    if (WEXITSTATUS(status_o)){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(fd_down[CHILD_EVEN]);

    int status_e;
    waitpid(child[CHILD_ODD].pid, &status_e, 0);
    if (WEXITSTATUS(status_e)){
        fprintf(stderr, "Error: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fclose(fd_down[CHILD_ODD]);

    
    fd_up[CHILD_EVEN] = fdopen(child[CHILD_EVEN].pipefd_up[PIPE_READ_END], "r");
    if (fd_up[CHILD_EVEN]==NULL){
        fprintf(stderr, "Failed to open file. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    fd_up[CHILD_ODD] = fdopen(child[CHILD_ODD].pipefd_up[PIPE_READ_END], "r");
    if (fd_up[CHILD_ODD]==NULL){
        fprintf(stderr, "Failed to open file. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    complex_t result[count];
    
    complex_t *c_odd = malloc(sizeof(complex_t));
    if (c_odd == NULL){
        fprintf(stderr, "Failed to allocate memory to c_odd. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    complex_t *c_even = malloc(sizeof(complex_t));
    if (c_even==NULL){
        fprintf(stderr, "Failed to allocate memory to c_even. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    butterfly(result, c_even, c_odd, count, fd_up);
    free(c_odd);
    free(c_even);
    for (int i = 0; i < count; i++)
    {
        fprintf(stdout, "%f %f*i\n", result[i].real, result[i].img);
        fflush(stdout);
    }

    if (fclose(fd_up[CHILD_ODD])==EOF){
        fprintf(stderr, "Failed to close file. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (fclose(fd_up[CHILD_EVEN])==EOF){
        fprintf(stderr, "Failed to close file. %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
