/**
 * @file forksort.c
 * @author Paulina Patuzzi (01607360)
 * @date 2020-12-08
 *
 * @brief Implementation of forksort module
 * */

#include "forksort.h"


/**
 * @brief see forksort.h
 */
int main(int argc, char *argv[]){    

    char *snd_line = NULL;
    char *fst_line = NULL;
    size_t n = 0;
    int len1, len2;

    if ((len1 = getline(&fst_line, &n, stdin)) != -1) {
        
        if ((len2 = getline(&snd_line, &n, stdin)) != -1) {

            // at least two lines given, fork etc
            int fd_read_from_fst[2];
            pipe(fd_read_from_fst);
            
            int fd_write_to_fst[2];
            pipe(fd_write_to_fst);

            pid_t child_1, child_2;
            
            child_1 = fork();

            switch (child_1) {
                case -1:
                    fprintf(stderr, "Error when forking!\n");
                    exit(EXIT_FAILURE);

                case 0:
                    // is child 1

                    // close read end of read_from_fst and write end of write_to_fst
                    close(fd_read_from_fst[0]);
                    close(fd_write_to_fst[1]);

                    // redirect stdin to read end of write_to_fst
                    dup2(fd_write_to_fst[0], STDIN_FILENO);
                    close(fd_write_to_fst[0]);

                    // redirect stdout to write end of read_from_fst
                    dup2(fd_read_from_fst[1], STDOUT_FILENO);
                    close(fd_read_from_fst[1]);

                    execlp(argv[0], argv[0], NULL);
                    break;

                default: 
                {
                    // is parent
                    // close read end of write_to_fst and write end of read_from_fst
                    close(fd_write_to_fst[0]);
                    close(fd_read_from_fst[1]);

                    // create pipes for second child
                    int fd_read_from_snd[2];
                    pipe(fd_read_from_snd);
                    
                    int fd_write_to_snd[2];
                    pipe(fd_write_to_snd);

                    child_2 = fork();

                    switch (child_2) {
                        case -1:
                            fprintf(stderr, "Error when forking!\n");
                            exit(EXIT_FAILURE);

                        case 0:
                            // is child 2
                            
                            // close read end of read_from_snd and write end of write_to_snd
                            close(fd_read_from_snd[0]);
                            close(fd_write_to_snd[1]);

                            // redirect stdin to read end of write_to_snd
                            dup2(fd_write_to_snd[0], STDIN_FILENO);
                            close(fd_write_to_snd[0]);

                            // redirect stdout to write end of read_from_snd
                            dup2(fd_read_from_snd[1], STDOUT_FILENO);
                            close(fd_read_from_snd[1]);

                            execlp(argv[0], argv[0], NULL);
                            break;

                        default:
                        {
                            
                            close(fd_read_from_snd[1]);
                            close(fd_write_to_snd[0]);

                            FILE *write_to_fst = fdopen(fd_write_to_fst[1], "w");
                            if (fputs(fst_line, write_to_fst) == EOF) {
                                fprintf(stderr, "error writing to first pipe\n");
                                exit(EXIT_FAILURE);
                            }
                            
                            FILE *write_to_snd = fdopen(fd_write_to_snd[1], "w");
                            if (fputs(snd_line, write_to_snd) == EOF) {
                                fprintf(stderr, "error writing to second pipe\n");
                                exit(EXIT_FAILURE);
                            }

                            while((len1 = getline(&fst_line, &n, stdin)) != -1) {
                                if (fputs(fst_line, write_to_fst) == EOF) {
                                    fprintf(stderr, "error writing to first pipe\n");
                                    exit(EXIT_FAILURE);
                                }

                                if ((len2 = getline(&snd_line, &n, stdin)) != -1) {
                                    if (fputs(snd_line, write_to_snd) == EOF) {
                                        fprintf(stderr, "error writing to second pipe\n");
                                        exit(EXIT_FAILURE);
                                    }
                                }
                            }

                            // close write end of write pipes to indicate that writing is finished
                            fclose(write_to_fst);
                            close(fd_write_to_fst[1]);
                            fclose(write_to_snd);
                            close(fd_write_to_snd[1]);

                            int state1, state2;
                            if (waitpid(child_1, &state1, 0) == -1) {
                                fprintf(stderr, "error when waiting for child\n");
                                exit(EXIT_FAILURE);
                            }

                            if (waitpid(child_2, &state2, 0) == -1) {
                                fprintf(stderr, "error when waiting for child\n");
                                exit(EXIT_FAILURE);
                            }
                            fflush(stderr);

                            FILE *read_from_fst = fdopen(fd_read_from_fst[0], "r");
                            FILE *read_from_snd = fdopen(fd_read_from_snd[0], "r");

                            len1 = 0;
                            len2 = 0;

                            if ((len1 = getline(&fst_line, &n, read_from_fst)) == -1) {
                                fprintf(stderr, "no input from first child on first read\n");
                            }

                            if ((len2 = getline(&snd_line, &n, read_from_snd)) == -1) {
                                fprintf(stderr, "no input from second child on first read\n");
                            }
                            
                            while (1) {
                                if (strcmp(fst_line, snd_line) <= 0) {
                                    fprintf(stdout, "%s", fst_line);
                                    if ((len1 = getline(&fst_line, &n, read_from_fst)) != -1) {
                                        continue;
                                    }
                                } else {
                                    fprintf(stdout, "%s", snd_line);
                                    if ((len2 = getline(&snd_line, &n, read_from_snd)) != -1) {
                                        continue;
                                    }
                                }

                                // finish reading from one pipe if the other one is already empty
                                // and close the pipes
                                if ((len1 == -1) && (len2 == -1)) {
                                    fclose(read_from_fst);
                                    fclose(read_from_snd);
                                    close(fd_read_from_fst[0]);
                                    close(fd_read_from_snd[0]);
                                } else if (len1 == -1) {
                                    fclose(read_from_fst);
                                    close(fd_read_from_fst[0]);
                                    fprintf(stdout, "%s", snd_line);
                                    while ((len2 = getline(&snd_line, &n, read_from_snd)) != -1) {
                                        fprintf(stdout, "%s", snd_line);
                                    }
                                    fclose(read_from_snd);
                                    close(fd_read_from_snd[0]);
                                } else {
                                    fclose(read_from_snd);
                                    close(fd_read_from_snd[0]);
                                    fprintf(stdout, "%s", fst_line);
                                    while((len1 = getline(&fst_line, &n, read_from_fst)) != -1) {
                                        fprintf(stdout, "%s", fst_line);
                                    }
                                    fclose(read_from_fst);
                                    close(fd_read_from_fst[0]);
                                }
                                break;
                            }
                            free(fst_line);
                            free(snd_line);
                                                
                        }
                    } 
                }
            }
        } else {
            // simply print line to stdout
            fprintf(stdout, "%s", fst_line);
            fflush(stdout);
            free(fst_line);
        }

    } else {
        // TODO: better error message I guess
        fprintf(stderr, "Invalid usage: no input given");
        fflush(stdout);
    }

    return EXIT_SUCCESS;
} 